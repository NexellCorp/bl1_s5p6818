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


#define __SET_GLOBAL_VARIABLES
#include "sysheader.h"
#include "nx_bootheader.h"

//#define SIMPLE_MEMTEST 			(1)

extern void DMC_Delay(int milisecond);

// extern void     flushICache(void);
// extern void     enableICache(CBOOL enable);

extern void enterSelfRefresh(void);
extern void exitSelfRefresh(void);
extern void set_bus_config(void);
extern void set_drex_qos(void);

extern CBOOL iUSBBOOT(struct NX_SecondBootInfo *const pTBI);
extern CBOOL iUARTBOOT(struct NX_SecondBootInfo *const pTBI);
extern CBOOL iSPIBOOT(struct NX_SecondBootInfo *const pTBI);
extern CBOOL iSDXCBOOT(struct NX_SecondBootInfo *const pTBI);
extern CBOOL iNANDBOOTEC(struct NX_SecondBootInfo *const pTBI);
extern CBOOL iSDXCFSBOOT(struct NX_SecondBootInfo *const pTBI);
extern void initClock(void);
#ifdef MEM_TYPE_DDR3
extern CBOOL ddr3_initialize(U32);
#endif
#ifdef MEM_TYPE_LPDDR23
extern CBOOL init_LPDDR3(U32);
#endif
extern CBOOL buildinfo(void);

extern void printClkInfo(void);

extern void ResetCon(U32 devicenum, CBOOL en);
extern CBOOL s5p6818_subcpu_bringup(U32 CPUID);

extern void initPMIC(void);
extern void s5p6818_resume(void);
extern void RomUSBBoot(U32 RomJumpAddr);
extern void secure_set_state(void);
extern int memtester_main(unsigned int start, unsigned int end);

extern int CRC_Check(void* buf, unsigned int size, unsigned int ref_crc);

void simple_memtest(U32 *pStart, U32 *pEnd);

#if defined(RAPTOR)
/*
 * Raptor board - revision check. (H/W: GPIOE 4,5,6)
 * GPIOE4 (Least Bit), GPIOE6(Most Bit)
 */
unsigned int raptor_check_hw_revision(void)
{
	volatile unsigned int *reg = 0;
	unsigned int val = 0;

	/*
	 * Check to GPIOE PORT
	 * Read to GPIOPAD Status Register
	 */
	reg = (volatile unsigned int*)(0xC001E000 + 0x18);
	val = (*reg & (0x7 << 4)) >> 4;

        return val;
}
#endif

/*
 * PWM Module Reset
 * This function is temporary code.
 */

void pwm_reset(void)
{
	int reset_number = RESETINDEX_OF_PWM_MODULE_PRESETn;

	ResetCon(reset_number, CTRUE);	// reset on
	ResetCon(reset_number, CFALSE); // reset negate
}

/*
 * TIMER Module Reset
 * This function is temporary code.
 */

void timer_reset(void)
{
	int reset_number = RESETINDEX_OF_TIMER_MODULE_PRESETn;

	ResetCon(reset_number, CTRUE);	// reset on
	ResetCon(reset_number, CFALSE); // reset negate
}

void device_set_env(void)
{
	/* (Device Port Number) for U-BOOT  */
	unsigned int dev_portnum = pSBI->DBI.SDMMCBI.PortNumber;
	WriteIO32(&pReg_ClkPwr->SCRATCH[1], dev_portnum );
}

//------------------------------------------------------------------------------
#if (CCI400_COHERENCY_ENABLE == 1)
void initCCI400(void)
{
	// before set barrier instruction.
	SetIO32(&pReg_CCI400->SCR, 1 << 0); // static bus disable speculative fetches
	SetIO32(&pReg_CCI400->SCR, 1 << 1); // SFR bus disable speculative fetches

	WriteIO32(&pReg_CCI400->COR, (1UL << 3)); // protect to send barrier command to drex

	WriteIO32(&pReg_CCI400->CSI[BUSID_CS].SCR, 0); // snoop request disable
	WriteIO32(&pReg_CCI400->CSI[BUSID_CODA].SCR,
		  0);					// snoop request disable
	WriteIO32(&pReg_CCI400->CSI[BUSID_TOP].SCR, 0); // snoop request disable

#if (MULTICORE_BRING_UP == 1)
	WriteIO32(&pReg_CCI400->CSI[BUSID_CPUG0].SCR,
		  0x3); // cpu 0~3 Snoop & DVM Req
	while (ReadIO32(&pReg_CCI400->STSR) & 0x1);

	WriteIO32(&pReg_CCI400->CSI[BUSID_CPUG1].SCR, 0x3); // cpu 4~7 Snoop & DVM Req
	while (ReadIO32(&pReg_CCI400->STSR) & 0x1);
#else
	WriteIO32(&pReg_CCI400->CSI[BUSID_CPUG0].SCR, 0x0);
	WriteIO32(&pReg_CCI400->CSI[BUSID_CPUG1].SCR, 0x0);
#endif
}
#endif // #if (CCI400_COHERENCY_ENABLE == 1)

struct NX_CLKPWR_RegisterSet *const clkpwr;

//------------------------------------------------------------------------------
void BootMain(U32 CPUID)
{
	struct NX_SecondBootInfo TBI;
	struct NX_SecondBootInfo *pTBI = &TBI; // third boot info
	CBOOL Result = CFALSE;
	register volatile U32 temp;
	U32 signature, is_resume = 0;
	U32 serial_ch = 0;

#if defined(RAPTOR_PMIC_INIT) || defined(AVN_PMIC_INIT)
	serial_ch = 3;
#endif

	/* setp 01. set the ema for sram and instruction-cache */
	cache_setup_ema();
	
#if 0	/* (early) low level - log message */
	/* stepxx. serial console(uartX) initialize. */
	serial_init(serial_ch);
#endif

	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);
	WriteIO32(&pReg_Alive->VDDCTRLSETREG, 0x000003FC); //; Retention off (Pad hold off)

	/* step xx. check the suspend, resume */
	is_resume = s5p6818_resume_check();

	/*
	 * SD/MMC,SPI - port number stored for u-boot.
	 */
	device_set_env();

//--------------------------------------------------------------------------
// Initialize PMIC device.
//--------------------------------------------------------------------------
#if defined(INITPMIC_YES)
	initPMIC();
#endif
	//--------------------------------------------------------------------------
	// Change to PLL.
	//--------------------------------------------------------------------------
	initClock();

	/* stepxx. serial console(uartX) initialize. */
	serial_init(serial_ch);

	/* stepxx. display the ema(extra margin adjustments) information. */
	ema_information();

	//--------------------------------------------------------------------------
	// build information. version, build time and date
	//--------------------------------------------------------------------------
#if 1
	buildinfo();
#else
	if (buildinfo() == CFALSE) {
		printf("WARNING : NSIH mismatch...!!!\r\n");
		while (1);
	}
#endif

	//--------------------------------------------------------------------------
	// print clock information
	//--------------------------------------------------------------------------
	printClkInfo();

	SYSMSG("\r\nDDR3 POR Init Start %d\r\n", is_resume);
#ifdef MEM_TYPE_DDR3
#if 0
	if (ddr3_initialize(is_resume) == CFALSE)
		ddr3_initialize(is_resume);
#else
	/*
	 * DDR initialization fails, a temporary code
	 * code for the defense.
	 */
	int ddr_retry = 0;
	while (ddr3_initialize(0) == CFALSE) {
		ddr_retry++;
		if (ddr_retry > 3) {
			printf("Memory Initialize Retry : %d \r\n", ddr_retry);
			printf("Memory Initializa or Calibration Failed! \r\n");
			break;
		}
	}
#endif
#endif
#ifdef MEM_TYPE_LPDDR23
	if (init_LPDDR3(0) == CFALSE)
		init_LPDDR3(0);
#endif

	if (is_resume)
		exit_self_refresh();

	SYSMSG("DDR3 Init Done!\r\n");

	set_bus_config();
	set_drex_qos();
	/* Temporary Code - PWM Reset */
	pwm_reset();
	/* Temporary Code - Timer Reset */
	timer_reset();

#if (CCI400_COHERENCY_ENABLE == 1)
	SYSMSG("CCI Init!\r\n");
	initCCI400();
#endif

#if (SUPPORT_KERNEL_3_4 == 0)
	secure_set_state();
#endif

#if (MULTICORE_BRING_UP == 1)
	s5p6818_subcpu_bringup(CPUID);
#endif

	if (is_resume) {
		SYSMSG(" DDR3 SelfRefresh exit Done!\r\n0x%08X\r\n", 
			ReadIO32(&pReg_Alive->WAKEUPSTATUS));
		s5p6818_resume();
	}
	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0);

	if (pSBI->SIGNATURE != HEADER_ID)
		printf("2nd Boot Header is invalid, Please check it out!\r\n");

#ifdef SIMPLE_MEMTEST
	simple_memtest((U32 *)0x40000000UL, (U32 *)0xBFFF0000);
#endif

	switch (pSBI->DBI.SPIBI.LoadDevice) {
#if defined(SUPPORT_USB_BOOT)
	case BOOT_FROM_USB:
		SYSMSG("Loading from usb...\r\n");
		Result = iUSBBOOT(pTBI); // for USB boot
		break;
#endif

#if defined(SUPPORT_SPI_BOOT)
	case BOOT_FROM_SPI:
		SYSMSG("Loading from spi...\r\n");
		Result = iSPIBOOT(pTBI); // for SPI boot
		break;
#endif

#if defined(SUPPORT_NAND_BOOT)
	case BOOT_FROM_NAND:
		SYSMSG( "Loading from nand...\r\n" );
		Result = iNANDBOOTEC(pTBI);     // for NAND boot
		break;
#endif

#if defined(SUPPORT_SDMMC_BOOT)
	case BOOT_FROM_SDMMC:
		SYSMSG("Loading from sdmmc...\r\n");
		Result = iSDXCBOOT(pTBI); // for SD boot
		break;
#endif

#if defined(SUPPORT_SDFS_BOOT)
	case BOOT_FROM_SDFS:
		SYSMSG("Loading from sd FATFS...\r\n");
		Result = iSDXCFSBOOT(pTBI); // for SDFS boot
		break;
#endif

#if defined(SUPPORT_UART_BOOT)
	case BOOT_FROM_UART:
		SYSMSG("Loading from uart...\r\n");
		Result = iUARTBOOT(pTBI);       // for UART boot
		break;
#endif
	}

#ifdef CRC_CHECK_ON
	Result = crc_check((void*)pTBI->LOADADDR, (unsigned int)pTBI->LOADSIZE,
			   (unsigned int)pTBI->DBI.SDMMCBI.CRC32);
#endif
	if (Result) {
	#if (SUPPORT_KERNEL_3_4 == 1)
		void (*pLaunch)(U32,U32) = (void(*)(U32,U32))((MPTRS)pTBI->LAUNCHADDR);
	#else
		struct nx_tbbinfo *tbi = (struct nx_tbbinfo *)&TBI;
		void (*pLaunch)() = (void (*)())(tbi->startaddr);
	#endif
		SYSMSG(" Image Loading Done!\r\n");
		SYSMSG("Launch to 0x%08X\r\n", (MPTRS)pLaunch);
		temp = 0x10000000;

		while (!serial_done() && temp--);
		pLaunch(0, 4330);
	}

	printf(" Image Loading Failure Try to USB boot\r\n");
	temp = 0x10000000;
	while (!serial_done() && temp--);
	RomUSBBoot((U32)0x0000009C);
	while (1);
}
