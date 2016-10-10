/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sangjong, Han <hans@nexell.co.kr>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//; PUSH_HANDLER_FRAME : call at the top of exception entry (13 instructions)

	.global PUSH_HANDLER_FRAME
	.macro PUSH_HANDLER_FRAME

	SUB     SP, SP, #0x100
	STR     x30, [SP, #0xF0]	//; Push old LR
	STP     x28, x29, [sp, #0xE0]   //; Push old FP
	ADD     x29, SP, #0xE8          //; update FP
	STP     x26, x27, [sp, #0xD0]
	STP     x24, x25, [sp, #0xC0]
	STP     x22, x23, [sp, #0xB0]
	STP     x20, x21, [sp, #0xA0]
	STP     x18, x19, [sp, #0x90]
	STP     x16, x17, [sp, #0x80]
	STP     x14, x15, [sp, #0x70]
	STP     x12, x13, [sp, #0x60]
	STP     x10, x11, [sp, #0x50]
	STP     x8, x9, [sp, #0x40]
	STP     x6, x7, [sp, #0x30]
	STP     x4, x5, [sp, #0x20]
	STP     x2, x3, [sp, #0x10]
	STP     x0, x1, [sp, #0x0]

	ADD     x0, SP, #0x100                  //; previous sp location
	STR     x0, [SP, #0xF8]                 //; save previous sp

	MOV     x0, SP

	.endm

//; POP_HANDLER_FRAME : call at the end of exception entry (12 instructions)
	.global POP_HANDLER_FRAME
	.macro POP_HANDLER_FRAME

	MOV     x30, sp                         //; replace sp to x30
	LDR     x0, [SP, #0xF8]                 //; get saved previous sp
	MOV     sp, x0                          //; restore sp. now x30 is stack ptr

	LDP     x0, x1, [x30, #0x0]
	LDP     x2, x3, [x30, #0x10]
	LDP     x4, x5, [x30, #0x20]
	LDP     x6, x7, [x30, #0x30]
	LDP     x8, x9, [x30, #0x40]
	LDP     x10, x11, [x30, #0x50]
	LDP     x12, x13, [x30, #0x60]
	LDP     x14, x15, [x30, #0x70]
	LDP     x16, x17, [x30, #0x80]
	LDP     x18, x19, [x30, #0x90]
	LDP     x20, x21, [x30, #0xA0]
	LDP     x22, x23, [x30, #0xB0]
	LDP     x24, x25, [x30, #0xC0]
	LDP     x26, x27, [x30, #0xD0]
	LDP     x28, x29, [x30, #0xE0]
	LDR     x30, [x30, #0xF0]

	.endm

