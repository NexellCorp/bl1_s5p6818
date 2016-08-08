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
#include "sysheader.h"

extern void ResetCon(U32 devicenum, CBOOL en);
extern void DMC_Delay(int milisecond);

//------------------------------------------------------------------------------
#if (MULTICORE_BRING_UP == 1)
#ifdef aarch32
void BringUpSlaveCPU(U32 CPUID)
{
	WriteIO32(&pReg_ClkPwr->CPURESETMODE, 0x1);
	WriteIO32(&pReg_ClkPwr->CPUPOWERDOWNREQ, (1 << CPUID));
	WriteIO32(&pReg_ClkPwr->CPUPOWERONREQ, (1 << CPUID));
}

void SetVectorLocation(U32 CPUID, CBOOL LowHigh)
{
	U32 addr, bits, regvalue;

	if (CPUID & 0x4) // cpu 4, 5, 6, 7
	{
		addr = (U32)&pReg_Tieoff->TIEOFFREG[95];
		bits = 1 << (12 + (CPUID & 0x3));
	} else {
		addr = (U32)&pReg_Tieoff->TIEOFFREG[78];
		bits = 1 << (20 + (CPUID & 0x3));
	}

	regvalue = ReadIO32(addr);
	if (LowHigh)
		regvalue |= bits;
	else
		regvalue &= ~bits;
	WriteIO32(addr, regvalue);
}
#endif

#ifdef aarch64
void BringUpSlaveCPU(U32 CPUID)
{
	WriteIO32(&pReg_ClkPwr->CPURESETMODE, 0x1);
	WriteIO32(&pReg_ClkPwr->CPUPOWERONREQ, (1 << CPUID));
	ClearIO32(&pReg_ClkPwr->CPUPOWERONREQ, (1 << CPUID));
}

void SetVectorLocation(U32 CPUID, CBOOL LowHigh)
{
	U32 regvalue;
	LowHigh = LowHigh; // for AArch32 comfortable
	if (CPUID & 0x4)   // cpu 4, 5, 6, 7
	{
		CPUID &= 0x3;
		regvalue = ReadIO32(&pReg_Tieoff->TIEOFFREG[96]);
		regvalue |= 1 << (4 + CPUID);
		WriteIO32(&pReg_Tieoff->TIEOFFREG[96], regvalue);
		WriteIO32(&pReg_Tieoff->TIEOFFREG[97 + (CPUID << 1)],
			  0xFFFF0200 >> 2);
	} else // cpu 0, 1, 2, 3
	{
		regvalue = ReadIO32(&pReg_Tieoff->TIEOFFREG[79]);
		regvalue |= 1 << (12 + CPUID); // set cpu mode to AArch64
		WriteIO32(&pReg_Tieoff->TIEOFFREG[79], regvalue);
		WriteIO32(&pReg_Tieoff->TIEOFFREG[80 + (CPUID << 1)],
			  0xFFFF0200 >> 2);
	}
}
#endif

#endif // #if (MULTICORE_BRING_UP == 1)

struct NX_SubCPUBringUpInfo {
	volatile U32 JumpAddr;
	volatile U32 CPUID;
	volatile U32 WakeupFlag;
};
void SwitchToEL2(void);
void SetGIC_All(void);
#define CPU_ALIVE_FLAG_ADDR 0xC0010230
void SubCPUBoot(U32 CPUID)
{
	register struct NX_SubCPUBringUpInfo *pCPUStartInfo =
	    (struct NX_SubCPUBringUpInfo *)CPU_ALIVE_FLAG_ADDR;

	SetGIC_All();
	WriteIO32(&pReg_GIC400->GICD.ISENABLER[0], 0xFF); // enable sgi all
	WriteIO32(&pReg_ClkPwr->CPUPOWERONREQ,
		  0x00); // clear own wakeup req bit

	//    printf("Hello World. I'm CPU %d!\r\n", CPUID);
	pCPUStartInfo->WakeupFlag = 1;
	DebugPutch('0' + CPUID);
#ifdef aarch64
	SwitchToEL2();
#endif
	do {
		register void (*pLaunch)(void);
		__asm__ __volatile__("wfi");

		//        WriteIO32(&pReg_GIC400->GICD.ICPENDR[0], 1<<CPUID);
		//        WriteIO32(&pReg_GIC400->GICC.EOIR,
		//        ReadIO32(&pReg_GIC400->GICC.IAR));
		//        __asm__ __volatile__ ("wfe");
		pLaunch = (void (*)(void))((MPTRS)pCPUStartInfo->JumpAddr);
		if ((MPTRS)pLaunch != (MPTRS)0xFFFFFFFF) {
			if (CPUID == pCPUStartInfo->CPUID)
				pLaunch();
		}
	} while (1);
}

//------------------------------------------------------------------------------
CBOOL SubCPUBringUp(U32 CPUID)
{
	register struct NX_SubCPUBringUpInfo *pCPUStartInfo =
	    (struct NX_SubCPUBringUpInfo *)CPU_ALIVE_FLAG_ADDR;
	S32 CPUNumber, retry = 0;
	S32 result = CPUID;

	WriteIO32(&pReg_GIC400->GICC.CTLR, 0x07); // enable cpu interface
	WriteIO32(&pReg_GIC400->GICC.PMR, 0xFF);  // all high priority
	WriteIO32(&pReg_GIC400->GICD.CTLR, 0x03); // distributor enable

	printf("Wakeup Sub CPU ");

#if (MULTICORE_BRING_UP == 1)

	pCPUStartInfo->JumpAddr =
	    (U32)0xFFFFFFFF; // set cpu jump info to invalid

	for (CPUNumber = 1; CPUNumber < 8;) {
		register volatile U32 delay;

		pCPUStartInfo->WakeupFlag = 0;
		delay = 0x10000;
		SetVectorLocation(CPUNumber, CTRUE); // CTRUE: High
						     // Vector(0xFFFF0000),
						     // CFALSE: Low Vector (0x0)
		BringUpSlaveCPU(CPUNumber);
		DMC_Delay(10000);
		while ((pCPUStartInfo->WakeupFlag == 0) && (--delay))
			;
		if (delay == 0) {
			if (retry > 3) {
				printf("maybe cpu %d is dead. -_-;\r\n",
				       CPUNumber);
				CPUNumber++; // try next cpu bringup
				retry = 0;
				result = CFALSE;
			} else {
				printf("cpu %d is not bringup, retry\r\n",
				       CPUNumber);
				retry++;
			}
		} else {
			retry = 0;
			result++;
			CPUNumber++; // try next cpu bringup
		}
		DMC_Delay(10000);
	}
#endif // #if (MULTICORE_BRING_UP == 1)

	printf("\r\nCPU Wakeup done! WFI is expected.\r\n");
	printf("CPU%d is Master!\r\n\n", CPUID);
	return result;
}
