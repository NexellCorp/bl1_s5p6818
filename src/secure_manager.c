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

#include "secure_manager.h"

struct nx_tzpc_prot_bit {
	unsigned char port0 : 1;
	unsigned char port1 : 1;
	unsigned char port2 : 1;
	unsigned char port3 : 1;
	unsigned char port4 : 1;
	unsigned char port5 : 1;
	unsigned char port6 : 1;
	unsigned char port7 : 1;
};

const unsigned short tzpc_r0_size[NX_TZPC_MODULE_NUMBER] = {
	NX_TZPC_R0SIZE_ALL,	// size of secure mode management area of sram
	NX_TZPC_R0SIZE_NONE,	// no connection, don't care
	NX_TZPC_R0SIZE_NONE,	// no connection, don't care
	NX_TZPC_R0SIZE_NONE,	// no connection, don't care
	NX_TZPC_R0SIZE_NONE,	// no connection, don't care
	NX_TZPC_R0SIZE_NONE,	// no connection, don't care
	NX_TZPC_R0SIZE_NONE 	// no connection, don't care
};

const struct nx_tzpc_prot_bit prob_bit[NX_TZPC_MODULE_NUMBER * NX_TZPC_PORT] = {
	/* TZPC0 */
	{
		NX_TZPC_SMODE_NONSECURE, // ioperi_bus_m0
		NX_TZPC_SMODE_NONSECURE, // ioperi_bus_m1
		NX_TZPC_SMODE_NONSECURE, // ioperi_bus_m2
		NX_TZPC_SMODE_NONSECURE, // top_bus m0
		NX_TZPC_SMODE_NONSECURE, // static_bus m0		// mcus address area
		NX_TZPC_SMODE_NONSECURE, // static_bus m1		// sram address area
		NX_TZPC_SMODE_NONSECURE, // display_bus m0
		NX_TZPC_SMODE_NONSECURE  // bot_bus m0
	},
	{
		NX_TZPC_SMODE_NONSECURE, // sfr_bus m0
		NX_TZPC_SMODE_NONSECURE, // sfr_bus m1
		NX_TZPC_SMODE_NONSECURE, // sfr_bus m2
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // drex_0_secure_boot_lock
		0			 // GIC400	CFGSDISABLE: 1: lockdown
	},
	{
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m0
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m1
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m2
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m3
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m4
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m5
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus m6
		NX_TZPC_SMODE_SECURE     // NC
	},
	{
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	},
	/* TZPC1 */
	{
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m0
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m1
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m2
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m3
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m4
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m5
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus m6
		NX_TZPC_SMODE_NONSECURE  // mali400 XPROT
	},
	{
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	},
	{
		NX_TZPC_SMODE_NONSECURE, // sfr2_bus m0
		NX_TZPC_SMODE_NONSECURE, // sfr2_bus m1
		NX_TZPC_SMODE_NONSECURE, // sfr2_bus m2
		NX_TZPC_SMODE_NONSECURE, // sfr2_bus m3
		NX_TZPC_SMODE_NONSECURE, // sfr2_bus m4
		NX_TZPC_SMODE_NONSECURE, // sfr2_bus m5
		NX_TZPC_SMODE_NONSECURE, // display s0 XPROT
		NX_TZPC_SMODE_NONSECURE  // display s0 XPROT
	},
	{
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	},
	/* TZPC2 */
	{
		NX_TZPC_SMODE_NONSECURE, // sysctrltop	--- finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // tieoff --- finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // rstcon --- finally need to set secure
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_NONSECURE, // pdm
		NX_TZPC_SMODE_NONSECURE, // crypto
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_NONSECURE  // pwmtimer0
	},
	{
		NX_TZPC_SMODE_NONSECURE, // pwmtimer1
		NX_TZPC_SMODE_NONSECURE, // wdt	---finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // gpioA	---finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // gpioB	---finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // gpioC	---finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // gpioD	---finally need to set secure
		NX_TZPC_SMODE_NONSECURE, // gpioE	---finally need to set secure
		NX_TZPC_SMODE_SECURE     // NC
	},
	{
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_NONSECURE, // mcustop	// controll interface
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_NONSECURE, // adc
		NX_TZPC_SMODE_NONSECURE, // ppm
		NX_TZPC_SMODE_NONSECURE, // i2s0
		NX_TZPC_SMODE_NONSECURE, // i2s1
		NX_TZPC_SMODE_NONSECURE  // i2s2
	},
	{
		NX_TZPC_SMODE_NONSECURE, // ac97
		NX_TZPC_SMODE_NONSECURE, // spdiftx
		NX_TZPC_SMODE_NONSECURE, // spdifrx
		NX_TZPC_SMODE_NONSECURE, // ssp0
		NX_TZPC_SMODE_NONSECURE, // ssp1
		NX_TZPC_SMODE_NONSECURE, // mpegtsi
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_NONSECURE  // ssp2
	},
	/* TZPC3 */
	{
		NX_TZPC_SMODE_NONSECURE, // gmac gmac0 XPROT
		NX_TZPC_SMODE_NONSECURE, // gmac
		NX_TZPC_SMODE_NONSECURE, // sdmmc0
		NX_TZPC_SMODE_NONSECURE, // vip1 VIP001 XPROT
		NX_TZPC_SMODE_NONSECURE, // vip0 VIP000 XPROT
		NX_TZPC_SMODE_NONSECURE, // deinterlace DEINTERLACE XPROT
		NX_TZPC_SMODE_NONSECURE, // scaler SCALER XPROT
		NX_TZPC_SMODE_NONSECURE  // ecid		---finally need to set secure
	},
	{
		NX_TZPC_SMODE_NONSECURE, // sdmmc1
		NX_TZPC_SMODE_NONSECURE, // sdmmc2
		NX_TZPC_SMODE_NONSECURE, // usbhost_phy
		NX_TZPC_SMODE_NONSECURE, // usbhostclk
		NX_TZPC_SMODE_NONSECURE, // usbotg_phy
		NX_TZPC_SMODE_NONSECURE, // uart04
		NX_TZPC_SMODE_NONSECURE, // uart04clk
		NX_TZPC_SMODE_NONSECURE  // uart05
	},
	{
		NX_TZPC_SMODE_NONSECURE, // coda0
		NX_TZPC_SMODE_NONSECURE, // coda1
		NX_TZPC_SMODE_NONSECURE, // coda2
		NX_TZPC_SMODE_NONSECURE, // coda3
		NX_TZPC_SMODE_NONSECURE, // coda to bottom bus xiu
		NX_TZPC_SMODE_NONSECURE, // coda to ccibus xiu
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_NONSECURE  // CODA960 XPROT
	},
	{
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	},
	/* TZPC4 */
	{
		NX_TZPC_SMODE_NONSECURE, // ioperi_bus
		NX_TZPC_SMODE_NONSECURE, // bot_bus
		NX_TZPC_SMODE_NONSECURE, // top_bus
		NX_TZPC_SMODE_NONSECURE, // disp_bus
		NX_TZPC_SMODE_NONSECURE, // sfr_bus
		NX_TZPC_SMODE_NONSECURE, // sfr0_bus
		NX_TZPC_SMODE_NONSECURE, // sfr1_bus
		NX_TZPC_SMODE_NONSECURE  // sfr2_bus
	},
	{
		NX_TZPC_SMODE_NONSECURE, // static_bus
		NX_TZPC_SMODE_NONSECURE, // vip2 VIP002 XPROT
		NX_TZPC_SMODE_NONSECURE, // vip2clk
		NX_TZPC_SMODE_SECURE,    // tmu
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE     // NC
	},
	{
		NX_TZPC_SMODE_NONSECURE, // uart01
		NX_TZPC_SMODE_NONSECURE, // uart00
		NX_TZPC_SMODE_NONSECURE, // uart02
		NX_TZPC_SMODE_NONSECURE, // uart03
		NX_TZPC_SMODE_NONSECURE, // i2c0
		NX_TZPC_SMODE_NONSECURE, // i2c1
		NX_TZPC_SMODE_NONSECURE, // i2c2
		NX_TZPC_SMODE_NONSECURE  // ssp2clk
	},
	{
		NX_TZPC_SMODE_NONSECURE, // uart01clk
		NX_TZPC_SMODE_NONSECURE, // uart00clk
		NX_TZPC_SMODE_NONSECURE, // uart02clk
		NX_TZPC_SMODE_NONSECURE, // uart03clk
		NX_TZPC_SMODE_NONSECURE, // ssp0clk
		NX_TZPC_SMODE_NONSECURE, // ssp1clk
		NX_TZPC_SMODE_NONSECURE, // i2c0clk
		NX_TZPC_SMODE_NONSECURE  // i2c1clk
	},
	/* TZPC5 */
	{
		NX_TZPC_SMODE_NONSECURE, // i2c2clk
		NX_TZPC_SMODE_NONSECURE, // uart05clk
		NX_TZPC_SMODE_NONSECURE, // i2s0clk
		NX_TZPC_SMODE_NONSECURE, // i2s1clk
		NX_TZPC_SMODE_NONSECURE, // i2s2clk
		NX_TZPC_SMODE_NONSECURE, // deinterclk
		NX_TZPC_SMODE_NONSECURE, // scalerclk
		NX_TZPC_SMODE_NONSECURE  // mpegtsiclk
	},
	{
		NX_TZPC_SMODE_NONSECURE, // spdiftxclk
		NX_TZPC_SMODE_NONSECURE, // pwmt0clk
		NX_TZPC_SMODE_NONSECURE, // pwmt1clk
		NX_TZPC_SMODE_NONSECURE, // timer01
		NX_TZPC_SMODE_NONSECURE, // timer02
		NX_TZPC_SMODE_NONSECURE, // timer03
		NX_TZPC_SMODE_NONSECURE, // pwm01
		NX_TZPC_SMODE_NONSECURE  // pwm02
	},
	{
		NX_TZPC_SMODE_NONSECURE, // pwm03
		NX_TZPC_SMODE_NONSECURE, // vip1clk
		NX_TZPC_SMODE_NONSECURE, // vip0clk
		NX_TZPC_SMODE_NONSECURE, // maliclk
		NX_TZPC_SMODE_NONSECURE, // ppmclk
		NX_TZPC_SMODE_NONSECURE, // sdmmc0clk
		NX_TZPC_SMODE_NONSECURE, // cryptoclk
		NX_TZPC_SMODE_NONSECURE  // codaclk
	},
	{
		NX_TZPC_SMODE_NONSECURE, // gmacclk
		NX_TZPC_SMODE_NONSECURE, // reserved
		NX_TZPC_SMODE_NONSECURE, // mipitop clk
		NX_TZPC_SMODE_NONSECURE, // pdmclk
		NX_TZPC_SMODE_NONSECURE, // sdmmc1clk
		NX_TZPC_SMODE_NONSECURE, // sdmmc2clk
		NX_TZPC_SMODE_NONSECURE, // can0
		NX_TZPC_SMODE_NONSECURE  // can1
	},
	/* TZPC6 */
	{
		NX_TZPC_SMODE_SECURE, // DREX
		NX_TZPC_SMODE_SECURE, // DDRPHY
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	},
	{
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	},
	{
		NX_TZPC_SMODE_NONSECURE, // HDMI_PHY
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE,    // NC
		NX_TZPC_SMODE_SECURE     // NC
	},
	{
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE, // NC
		NX_TZPC_SMODE_SECURE  // NC
	}
};

union nx_tzpc_prot {
	unsigned char port_group;
	struct nx_tzpc_prot_bit pro_bit;
};

static inline void secure_set_tzpc(void)
{
	U32 i, j;
	union nx_tzpc_prot *pProt = (union nx_tzpc_prot *)prob_bit;

	for(i = 0; i < NX_TZPC_MODULE_NUMBER; i++) {
//		printf("i:%d, base:%X, size:%x\r\n", i, &pReg_TZPC[i]->R0SIZE, sizeof(struct NX_TZPC_RegisterSet));
		mmio_write_32(&pReg_TZPC[i]->R0SIZE, tzpc_r0_size[i]);		// 1: 4KB, 2:8KB, ....
		for(j = 0; j < NX_TZPC_PORT; j++) {
			mmio_write_32(&pReg_TZPC[i]->DETPROT[j].CLEAR, ~(pProt[i*NX_TZPC_PORT+j].port_group));		// set all secure mode to default
			mmio_write_32(&pReg_TZPC[i]->DETPROT[j].SET, pProt[i*NX_TZPC_PORT+j].port_group);	// 0: secure, 1: non-secure
//			mmio_write_32(&pReg_TZPC[i]->DETPROT[j].SET, 0xFFFFFFFF);	// 0: secure, 1: non-secure
		}
	}
//	mmio_write_32(&pReg_Tieoff->TIEOFFREG[26], 0xFFFFFFFF);		// tzpc decprot mode set. 0:secure, 1:non-secure
}

static inline void secure_set_tzasc(void)
{
	U32 i, j;
	struct NX_TZC380_RegisterSet *const pTZC380 =
	    (struct NX_TZC380_RegisterSet *)PHY_BASEADDR_DREX_TZ_MODULE;

//	printf("address width %d, num of region:%d\r\n", ((pTZC380->CONFIGURATION >>8)&0x3F)+1, (pTZC380->CONFIGURATION & 0xF));
	mmio_write_32( &pTZC380->ACTION, NX_TZASC_REACTION_INTLOW_DECERR);			// int low and DECERR response so occur exception

	// region 0 is always cover all area to secure.
	j = (mmio_read_32(&pTZC380->CONFIGURATION) & 0xF) + 1;
	for(i = 1; i < j; i++) {
		mmio_write_32(&pTZC380->RS[i].REGION_SETUP_HIGH, 0);
		mmio_write_32(&pTZC380->RS[i].REGION_SETUP_LOW,  0);//x40000000;
		mmio_write_32(&pTZC380->RS[i].REGION_ATTRIBUTES,
			((0x0F << 28) |			// secure r/w, non-secure r/w
			(0xFF <<  8) |			// sub region x is enabled
			(0x20 <<  1) |			// 2GB region
			(0x00 <<  0)));			// enable for region 0: disable, 1:enable

	}

	/* Set TZASC region 1 as secure DRAM (from 0x7E000000, size 32MB) */
	mmio_write_32(&pTZC380->RS[1].REGION_SETUP_HIGH, 0);
	mmio_write_32(&pTZC380->RS[1].REGION_SETUP_LOW, 0x4E000000); // x40000000;
	mmio_write_32(&pTZC380->RS[1].REGION_ATTRIBUTES,
		  ((0x0C << 28) |			// Only secure r/w
		   (0x200 << 4) |			// 32MB region (0x200 * 64KB)
		   (0x01 <<  0)));			// enable for region

	mmio_write_32(&pTZC380->SPECULATION_CONTROL, 0); // 0: speculation is enabled. defalut
}

static inline void secure_set_gic_master(void)
{
	U32 i, nPPI;

	mmio_write_32(&pReg_GIC400->GICD.CTLR,
			(0x0 << 1) |   			// enable group1	routed to non-secure IRQ by GICC.CTLR.FIQEn
			(0x0 << 0));			// enable group0	routed to secure FIQ

//	printf("GICD.CTLR:%X\r\n", pReg_GIC400->GICD.CTLR);
	nPPI = mmio_read_32(&pReg_GIC400->GICD.TYPER) & 0x1F;
//	printf("nPPI is %d\r\n", nPPI);
	for (i = 1; i <= nPPI; i++) {
		mmio_write_32(&pReg_GIC400->GICD.ICENABLER[i], 0xFFFFFFFF);
		mmio_write_32(&pReg_GIC400->GICD.ICPENDR[i], 0xFFFFFFFF);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 0], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 1], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 2], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 3], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 4], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 5], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 6], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[i * 8 + 7], 0x80808080);
		mmio_write_32(&pReg_GIC400->GICD.IGROUPR[i], 0xFFFFFFFF);				// set int group to non-secur 0: to secure FIQ, 1: to non-secure IRQ
//		printf("GICD.IGROUPR[%d]:%X\r\n", i, mmio_read_32(&pReg_GIC400->GICD.IGROUPR[i]));
	}
//	mmio_write_8 (&pReg_GIC400->GICD.ITARGETSR[0], 0xFF);		// sgi target to all cpu
//	mmio_write_8 (&pReg_GIC400->GICD.IPRIORITYR[0], 0xFF);		// sgi target to all cpu
//	mmio_write_32(&pReg_GIC400->GICC.NSAPR0, 0xFFFFFFFF);		// non-secure read/write access permitted
//	mmio_write_32(&pReg_GIC400->GICD.NSACR[0], 0xFFFFFFFF);		// non-secure read/write access permitted
//	mmio_write_32(&pReg_GIC400->GICD.ICFGR[0], 0x0);		// 0: level, 1: edge

}

void secure_set_gic_all(void)
{
//	mmio_write_32(&pReg_GIC400->GICC.PMR, 0x1<<7);	// all high priority
	mmio_write_32(&pReg_GIC400->GICC.PMR, 0xFF);		// all high priority
	mmio_write_32(&pReg_GIC400->GICC.CTLR,
			    	(0 << 10) |	// EOImodeNS
				(0 <<  9) |	// EOImodeS
				(1 <<  8) |	// IRQBypDisGrp1
				(1 <<  7) |	// FIQBypDisGrp1
				(1 <<  6) |	// IRQBypDisGrp0
				(0 <<  5) |	// FIQBypDisGrp0
				(0 <<  4) |	// CBPR 	0: to determine any preemption, use GICC_BPR for Group0, GICC_ABPR for Group1, 1: GICC_BPR for both Group 0 and 1
				(1 <<  3) |	// FIQEn	0: Group0 int to IRQ,	1: Group int to FIQ,
				(0 <<  2) |	// AckCtl	0: If the highest priority pending int is a Group 1, read GICC_IAR or GICC_HPPIR return 1022 1:	" return IAR
				(1 <<  1) |	// Group 1 int Enable
				(1 <<  0));	// Group 0 int Enable
						// enable cpu interface
//	mmio_write_32(&pReg_GIC400->GICC.CTLR, 0x1);
//	mmio_write_8 (&pReg_GIC400->GICD.ITARGETSR[0], 1<<CPUID); 		// sgi target to all cpu
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[0], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[1], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[2], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[3], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[4], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[5], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[6], 0x0);
	mmio_write_32(&pReg_GIC400->GICD.IPRIORITYR[7], 0x0);

	mmio_write_32(&pReg_GIC400->GICD.ICENABLER[0], 0xFFFFFFFF);
	mmio_write_32(&pReg_GIC400->GICD.ICPENDR[0],   0xFFFFFFFF);
	mmio_write_32(&pReg_GIC400->GICD.IGROUPR[0],   0xFFFFFFFF);		// banked, set int group to 1	// 0: to secure FIQ, 1: to non-secure IRQ
//	mmio_write_32(&pReg_GIC400->GICD.ISENABLER[0], 0xFF);		// enable SGI 0
//	mmio_write_32(&pReg_GIC400->GICD.ICPENDR[0], 0xFF);			// clear pending
}

void secure_set_state(void)
{
	secure_set_tzpc();
	secure_set_tzasc();
	secure_set_gic_all();
	secure_set_gic_master();
}
