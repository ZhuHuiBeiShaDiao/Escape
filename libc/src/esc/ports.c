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

#include <esc/common.h>
#include <esc/ports.h>

s32 requestIOPort(u16 port) {
	return requestIOPorts(port,1);
}

s32 releaseIOPort(u16 port) {
	return releaseIOPorts(port,1);
}

void inWordStr(u16 port,void *addr,u32 count) {
	__asm__ volatile (
		"rep; insw"
		: : "D"(addr), "c"(count), "d"(port)
	);
}

void outWordStr(u16 port,const void *addr,u32 count) {
	__asm__ volatile (
		"rep; outsw"
		: : "S"(addr), "c"(count), "d"(port)
	);
}
