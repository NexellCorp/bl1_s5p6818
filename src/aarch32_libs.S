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

#include "nx_peridot.h"

         .global get_fcs
get_fcs:
//      .long   0xE(cond:all), 1, 0(byte), 0(Rn), 0(Rd), 0(C), 4, 1(Rd)
         .long   0xE1000041      // CRC32B       R0, R0, R1
         bx      lr
         .global sget_fcs
sget_fcs:
         .long   0xE1200041      // CRC32H       R0, R0, R1
         bx      lr
         .global iget_fcs
iget_fcs:
         .long   0xE1400041      // CRC32W       R0, R0, R1
         bx      lr

	.global GetCurrentSMode
//; unsigned int GetCurrentSMode(void);
GetCurrentSMode:
	mrs		r0, cpsr
	and		r0, r0, #0x1F
	bx		lr

	.global GetCPUID
//; unsigned int GetCPUID(void);
GetCPUID:
	mrc     p15, 0, r0, c0, c0, 5              //; Get our cpu id
	tst     r0, #0x4400                        //; check processor affinity
	orrne   r0, r0, #4                        //; mark cpu0 or cpu1
	ands    r0, r0, #0xF                      //; Save CPU id
	bx		lr

	.global GetSCR
//; unsigned int GetSCR(void);
GetSCR:
	mrc		p15, 0, r0, c1, c1, 0
	bx		lr

	.global SetSCR
//; void SetSCR(unsigned int rSCR);
SetSCR:
	mcr		p15, 0, r0, c1, c1, 0
	bx		lr

	.global SetNS
//; void SetNS(void);
SetNS:
	mrc		p15, 0, r0, c1, c1, 0
	orr		r0, r0, #(1<<0)
	mcr		p15, 0, r0, c1, c1, 0

	.global GetSMCCode
//; unsigned int GetSMCCode(void*);
GetSMCCode:
	ldr		r0, [r0, #-4]			//; get smc instruction
	and		r0, r0, #0xf			//; get imm4 [3:0]
	bx		lr

	.global	EnterLowLevel
//; void EnterLowLevel(unsigned int *SVCStart, unsigned int SPSR, unsigned int SCR);
EnterLowLevel:
	mcr		p15, 0, r2, c1, c1, 0		// write r2 to SCR
	msr		SPSR, r1
	mov		lr, r0
	dsb		sy
	movs	pc, lr

.global RomUSBBoot
RomUSBBoot:
	mov		r11, #0x00000006		// boot config - usb boot
	mov		r12, #0				// cpuid 0
	bx		r0				// jump to rom boot

