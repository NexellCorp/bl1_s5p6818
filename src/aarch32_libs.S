/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : ARM
 *      File            : aarch32_lib.S
 *      Description     : 
 *      Author          : Firmware Team
 *      History         :
 */
#include "nx_peridot.h"

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
	ldr		r0, [r0, #-4]				//; get smc instruction
	and		r0, r0, #0xf					//; get imm4 [3:0]
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
		mov		r12, #0					// cpuid 0
		bx		r0						// jump to rom boot

