/**
 * $Id$
 * Copyright (C) 2008 - 2014 Nils Asmussen
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

#include <sys/common.h>
#include <sys/arch/x86/serial.h>
#include <sys/arch/x86/gdt.h>
#include <sys/task/proc.h>
#include <sys/task/thread.h>
#include <sys/task/smp.h>
#include <sys/mem/pagedir.h>
#include <sys/boot.h>
#include <sys/util.h>

/* make gcc happy */
EXTERN_C void bspstart(void *mbp);
EXTERN_C uintptr_t smpstart();
EXTERN_C void apstart();
static void idlestart();
extern SpinLock aplock;
extern ulong proc0TLPD;

static uint8_t initloader[] = {
#if DEBUGGING
#	include "../../../../build/x86_64-debug/user_initloader.dump"
#else
#	include "../../../../build/x86_64-release/user_initloader.dump"
#endif
};

void bspstart(void *mbp) {
	Boot::start(mbp);
}

uintptr_t smpstart() {
	ELF::StartupInfo info;
	size_t total = SMP::getCPUCount();
	/* the running thread has been stored on a different stack last time */
	Thread::setRunning(Thread::getById(0));

	/* start an idle-thread for each cpu */
	for(size_t i = 0; i < total; i++)
		Proc::startThread((uintptr_t)&idlestart,T_IDLE,NULL);

	/* start all APs */
	SMP::start();
	Timer::start(true);

	// remove first page-directory entry. now that all CPUs are started, we don't need that anymore
	(&proc0TLPD)[0] = 0;
	for(int i = 0; i < 3; ++i)
		PageTables::flushAddr(0x200000 * i,true);

	/* load initloader */
	if(ELF::loadFromMem(initloader,sizeof(initloader),&info) < 0)
		Util::panic("Unable to load initloader");
	/* give the process some stack pages */
	Thread *t = Thread::getRunning();
	if(!t->reserveFrames(INITIAL_STACK_PAGES))
		Util::panic("Not enough mem for initloader-stack");
	t->addInitialStack();
	t->discardFrames();

	extern ulong kstackPtr;
	kstackPtr = KSTACK_AREA + PAGE_SIZE - sizeof(ulong);
	return info.progEntry;
}

void apstart() {
	// TODO
}

static void idlestart() {
	if(!SMP::isBSP()) {
		/* unlock the temporary kernel-stack, so that other CPUs can use it */
		aplock.up();
	}
	thread_idle();
}
