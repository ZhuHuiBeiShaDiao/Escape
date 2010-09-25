/**
 * $Id$
 * Copyright (C) 2008 - 2009 Nils Asmussen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <assert.h>
#ifdef IN_KERNEL
#	include <sys/machine/cpu.h>
#	include <sys/task/thread.h>
#	include <sys/mem/paging.h>
#	include <sys/ksymbols.h>
#	include <sys/video.h>
#	include <sys/util.h>
#	include <esc/register.h>
#	define outb			util_outByte
#	define inb			util_inByte
#	define gettid()		({ \
	u32 __tid,__esp; \
	GET_REG("esp",__esp); \
	__tid = (__esp >= KERNEL_STACK - PAGE_SIZE) ? thread_getRunning()->tid : 0; \
})
#	define getCycles()	cpu_rdtsc()
#else
#	include <esc/ports.h>
#	include <esc/thread.h>
#	include <esc/debug.h>
#	include <stdio.h>
#	define outb			outByte
#	define inb			inByte
#endif

#ifndef PROFILE
#define PROFILE
#endif
#define STACK_SIZE	1024

#ifdef PROFILE
static void logUnsigned(u64 n,u8 base);
static void logChar(char c);

#if !IN_KERNEL
static bool initialized = false;
#endif
static u32 stackPos = 0;
static bool inProf = false;
static u64 callStack[STACK_SIZE];

#ifdef __cplusplus
extern "C" {
#endif

void __cyg_profile_func_enter(void *this_fn,void *call_site);
void __cyg_profile_func_exit(void *this_fn,void *call_site);

#ifdef __cplusplus
}
#endif

void __cyg_profile_func_enter(void *this_fn,void *call_site) {
	UNUSED(call_site);
	if(inProf)
		return;
	inProf = true;
#if !IN_KERNEL
	if(!initialized) {
		requestIOPort(0xe9);
		requestIOPort(0x3f8);
		requestIOPort(0x3fd);
		initialized = true;
	}
#endif
	logChar('>');
	logUnsigned((u32)gettid(),10);
	logChar(':');
	logUnsigned((u32)this_fn,16);
	logChar('\n');
	callStack[stackPos++] = getCycles();
	inProf = false;
}

void __cyg_profile_func_exit(void *this_fn,void *call_site) {
	UNUSED(this_fn);
	UNUSED(call_site);
	u64 now;
	if(inProf || stackPos <= 0)
		return;
	inProf = true;
	now = getCycles();
	stackPos--;
	logChar('<');
	logUnsigned((u32)gettid(),10);
	logChar(':');
	logUnsigned(now - callStack[stackPos],10);
	logChar('\n');
	inProf = false;
}

static void logUnsigned(u64 n,u8 base) {
	if(n >= base)
		logUnsigned(n / base,base);
	logChar("0123456789ABCDEF"[(n % base)]);
}

static void logChar(char c) {
	outb(0xe9,c);
	outb(0x3f8,c);
	while((inb(0x3fd) & 0x20) == 0)
		;
}
#endif
