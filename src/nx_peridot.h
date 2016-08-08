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

#ifndef NX_ARM_H
#define NX_ARM_H

#include "cfgBootDefine.h"

//; Standard definitions of mode bits and interrupt (I & F) flags in PSRs

#define Mode_USR 			0x10
#define Mode_FIQ 			0x11
#define Mode_IRQ 			0x12
#define Mode_SVC 			0x13
#define Mode_ABT 			0x17
#define Mode_UNDEF 			0x1B
#define Mode_SYS 			0x1F
#define Mode_HYP 			0x1A
#define Mode_MON 			0x16

#ifdef aarch32
#define A_Bit 				(1 << 8) //; when A bit is set, Abort is disabled
#define I_Bit 				(1 << 7) //; when I bit is set, IRQ is disabled
#define F_Bit 				(1 << 6) //; when F bit is set, FIQ is disabled
#endif

#ifdef aarch64
#define AArch64_EL3_SP3 		0x0D // EL3h
#define AArch64_EL3_SP0 		0x0C // EL3t
#define AArch64_EL2_SP2 		0x09 // EL2h
#define AArch64_EL2_SP0 		0x08 // EL2t
#define AArch64_EL1_SP1 		0x05 // EL1h
#define AArch64_EL1_SP0			0x04 // EL1t
#define AArch64_EL0_SP0			0x00

#define A_Bit 				(1 << 2) //; when A bit is set, Abort is disabled
#define I_Bit 				(1 << 1) //; when I bit is set, IRQ is disabled
#define F_Bit 				(1 << 0) //; when F bit is set, FIQ is disabled
#endif

//;-------------------------------------------------------------------------------
//; Control register 1 format bit definition
//;-------------------------------------------------------------------------------
#define BIT1_XP 			(1 << 23) //; Subpage AP
#define BIT1_U 				(1 << 22)  //; Enables unaligned data access
#define BIT1_L4 			(1 << 15) //; for ARM4V architecture
#define BIT1_RR 			(1 << 14) //; Replacement strategy for ICache & DCache
#define BIT1_V 				(1 << 13)  //; Loaction of exception vectors
#define BIT1_I 				(1 << 12)  //; ICache enable/disable
#define BIT1_Z 				(1 << 11)  //; Branch prediction
#define BIT1_R 				(1 << 9)   //; ROM protection
#define BIT1_S 				(1 << 8)   //; system protection
#define BIT1_B 				(1 << 7)   //; Endianness(Bin/Little : 1/0)
#define BIT1_C 				(1 << 2)   //; DCache enable/disable
#define BIT1_A 				(1 << 1)   //; Alignment fault enable/disable
#define BIT1_M 				(1 << 0)   //; MMU enable/disable

#define PRIMARY_CPU 			(0)
//;==================================================================
//; Porting defines
//;==================================================================

#define CPU_CLKSRC 			1
#define BUS_CLKSRC			0
#define DDR_CLKSRC			0
#define G3D_CLKSRC			0
#define MPG_CLKSRC 			0

#define USBD_VID			0x2375
#define USBD_PID 			0x5430

#define GPIO_GROUP_A 			0
#define GPIO_GROUP_B 			1
#define GPIO_GROUP_C 			2
#define GPIO_GROUP_D 			3
#define GPIO_GROUP_E 			4

#define POLY 0xEDB88320L
//#define POLY 0x04C11DB7L		// Reverse

#define ATF_SUSPEND_SIGNATURE		(0x41544600) /* ATF (ASCII) */
#define SUSPEND_SIGNATURE 		(0x50575200) /* PWR (ASCII) */
#define BL1_SUSPEND_SIGNATURE		(0x01020304)
#define USBREBOOT_SIGNATURE 		(0x85836666)

#define SDFSBOOT 			2
#define UARTBOOT 			3
#define SPIBOOT 			4
#define SDBOOT 				5
#define USBBOOT 			6
#define NANDEC 				7

#define BOOTMODE 			12

#define NANDTYPE 			0
#define NANDPAGE 			2
#define SELCS 				3
#define SELSDEX 			15

#define eMMCBOOTMODE 			7
#define SDXCPARTITION 			9
#define eMMCBOOT 			10

#define UARTBAUD 			10

#define SERIALFLASHADDR 		9

#define OTG_SESSION_CHECK 		6

#define DECRYPT 			4
#define ICACHE 				5

#define BASEADDR_SRAM 			(0xFFFF0000)
#define INTERNAL_SRAM_SIZE 		(64 * 1024) // 64KB

#define BASEADDR_NFMEM 			(0x2C000000)
#define BASEADDR_DDRSDRAM 		(0x40000000)

#define DIRECT_IO 0
#endif
