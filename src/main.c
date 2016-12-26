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
#define __SET_GLOBAL_VARIABLES
#include <sysheader.h>
#include <main.h>
#include "nx_bootheader.h"

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

/* s5p6818 - pwm module reset */
void s5p6818_pwm_reset(void)
{
	int reset_number = RESETINDEX_OF_PWM_MODULE_PRESETn;

	ResetCon(reset_number, CTRUE);	// reset on
	ResetCon(reset_number, CFALSE); // reset negate
}

/* s5p6818 - timer module reset */
void s5p6818_timer_reset(void)
{
	int reset_number = RESETINDEX_OF_TIMER_MODULE_PRESETn;

	ResetCon(reset_number, CTRUE);	// reset on
	ResetCon(reset_number, CFALSE); // reset negate
}

static void s5p6818_set_device_env(void)
{
	/* (device port number) for u-boot  */
	mmio_write_32(&pReg_ClkPwr->SCRATCH[1], (pSBI->DBI.SDMMCBI.PortNumber));
}

void main(unsigned int cpu_id)
{
	struct NX_SecondBootInfo TBI;
	struct NX_SecondBootInfo *pTBI = &TBI; // third boot info
	unsigned int serial_ch = 0;
	unsigned int is_resume = 0, temp;
	int ret = 0;

#if defined(RAPTOR) || defined(AVN)
	serial_ch = 3;
#endif

	/* setp 01. set the ema for sram and instruction-cache */
	cache_setup_ema();

	/* step xx. remove the warining message */
	cpu_id = cpu_id;

#if 1	/* (early) low level - log message */
	/* step xx. serial console(uartX) initialize. */
	serial_init(serial_ch);
#endif

	/* step xx. alive pad - hold off  */
	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 1);
#if (SUPPORT_KERNEL_3_4 == 0)
	mmio_write_32(&pReg_Alive->VDDCTRLSETREG, 0x000003FC);			//; Retention off (Pad hold off)
#endif

	/* step xx. check the suspend, resume */
	is_resume = s5p6818_resume_check();

	/* step 02. set the pmic(power management ic) */
#if defined(INITPMIC_YES)
	initPMIC();
#endif

	/* step 03. clock(pll) intialize */
	clock_initialize();

	/* step 04. serial console(uartX) initialize. */
	serial_init(serial_ch);

	/* step xx. display the ema(extra margin adjustments) information. */
	ema_information();

	/* step xx. build information. version, build time and date */
	if (build_information() < 0)
		WARN("NSIH Version(or File) Mismatch...!!\r\n");

	/* step xx. display the clock information */
	clock_information();

	/* step 05. (ddr3/lpddr3) sdram memory initialize */
	SYSMSG("\r\n(LPDDR3/DDR3) POR Intialize Start!! (%d)\r\n", is_resume);

#ifdef MEM_TYPE_DDR3
	/*
	 * DDR initialization fails, a temporary code
	 * code for the defense.
	 */
	int mem_retry = 3;
	while (mem_retry--) {
		if (ddr3_initialize(is_resume) >= 0)
			break;
	}
#endif	// #ifdef MEM_TYPE_DDR3

#ifdef MEM_TYPE_LPDDR23
	if (init_LPDDR3(0) == CFALSE)
		init_LPDDR3(0);
#endif	// #ifdef MEM_TYPE_LPDDR23

	/* step 06-1. exit the (sdram) self-refresh  */
	if (is_resume)
		exit_self_refresh();

	SYSMSG("(LPDDR3/DDR3) Initialize Done!\r\n");

	/* step 07-1. set the system bus configuration */
	set_bus_config();
	/* step 07-2. set the drex configuration */
	set_drex_qos();

#if (SUPPORT_KERNEL_3_4 == 0)
	/* step xx. s5p6818 - device reset (temporary) */
	s5p6818_pwm_reset();
	s5p6818_timer_reset();
#endif

	/* step 08-1. cci400(fast bclk) initialize */
#if (CCI400_COHERENCY_ENABLE == 1)
	cci400_initialize();
	NOTICE("CCI400 Initialize!\r\n");
#endif

	/* step 08-2. set the secure(tzpc, gic) configuration */
#if (SUPPORT_KERNEL_3_4 == 0)
	secure_set_state();
#endif

	/* step 08-3. set the secondary-core */
#if (MULTICORE_BRING_UP == 1)
	s5p6818_subcpu_bringup(cpu_id);
#endif

#if (SUPPORT_KERNEL_3_4 == 0)
	/* step xx. (sd/mmc, spi) - port number stored for u-boot. */
	s5p6818_set_device_env();
#endif

	/* step 09. s5p6818 resume sequence */
	if (is_resume) {
		NOTICE("(LPDDR3/DDR3) Self-Refresh Exit Done!(0x%08X)\r\n",
			mmio_read_32(&pReg_Alive->WAKEUPSTATUS));
		s5p6818_resume();
	}
	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 0);

	/* step xx. check the nsih header id */
	if (pSBI->SIGNATURE != HEADER_ID)
		ERROR("2nd boot header is invalid, please check it out!\r\n");

	/* step xx. check the memory test (optional) */
#ifdef SIMPLE_MEMTEST
	simple_memtest((unsigned int *)0x40000000UL, (unsigned int *)0xBFFF0000);
#endif

	/*
	  * step 10-1. check the (thirdboot) boot mode
	  * step 10-2. loading the next file (thirdboot)
	  */
	switch (pSBI->DBI.SPIBI.LoadDevice) {
#if defined(SUPPORT_USB_BOOT)
	case BOOT_FROM_USB:
		SYSMSG("Loading from usb...\r\n");
		ret = iUSBBOOT(pTBI);		// for USB boot
		break;
#endif

#if defined(SUPPORT_SPI_BOOT)
	case BOOT_FROM_SPI:
		SYSMSG("Loading from spi...\r\n");
		ret = iSPIBOOT(pTBI);		// for SPI boot
		break;
#endif

#if defined(SUPPORT_NAND_BOOT)
	case BOOT_FROM_NAND:
		SYSMSG( "Loading from nand...\r\n" );
		ret = iNANDBOOTEC(pTBI);	// for NAND boot
		break;
#endif

#if defined(SUPPORT_SDMMC_BOOT)
	case BOOT_FROM_SDMMC:
		SYSMSG("Loading from sdmmc...\r\n");
		ret = iSDXCBOOT(pTBI);		// for SD boot
		break;
#endif

#if defined(SUPPORT_SDFS_BOOT)
	case BOOT_FROM_SDFS:
		SYSMSG("Loading from sd FATFS...\r\n");
		ret = iSDXCFSBOOT(pTBI);	// for SDFS boot
		break;
#endif
	}

	/* step xx. check the memory crc check (optional) */
#ifdef CRC_CHECK_ON
	ret = crc_check((void*)pTBI->LOADADDR, (unsigned int)pTBI->LOADSIZE,
				(unsigned int)pTBI->DBI.SDMMCBI.CRC32);
#endif
	/* step 11. jump the next bootloader (thirdboot) */
	if (ret) {
	#if (SUPPORT_KERNEL_3_4 == 1)
		void (*pLaunch)(unsigned int, unsigned int)
			= (void(*)(unsigned int, unsigned int))((MPTRS)pTBI->LAUNCHADDR);
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

	ERROR("Image Loading Failure Try to USB boot\r\n");
	temp = 0x10000000;
	while (!serial_done() && temp--);
	RomUSBBoot((unsigned int)0x0000009C);
	while (1);
}
