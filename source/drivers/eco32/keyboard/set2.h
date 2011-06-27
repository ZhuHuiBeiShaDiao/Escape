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

#ifndef SET2_H_
#define SET2_H_

#include <esc/common.h>

/**
 * Converts the given scancode to a keycode
 *
 * @param isBreak whether it is a break-keycode
 * @param keycode the keycode
 * @param scanCode the received scancode
 * @return true if it was a keycode
 */
bool kb_set2_getKeycode(uchar *isBreak,uchar *keycode,uchar scanCode);

#endif /* SET2_H_ */