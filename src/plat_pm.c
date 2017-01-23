/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: DeokJin, Lee <truevirtue@nexell.co.kr>
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
#include <sysheader.h>
#include <nx_bootheader.h>

#define ATF_HEADER_SIZE		(1024)

/* External function */
extern unsigned int calc_crc(void *addr, int len);

extern void DMC_Delay(int milisecond);

extern void enter_self_refresh(void);
extern void exit_self_refresh(void);

/* External Variable */
extern unsigned int g_WR_lvl;
extern unsigned int g_GT_cycle;
extern unsigned int g_GT_code;
extern unsigned int g_RD_vwmc;
extern unsigned int g_WR_vwmc;

/*************************************************************
 * Before entering suspend and Mark the location and promise Kernel.
 * Reference CRC, Jump Address, Memory Address(CRC), Size(CRC)
 *************************************************************/
void suspend_mark(unsigned int state, unsigned int entrypoint, unsigned int crc,
						unsigned int mem, unsigned int size)
{
#if 0	// Not determined mark parmeter
	unsigned int crc;
	unsigned int mem = 0x40000000, size = (512*1024*1024);
#endif
	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 1);

	mmio_write_32(&pReg_Alive->ALIVESCRATCHRSTREG, 0xFFFFFFFF);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST1, 0xFFFFFFFF);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST2, 0xFFFFFFFF);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST3, 0xFFFFFFFF);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST4, 0xFFFFFFFF);

	crc = calc_crc((void *)mem, size);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSETREG, SUSPEND_SIGNATURE);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET1, entrypoint);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET2, crc);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET3, mem);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET4, size);
}

int s5p6818_resume_check(void)
{
	int signature;
	int is_resume = 0;		// 0: no resume, 1:resume

	/* Get resume information. */

	/* Kernel */
	signature = mmio_read_32(&pReg_Alive->ALIVESCRATCHREADREG);
	if ((SUSPEND_SIGNATURE == (signature & 0xFFFFFF00))
		&& mmio_read_32(&pReg_Alive->WAKEUPSTATUS)) {
		is_resume = 1;
	}

	/* Arm Trusted Firmware */
	signature = mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE4);
	if ((ATF_SUSPEND_SIGNATURE == (signature & 0xFFFFFF00))
		&& mmio_read_32(&pReg_Alive->WAKEUPSTATUS)) {
		is_resume= 1;
	}

	return is_resume;
}

void s5p6818_resume(void)
{
	unsigned int function, physical;
	unsigned int signature, atf_signature, cal_crc, crc, size;
	void (*JumpNextBoot)(void) = 0;

	/* for Kernel WakeUp */
	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 1);		// open alive power gate
	function  = mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE1 );
	signature = mmio_read_32(&pReg_Alive->ALIVESCRATCHREADREG);
	physical  = mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE3 );
	crc       = mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE2 );
	size      = mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE4 );
	JumpNextBoot = (void (*)(void))((MPTRS)function);

	/* for Arm Trusted Firmware WakeUp*/
	atf_signature = mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE4);

	if (SUSPEND_SIGNATURE == (signature & 0xFFFFFF00)) {
		/* Only Kernel */
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRSTREG, 0xFFFFFFFF);
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST1, 0xFFFFFFFF);
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST2, 0xFFFFFFFF);
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST3, 0xFFFFFFFF);
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST4, 0xFFFFFFFF);

		unsigned int ret = calc_crc((void *)((MPTRS)physical), size);

		printf("CRC: 0x%08X FN: 0x%08X phy: 0x%08X len: 0x%08X ret: 0x%08X\r\n",
			crc, function, physical, size, ret);
		if (function && (crc == ret)) {
			unsigned int timerout = 0x100000;
			printf("It's WARM BOOT\r\nJump to Kernel!\r\n");
			while (serial_busy() && timerout--);
			JumpNextBoot();
		}
	} else if (ATF_SUSPEND_SIGNATURE == (atf_signature & 0xFFFFFF00)) {
		/* Arm Trusted Firmware */
		struct nx_tbbinfo *header = (struct nx_tbbinfo*)mmio_read_32(&pReg_Alive->ALIVESCRATCHVALUE2);
		cal_crc = calc_crc((void *)((MPTRS)(header->loadaddr + ATF_HEADER_SIZE)), header->loadsize - ATF_HEADER_SIZE);
		printf("Calcurated CRC: 0x%08X, Normal CRC: 0x%08X \r\n", cal_crc, header->crc32);

		if (cal_crc == header->crc32) {
			unsigned int timerout = 0x100000;
			JumpNextBoot = (void (*)(void))((MPTRS)((header->startaddr) & 0xFFFFFFFF));
			printf("Jump to 0x%08X\r\n", JumpNextBoot);
			printf("It's WARM BOOT\r\nJump to ATF!\r\n");
			while (serial_busy() && timerout--);
			JumpNextBoot();
		}

		/* CRC Fail : Clear Scratch */
		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST4, 0xFFFFFFFF);
//		mmio_write_32(&pReg_Alive->ALIVESCRATCHRST2, 0xFFFFFFFF);
		printf("CRC check failed. Go to COLD BOOT.\r\n");
	} else {
		printf("General Suspend Signature is different.\r\nRead Signature :0x%08X\r\n", signature);
	}

	printf("It's COLD BOOT\r\n");
}

static void suspend_vdd_pwroff(void)
{
	mmio_write_32(&pReg_ClkPwr->CPUWARMRESETREQ, 0x0);			//; clear for reset issue.

//	mmio_clear_32( &pReg_ClkPwr->PWRCONT, (0x3FFFF << 8));		//; Clear USE_WFI & USE_WFE bits for STOP mode.
	mmio_set_32(&pReg_ClkPwr->PWRCONT, 0xF << 12 | 0xF << 20);		// stop mode needs all cpu wfi
	mmio_clear_32(&pReg_ClkPwr->PWRCONT, 0xF << 8 | 0xF << 16);		// stop mode does not need all cpu wfe

#if (MULTICORE_SLEEP_CONTROL == 1)
	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 0x00000001);		//; alive power gate open

	/* Save leveling & training values. */
#if 1
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST5, 0xFFFFFFFF);		// clear - ctrl_shiftc
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST6, 0xFFFFFFFF);		// clear - ctrl_offsetC
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST7, 0xFFFFFFFF);		// clear - ctrl_offsetr
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST8, 0xFFFFFFFF);		// clear - ctrl_offsetw

	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET5, g_GT_cycle);		// store - ctrl_shiftc
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET6, g_GT_code);		// store - ctrl_offsetc
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET7, g_RD_vwmc);		// store - ctrl_offsetr
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET8, g_WR_vwmc);		// store - ctrl_offsetw
#endif

	mmio_write_32(&pReg_Alive->VDDOFFCNTVALUERST, 0xFFFFFFFF);		// clear delay counter, refrence rtc clock
	mmio_write_32(&pReg_Alive->VDDOFFCNTVALUESET, 0x00000001);		// set minimum delay time for VDDPWRON pin. 1 cycle per 32.768Kh (about 30us)

	if (mmio_read_32(&pReg_Alive->ALIVEGPIODETECTPENDREG)) {
		__asm__ __volatile__("wfi");					// now real entering point to stop mode.
	} else {
		mmio_write_32(&pReg_Alive->VDDCTRLSETREG, 0x000003FC);		// Retention off (Pad hold off)
		mmio_write_32(&pReg_Alive->VDDCTRLRSTREG, 0x00000001);		// vddpoweron off, start counting down.
//        	mmio_write_32( &pReg_Alive->VDDCTRLRSTREG, 0x000003FD);	// vddpoweron off, start counting down.

		mmio_write_32(&pReg_Alive->ALIVEGPIODETECTPENDREG, 0xFF);	// all alive pend pending clear until power down.
		DMC_Delay(600);							// 600 : 110us, Delay for Pending Clear. When CPU clock is 400MHz, this value is minimum delay value.
//        	DMC_Delay(200);

//		mmio_write_32( &pReg_Alive->ALIVEPWRGATEREG,    0x00000000);// alive power gate close

		while (1) {
//            		mmio_set_32  (&pReg_ClkPwr->PWRMODE, (0x1 << 1));	// enter STOP mode.
			mmio_write_32(&pReg_ClkPwr->PWRMODE, (0x1 << 1));	// enter STOP mode.
			__asm__ __volatile__("wfi");				// now real entering point to stop mode.
		}
	}

#else  // #if (MULTICORE_SLEEP_CONTROL == 1)

	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 0x00000001);		// alive power gate open

	mmio_write_32(&pReg_Alive->VDDOFFCNTVALUERST, 0xFFFFFFFF);		// clear delay counter, refrence rtc clock
	mmio_write_32(&pReg_Alive->VDDOFFCNTVALUESET, 0x00000001);		// set minimum delay time for VDDPWRON pin. 1 cycle per 32.768Kh (about 30us)

	mmio_write_32(&pReg_Alive->VDDCTRLRSTREG, 0x00000001);			// vddpoweron off, start counting down.
	DMC_Delay(220);

	mmio_write_32(&pReg_Alive->ALIVEGPIODETECTPENDREG, 0xFF);		// all alive pend pending clear until power down.
	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 0x00000000);		// alive power gate close

	mmio_write_32(&pReg_ClkPwr->CPUWARMRESETREQ, 0x0); //; clear for reset issue.

	while (1) {
//		mmio_set_32  ( &pReg_ClkPwr->PWRMODE, (0x1 << 1) ); 		//; enter STOP mode.
		mmio_write_32(&pReg_ClkPwr->PWRMODE, (0x1 << 1));		//; enter STOP mode.
		__asm__ __volatile__("wfi"); //; now real entering point to stop mode.
	}
#endif // #if (MULTICORE_SLEEP_CONTROL == 1)
}

void s5p6818_suspend(void)
{
	unsigned int temp;

	do {
		temp = mmio_read_32(&pReg_Tieoff->TIEOFFREG[90]) & 0xF;
		temp |= (mmio_read_32(&pReg_Tieoff->TIEOFFREG[107]) & 0xF) << 4;	// wait for sub CPU WFI signal
	} while (temp != 0xFE); // except core 0


	mmio_clear_32(&pReg_Tieoff->TIEOFFREG[76], (7 << 6)); 			// Lock Drex port
	do {
		temp = mmio_read_32(&pReg_Tieoff->TIEOFFREG[76]) &
		       ((1 << 24) | (1 << 21) | (1 << 18));
	} while (temp);

	NOTICE("enter the self refresh !!\r\n");
	while (!serial_empty());
	while (serial_busy());
	enter_self_refresh();

	/* VDD Power OFF */
	suspend_vdd_pwroff();

	exit_self_refresh();
	DMC_Delay(50);
	NOTICE("exit the self refresh !!\r\n");

	mmio_set_32(&pReg_Tieoff->TIEOFFREG[76], (7 << 6));			// Unlock Drex port
	do {
		temp = mmio_read_32(&pReg_Tieoff->TIEOFFREG[76]) &
		       ((1 << 24) | (1 << 21) | (1 << 18));
	} while (temp != ((1 << 24) | (1 << 21) | (1 << 18)));
}
