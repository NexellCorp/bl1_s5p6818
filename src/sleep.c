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
#include "nx_bootheader.h"

#define ATF_HEADER_SIZE		(1024)

extern void DMC_Delay(int milisecond);

extern U32 iget_fcs(U32 fcs, U32 data);
extern U32 __calc_crc(void *addr, int len);

extern void enterSelfRefresh(void);
extern void exitSelfRefresh(void);

extern U32 g_WR_lvl;
extern U32 g_GT_cycle;
extern U32 g_GT_code;
extern U32 g_RD_vwmc;
extern U32 g_WR_vwmc;

void dowakeup(void)
{
	unsigned int function, physical;
	unsigned int signature, atf_signature, cal_crc, crc, size;
	void (*JumpNextBoot)(void) = 0;

	/* for Kernel WakeUp */
	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);		// open alive power gate
	function  = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE1 );
	signature = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG);
	physical  = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE3 );
	crc       = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE2 );
	size      = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE4 );
	JumpNextBoot = (void (*)(void))((MPTRS)function);

	/* for Arm Trusted Firmware WakeUp*/
	atf_signature = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE4);

	if (SUSPEND_SIGNATURE == (signature & 0xFFFFFF00)) {
		/* Only Kernel */
		WriteIO32(&pReg_Alive->ALIVESCRATCHRSTREG, 0xFFFFFFFF);
		WriteIO32(&pReg_Alive->ALIVESCRATCHRST1, 0xFFFFFFFF);
		WriteIO32(&pReg_Alive->ALIVESCRATCHRST2, 0xFFFFFFFF);
		WriteIO32(&pReg_Alive->ALIVESCRATCHRST3, 0xFFFFFFFF);
		WriteIO32(&pReg_Alive->ALIVESCRATCHRST4, 0xFFFFFFFF);

		U32 ret = __calc_crc((void *)((MPTRS)physical), size);

		printf("CRC: 0x%08X FN: 0x%08X phy: 0x%08X len: 0x%08X ret: 0x%08X\r\n",
			crc, function, physical, size, ret);
		if (function && (crc == ret)) {
			U32 timerout = 0x100000;
			printf("It's WARM BOOT\r\nJump to Kernel!\r\n");
			while (DebugIsBusy() && timerout--);
			JumpNextBoot();
		}
	} else if (ATF_SUSPEND_SIGNATURE == (atf_signature & 0xFFFFFF00)) {
		/* Arm Trusted Firmware */
		struct nx_tbbinfo *header = (struct nx_tbbinfo*)ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE2);
		cal_crc = __calc_crc((void *)((MPTRS)(header->loadaddr + ATF_HEADER_SIZE)), header->loadsize - ATF_HEADER_SIZE);
		printf("Calcurated CRC: 0x%08X, Normal CRC: 0x%08X \r\n", cal_crc, header->crc32);

		if (cal_crc == header->crc32) {
			U32 timerout = 0x100000;
			JumpNextBoot = (void (*)(void))((MPTRS)((header->startaddr) & 0xFFFFFFFF));
			printf("Jump to 0x%08X\r\n", JumpNextBoot);
			printf("It's WARM BOOT\r\nJump to ATF!\r\n");
			while (DebugIsBusy() && timerout--);
			JumpNextBoot();
		}

		/* CRC Fail : Clear Scratch */
		WriteIO32(&pReg_Alive->ALIVESCRATCHRST4, 0xFFFFFFFF);
//		WriteIO32(&pReg_Alive->ALIVESCRATCHRST2, 0xFFFFFFFF);
		printf("CRC check failed. Go to COLD BOOT.\r\n");
	} else {
		printf("General Suspend Signature is different.\r\nRead Signature :0x%08X\r\n", signature);
	}

	printf("It's COLD BOOT\r\n");
}

void vddPowerOff(void)
{
	WriteIO32(&pReg_ClkPwr->CPUWARMRESETREQ, 0x0); //; clear for reset issue.

//	ClearIO32( &pReg_ClkPwr->PWRCONT, (0x3FFFF << 8));	//; Clear USE_WFI & USE_WFE bits for STOP mode.
	SetIO32(&pReg_ClkPwr->PWRCONT, 0xF << 12 | 0xF << 20); // stop mode needs all cpu wfi
	ClearIO32(&pReg_ClkPwr->PWRCONT, 0xF << 8 | 0xF << 16); // stop mode does not need all cpu wfe

#if (MULTICORE_SLEEP_CONTROL == 1)
	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0x00000001); //; alive power gate open
	//----------------------------------
	// Save leveling & training values.
#if 1
	WriteIO32(&pReg_Alive->ALIVESCRATCHRST5, 0xFFFFFFFF); // clear - ctrl_shiftc
	WriteIO32(&pReg_Alive->ALIVESCRATCHRST6, 0xFFFFFFFF); // clear - ctrl_offsetC
	WriteIO32(&pReg_Alive->ALIVESCRATCHRST7, 0xFFFFFFFF); // clear - ctrl_offsetr
	WriteIO32(&pReg_Alive->ALIVESCRATCHRST8, 0xFFFFFFFF); // clear - ctrl_offsetw

	WriteIO32(&pReg_Alive->ALIVESCRATCHSET5, g_GT_cycle); // store - ctrl_shiftc
	WriteIO32(&pReg_Alive->ALIVESCRATCHSET6, g_GT_code); // store - ctrl_offsetc
	WriteIO32(&pReg_Alive->ALIVESCRATCHSET7, g_RD_vwmc); // store - ctrl_offsetr
	WriteIO32(&pReg_Alive->ALIVESCRATCHSET8, g_WR_vwmc); // store - ctrl_offsetw
#endif

	WriteIO32(&pReg_Alive->VDDOFFCNTVALUERST, 0xFFFFFFFF); //; clear delay counter, refrence rtc clock
	WriteIO32(&pReg_Alive->VDDOFFCNTVALUESET, 0x00000001); //; set minimum delay time for VDDPWRON pin. 1 cycle per 32.768Kh (about 30us)

	if (ReadIO32(&pReg_Alive->ALIVEGPIODETECTPENDREG)) {
		__asm__ __volatile__("wfi"); //; now real entering point to stop mode.
	} else {
		WriteIO32(&pReg_Alive->VDDCTRLSETREG, 0x000003FC); //; Retention off (Pad hold off)
		WriteIO32(&pReg_Alive->VDDCTRLRSTREG, 0x00000001); //; vddpoweron off, start counting down.
//        	WriteIO32( &pReg_Alive->VDDCTRLRSTREG, 0x000003FD); //; vddpoweron off, start counting down.

		WriteIO32(&pReg_Alive->ALIVEGPIODETECTPENDREG, 0xFF);	//; all alive pend pending clear until power down.
		DMC_Delay(600);						// 600 : 110us, Delay for Pending Clear. When CPU clock is 400MHz, this value is minimum delay value.
//        	DMC_Delay(200);

//		WriteIO32( &pReg_Alive->ALIVEPWRGATEREG,    0x00000000);       //; alive power gate close

		while (1) {
//            		SetIO32  (&pReg_ClkPwr->PWRMODE, (0x1 << 1));	//; enter STOP mode.
			WriteIO32(&pReg_ClkPwr->PWRMODE, (0x1 << 1));	//; enter STOP mode.
			__asm__ __volatile__("wfi"); //; now real entering point to stop mode.
		}
	}

#else  // #if (MULTICORE_SLEEP_CONTROL == 1)

	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0x00000001); //; alive power gate open

	WriteIO32(&pReg_Alive->VDDOFFCNTVALUERST, 0xFFFFFFFF); //; clear delay counter, refrence rtc clock
	WriteIO32(&pReg_Alive->VDDOFFCNTVALUESET, 0x00000001); //; set minimum delay time for VDDPWRON pin. 1 cycle per 32.768Kh (about 30us)

	WriteIO32(&pReg_Alive->VDDCTRLRSTREG, 0x00000001); //; vddpoweron off, start counting down.
	DMC_Delay(220);

	WriteIO32(&pReg_Alive->ALIVEGPIODETECTPENDREG, 0xFF); //; all alive pend pending clear until power down.
	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0x00000000); //; alive power gate close

	WriteIO32(&pReg_ClkPwr->CPUWARMRESETREQ, 0x0); //; clear for reset issue.

	while (1) {
//		SetIO32  ( &pReg_ClkPwr->PWRMODE, (0x1 << 1) );  //; enter STOP mode.
		WriteIO32(&pReg_ClkPwr->PWRMODE, (0x1 << 1)); //; enter STOP mode.
		__asm__ __volatile__("wfi"); //; now real entering point to stop mode.
	}
#endif // #if (MULTICORE_SLEEP_CONTROL == 1)
}

void sleepMain(void)
{
	U32 temp;

#if 1
	do {
		temp = ReadIO32(&pReg_Tieoff->TIEOFFREG[90]) & 0xF;
		temp |= (ReadIO32(&pReg_Tieoff->TIEOFFREG[107]) & 0xF)
			<< 4;   // wait for sub CPU WFI signal
	} while (temp != 0xFE); // except core 0
#endif

#if 0
	WriteIO32( &clkpwr->PLLSETREG[1],   0x100CC801 );       //; set PLL1 - 800Mhz
	//    WriteIO32( &clkpwr->PLLSETREG[1],   0x100CC802 );       //; set PLL1 - 400Mhz

	__pllchange(clkpwr->PWRMODE | 0x1<<15, &clkpwr->PWRMODE, 0x20000); //533 ==> 800MHz:#0xED00, 1.2G:#0x17000, 1.6G:#0x1E000

	{
		volatile U32 delay = 0x100000;
		while((clkpwr->PWRMODE & 0x1<<15) && delay--);    // it's never checked here, just for insure
		if( clkpwr->PWRMODE & 0x1<<15 )
		{
			//            printf("pll does not locked\r\nsystem halt!\r\r\n");    // in this point, it's not initialized uart debug port yet
			while(1);        // system reset code need.
		}
	}
#endif

	ClearIO32(&pReg_Tieoff->TIEOFFREG[76], (7 << 6)); //; Lock Drex port
	do {
		temp = ReadIO32(&pReg_Tieoff->TIEOFFREG[76]) &
		       ((1 << 24) | (1 << 21) | (1 << 18));
	} while (temp);

	printf("enter self refresh\r\n");
	while (!DebugIsTXEmpty());
	while (DebugIsBusy());
	enterSelfRefresh();

	vddPowerOff();

	exitSelfRefresh();
	DMC_Delay(50);

	SetIO32(&pReg_Tieoff->TIEOFFREG[76], (7 << 6)); //; Unlock Drex port
	do {
		temp = ReadIO32(&pReg_Tieoff->TIEOFFREG[76]) &
		       ((1 << 24) | (1 << 21) | (1 << 18));
	} while (temp != ((1 << 24) | (1 << 21) | (1 << 18)));
}
