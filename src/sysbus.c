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
#include <nx_prototype.h>
#include "nx_type.h"
#include "printf.h"
#include "debug.h"
#include <nx_debug2.h>
#include "nx_peridot.h"
#include <nx_tieoff.h>

#include <nx_drex.h>
#include "secondboot.h"
#include "sysbus.h"

#define CFG_TIEOFF_QOS_ID_RESET (1)
#define CFG_DREX_BRB_RESET (1)
#define CFG_DREX_QOS_RESET (1)
#define CFG_DREX_QOS_MSG_ON (0)

#define CFG_BOTT_BUS_RESET (0)
#define CFG_BOTT_BUS_MSG_ON (0)

#define CFG_TOP_BUS_RESET (0)
#define CFG_TOP_BUS_MSG_ON (0)

#define CFG_PERI_BUS_RESET (0)
#define CFG_PERI_BUS_MI_RESET (0) // off - fixed
#define CFG_PERI_BUS_MSG_ON (0)

//------------------------------------------------------------------------------
// DREX BRB
//
#define NX_DREX_PORT_3_W_BRB_EN_VAL (0)
#define NX_DREX_PORT_2_W_BRB_EN_VAL (1) // Bottom bus
#define NX_DREX_PORT_1_W_BRB_EN_VAL (1) // CCI400 bus
#define NX_DREX_PORT_0_W_BRB_EN_VAL (1) // Disp bus
#define NX_DREX_PORT_3_R_BRB_EN_VAL (0)
#define NX_DREX_PORT_2_R_BRB_EN_VAL (1) // Bottom bus
#define NX_DREX_PORT_1_R_BRB_EN_VAL (1) // CCI400 bus
#define NX_DREX_PORT_0_R_BRB_EN_VAL (1) // Disp bus

#define NX_DREX_PORT_3_W_BRB_TH_VAL (0xF)
#define NX_DREX_PORT_2_W_BRB_TH_VAL (0x3) // Bottom bus
#define NX_DREX_PORT_1_W_BRB_TH_VAL (0x8) // CCI400 bus
#define NX_DREX_PORT_0_W_BRB_TH_VAL (0xF) // Disp bus
#define NX_DREX_PORT_3_R_BRB_TH_VAL (0xF)
#define NX_DREX_PORT_2_R_BRB_TH_VAL (0x3) // Bottom bus
#define NX_DREX_PORT_1_R_BRB_TH_VAL (0x8) // CCI400 bus
#define NX_DREX_PORT_0_R_BRB_TH_VAL (0xF) // Disp bus

//------------------------------------------------------------------------------
//  TIEOFF
//

// TIEOFF 77
#define NX_QOS_ID_USB_HW (5)

// TIEOFF 76
#define NX_QOS_ID_USB_HA (5)

// TIEOFF 130
#define NX_QOS_ID_A53_1_AR (5)
#define NX_QOS_ID_A53_1_AW (5) // A53
#define NX_QOS_ID_A53_0_AR (5)
#define NX_QOS_ID_A53_0_AW (5) // A53
#define NX_QOS_ID_CODA_AR (14)
#define NX_QOS_ID_CODA_W (14)
#define NX_QOS_ID_CODA_AW (14) // CODA

// TIEOFF 74
#define NX_QOS_ID_GMAC_AR (5)
#define NX_QOS_ID_GMAC_W (5)
#define NX_QOS_ID_GMAC_AW (5) // GMAC
#define NX_QOS_ID_SCALER_AR (5)
#define NX_QOS_ID_SCALER_W (5)
#define NX_QOS_ID_SCALER_AW (5) // Scaler
#define NX_QOS_ID_DEINTER_AR (5)

// TIEOFF 73
#define NX_QOS_ID_DEINTER_W (5)
#define NX_QOS_ID_DEINTER_AW (5) // Deinterlace
#define NX_QOS_ID_3D_AR (14)
#define NX_QOS_ID_3D_W (14)
#define NX_QOS_ID_3D_AW (14)   // VR(3D)
#define NX_QOS_ID_MLC1_AR (15) // MLC1

// TIEOFF 72
#define NX_QOS_ID_MLC0_AR (15) // MLC0

// TIEOFF 71
#define NX_QOS_ID_CORESIGHT_AR (5)
#define NX_QOS_ID_CORESIGHT_W (5)
#define NX_QOS_ID_CORESIGHT_AW (5) // CoreSight
#define NX_QOS_ID_VIP2_AR (10)
#define NX_QOS_ID_VIP2_W (10)

// TIEOFF 70
#define NX_QOS_ID_VIP2_AW (10) // VIP2
#define NX_QOS_ID_VIP1_AR (10)
#define NX_QOS_ID_VIP1_W (10)
#define NX_QOS_ID_VIP1_AW (10) // VIP1
#define NX_QOS_ID_VIP0_AR (10)
#define NX_QOS_ID_VIP0_W (10)
#define NX_QOS_ID_VIP0_AW (10) // VIP0

// TIEOFF 69
#define NX_QOS_ID_DMAC_HW (5)
#define NX_QOS_ID_DMAC_HA (5)
#define NX_QOS_ID_SDMMC_HW (5)
#define NX_QOS_ID_SDMMC_HA (5)
#define NX_QOS_ID_DMAC_MP2TS_HW (5)
#define NX_QOS_ID_DMAC_MP2TS_HA (5)

/* Allowd time */
#if (CFG_DREX_QOS_RESET == 1)
static const u16 g_DrexQoS[NX_DREX_QOS_NUMBER][2] = {
    //  {Count, Threshold}
    {0xFFF, 0x000}, // QoS ID : 0 - Lowest
    {0xE00, 0x000}, // QoS ID : 1
    {0xD00, 0x000}, // QoS ID : 2
    {0xC00, 0x000}, // QoS ID : 3
    {0xB00, 0x000}, // QoS ID : 4
    {0xA00, 0x000}, // QoS ID : 5
    {0x900, 0x000}, // QoS ID : 6
    {0x800, 0x000}, // QoS ID : 7
    {0x700, 0x000}, // QoS ID : 8
    {0x600, 0x000}, // QoS ID : 9
    {0x500, 0x000}, // QoS ID : 10
    {0x400, 0x000}, // QoS ID : 11
    {0x300, 0x000}, // QoS ID : 12
    {0x200, 0x000}, // QoS ID : 13
    {0x100, 0x000}, // QoS ID : 14
    {0x000, 0x000}  // QoS ID : 15 - Hightest (No delay)
};
#endif

#if (CFG_PERI_BUS_RESET == 1)
static const u8 g_PeriBusSI[4] = {
    NX_BUS_IOPERI_SI_SLOT_MP2TS, NX_BUS_IOPERI_SI_SLOT_USB,
    NX_BUS_IOPERI_SI_SLOT_SDMMC, NX_BUS_IOPERI_SI_SLOT_GMAC};
#endif

#if (CFG_TOP_BUS_RESET == 1)
static const u8 g_TopBusSI[4] = {
    NX_BUS_TOP_SI_SLOT_VIP_0, NX_BUS_TOP_SI_SLOT_VIP_1,
    NX_BUS_TOP_SI_SLOT_VIP_2, NX_BUS_TOP_SI_SLOT_IOPERI};
#endif

#if (CFG_BOTT_BUS_RESET == 1)
static const u8 g_BottomBusSI[4] = {
    NX_BUS_BOTTOM_SI_SLOT_MALI, NX_BUS_BOTTOM_SI_SLOT_CODA,
    NX_BUS_BOTTOM_SI_SLOT_SCALER,
    NX_BUS_BOTTOM_SI_SLOT_MALI // NX_BUS_BOTTOM_SI_SLOT_DEINTERLACE
};
#endif

#if 0
const u8 g_PeriQoSSI[2] = {
    1,  // Tidemark
    (1<<PERIBUS_SI_SLOT_TOP)    // Control
};
#endif

void set_bus_config(void)
{
#if (CFG_BOTT_BUS_RESET | CFG_BOTT_BUS_MSG_ON | CFG_TOP_BUS_RESET |            \
     CFG_TOP_BUS_MSG_ON | CFG_PERI_BUS_RESET | CFG_PERI_BUS_MI_RESET |         \
     CFG_PERI_BUS_MSG_ON)
	u32 val, temp;
	u32 num_si, num_mi;
	u32 addr, i_slot;
#if 0
    u32 retry_count = 0;

retry_bus_reconfig:
#endif

#if (CFG_BOTT_BUS_MSG_ON == 1)
	printf("\r\n");
	printf("------------- Bottom BUS ------------\r\n");
	printf("\r\n");
	printf("PrimeCell Config Reg0(0xFC0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC0));
	printf("PrimeCell Config Reg1(0xFC4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC4));

	printf("PrimeCell Periph Reg0(0xFE0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFE0));
	printf("PrimeCell Periph Reg1(0xFE4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFE4));
	printf("PrimeCell Periph Reg2(0xFE8) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFE8));
	printf("PrimeCell Periph Reg3(0xFEC) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFEC));

	printf("PrimeCell ID     Reg0(0xFF0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFF0));
	printf("PrimeCell ID     Reg1(0xFF4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFF4));
	printf("PrimeCell ID     Reg2(0xFF8) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFF8));
	printf("PrimeCell ID     Reg3(0xFFC) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_BOTT + 0xFFC));
#endif /* CFG_BOTT_BUS_MSG_ON */

	num_si = readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC4);

/* Set progamming for AR */
#if (CFG_BOTT_BUS_RESET == 1)
	addr = NX_BASE_ADDR_BOTT_AR;
	if (num_mi > 1) {
		// Master Interface
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			val = readl(addr);
			if (val != i_slot)
				writel((i_slot << SLOT_NUM_POS) |
					   (i_slot << SI_IF_NUM_POS),
				       addr);
		}

		addr = (NX_BASE_ADDR_BOTT_AR + 0x20);
	}

	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		val = readl(addr);
		if (val != g_BottomBusSI[i_slot])
			writel((i_slot << SLOT_NUM_POS) |
				   (g_BottomBusSI[i_slot] << SI_IF_NUM_POS),
			       addr);
	}

	/* Set progamming for AW */
	addr = NX_BASE_ADDR_BOTT_AW;
	if (num_mi > 1) {
		// Master Interface
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			val = readl(addr);
			if (val != i_slot)
				writel((i_slot << SLOT_NUM_POS) |
					   (i_slot << SI_IF_NUM_POS),
				       addr);
		}

		addr = (NX_BASE_ADDR_BOTT_AW + 0x20);
	}

	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		val = readl(addr);
		if (val != g_BottomBusSI[i_slot])
			writel((i_slot << SLOT_NUM_POS) |
				   (g_BottomBusSI[i_slot] << SI_IF_NUM_POS),
			       addr);
	}
#endif // #if (CFG_BOTT_BUS_RESET == 1)

#if (CFG_BOTT_BUS_MSG_ON == 1)
	printf("MI QOS Tidemark : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_BOTT_QOS_TRDMARK));
	printf("MI QOS Control  : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_BOTT_QOS_CTRL));

	printf("SI QOS Tidemark : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_BOTT_QOS_TRDMARK + 0x20));
	printf("SI QOS Control  : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_BOTT_QOS_CTRL + 0x20));

	addr = NX_BASE_ADDR_BOTT_AR;
	if (num_mi > 1) {
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			printf("MI %02d - AR      : 0x%08x\r\n", i_slot,
			       readl(addr));
		}

		addr = (NX_BASE_ADDR_BOTT_AR + 0x20);
	}
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		printf("SI %02d - AR      : 0x%08x\r\n", i_slot, readl(addr));
	}

	addr = NX_BASE_ADDR_BOTT_AW;
	if (num_mi > 1) {
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			printf("MI %02d - AW      : 0x%08x\r\n", i_slot,
			       readl(addr));
		}

		addr = (NX_BASE_ADDR_BOTT_AW + 0x20);
	}
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		printf("SI %02d - AW      : 0x%08x\r\n", i_slot, readl(addr));
	}
#endif /* CFG_BOTT_BUS_MSG_ON */

#if (CFG_TOP_BUS_MSG_ON == 1)
	printf("\r\n");
	printf("------------- Top BUS ------------\r\n");
	printf("\r\n");
	printf("PrimeCell Config Reg0(0xFC0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFC0));
	printf("PrimeCell Config Reg1(0xFC4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFC4));

	printf("PrimeCell Periph Reg0(0xFE0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFE0));
	printf("PrimeCell Periph Reg1(0xFE4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFE4));
	printf("PrimeCell Periph Reg2(0xFE8) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFE8));
	printf("PrimeCell Periph Reg3(0xFEC) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFEC));

	printf("PrimeCell ID     Reg0(0xFF0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFF0));
	printf("PrimeCell ID     Reg1(0xFF4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFF4));
	printf("PrimeCell ID     Reg2(0xFF8) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFF8));
	printf("PrimeCell ID     Reg3(0xFFC) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_TOP + 0xFFC));
#endif /* CFG_TOP_BUS_MSG_ON */

	num_si = readl(NX_VA_BASE_REG_PL301_TOP + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_TOP + 0xFC4);

#if (CFG_TOP_BUS_RESET == 1)
	/* Set progamming for AR */
	addr = NX_BASE_ADDR_TOP_AR;
	if (num_mi > 1) {
		// Master Interface
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			val = readl(addr);
			if (val != i_slot)
				writel((i_slot << SLOT_NUM_POS) |
					   (i_slot << SI_IF_NUM_POS),
				       addr);
		}

		addr = (NX_BASE_ADDR_TOP_AR + 0x20);
	}

	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		val = readl(addr);
		if (val != g_TopBusSI[i_slot])
			writel((i_slot << SLOT_NUM_POS) |
				   (g_TopBusSI[i_slot] << SI_IF_NUM_POS),
			       addr);
	}

	/* Set progamming for AW */
	addr = NX_BASE_ADDR_TOP_AW;
	if (num_mi > 1) {
		// Master Interface
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			val = readl(addr);
			if (val != i_slot)
				writel((i_slot << SLOT_NUM_POS) |
					   (i_slot << SI_IF_NUM_POS),
				       addr);
		}

		addr = (NX_BASE_ADDR_TOP_AW + 0x20);
	}

	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		val = readl(addr);
		if (val != g_TopBusSI[i_slot])
			writel((i_slot << SLOT_NUM_POS) |
				   (g_TopBusSI[i_slot] << SI_IF_NUM_POS),
			       addr);
	}
#endif // #if (CFG_TOP_BUS_RESET == 1)

#if (CFG_TOP_BUS_MSG_ON == 1)
	printf("MI QOS Tidemark : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_TOP_QOS_TRDMARK));
	printf("MI QOS Control  : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_TOP_QOS_CTRL));

	printf("SI QOS Tidemark : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_TOP_QOS_TRDMARK + 0x20));
	printf("SI QOS Control  : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_TOP_QOS_CTRL + 0x20));

	addr = NX_BASE_ADDR_TOP_AR;
	if (num_mi > 1) {
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			printf("MI %02d - AR      : 0x%08x\r\n", i_slot,
			       readl(addr));
		}

		addr = (NX_BASE_ADDR_TOP_AR + 0x20);
	}
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		printf("SI %02d - AR      : 0x%08x\r\n", i_slot, readl(addr));
	}

	addr = NX_BASE_ADDR_TOP_AW;
	if (num_mi > 1) {
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			printf("MI %02d - AW      : 0x%08x\r\n", i_slot,
			       readl(addr));
		}

		addr = (NX_BASE_ADDR_TOP_AW + 0x20);
	}
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		printf("SI %02d - AW      : 0x%08x\r\n", i_slot, readl(addr));
	}
#endif /* CFG_TOP_BUS_MSG_ON */

#if (CFG_PERI_BUS_MSG_ON == 1)
	printf("\r\n");
	printf("------------- Peri BUS -----------\r\n");
	printf("\r\n");
	printf("PrimeCell Config Reg0(0xFC0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFC0));
	printf("PrimeCell Config Reg1(0xFC4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFC4));

	printf("PrimeCell Periph Reg0(0xFE0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFE0));
	printf("PrimeCell Periph Reg1(0xFE4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFE4));
	printf("PrimeCell Periph Reg2(0xFE8) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFE8));
	printf("PrimeCell Periph Reg3(0xFEC) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFEC));

	printf("PrimeCell ID     Reg0(0xFF0) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFF0));
	printf("PrimeCell ID     Reg1(0xFF4) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFF4));
	printf("PrimeCell ID     Reg2(0xFF8) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFF8));
	printf("PrimeCell ID     Reg3(0xFFC) : 0x%08x\r\n",
	       readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFFC));
#endif /* CFG_PERI_BUS_MSG_ON */

	num_si = readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_IOPERI + 0xFC4);

/* Set SI QoS */
#if 0
    val = readl(NX_BASE_ADDR_IOPERI_QOS_TRDMARK + 0x20) & 0xFF;
    if (val != g_PeriQoSSI[0])
        writel(g_PeriQoSSI[0], (NX_BASE_ADDR_IOPERI_QOS_TRDMARK + 0x20) );

    val = readl(NX_BASE_ADDR_IOPERI_QOS_CTRL + 0x20) & 0xFF;
    if (val != g_PeriQoSSI[1])
        writel(g_PeriQoSSI[1], (NX_BASE_ADDR_IOPERI_QOS_CTRL + 0x20) );
#endif

/* Set progamming for AR */
#if (CFG_PERI_BUS_RESET == 1)
#if (CFG_PERI_BUS_MI_RESET == 1)
	addr = NX_BASE_ADDR_IOPERI_AR;
	if (num_mi > 1) {
		// Master Interface
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			val = readl(addr);
			if (val != i_slot)
				writel((i_slot << SLOT_NUM_POS) |
					   (i_slot << SI_IF_NUM_POS),
				       addr);
		}

		addr = (NX_BASE_ADDR_IOPERI_AR + 0x20);
	}
#else
	addr = (NX_BASE_ADDR_IOPERI_AR + 0x20);
#endif

	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		val = readl(addr);
		if (val != g_PeriBusSI[i_slot])
			writel((i_slot << SLOT_NUM_POS) |
				   (g_PeriBusSI[i_slot] << SI_IF_NUM_POS),
			       addr);
	}

/* Set progamming for AW */
#if (CFG_PERI_BUS_MI_RESET == 1)
	addr = NX_BASE_ADDR_IOPERI_AW;
	if (num_mi > 1) {
		// Master Interface
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			val = readl(addr);
			if (val != i_slot)
				writel((i_slot << SLOT_NUM_POS) |
					   (i_slot << SI_IF_NUM_POS),
				       addr);
		}

		addr = (NX_BASE_ADDR_IOPERI_AW + 0x20);
	}
#else
	addr = (NX_BASE_ADDR_IOPERI_AW + 0x20);
#endif

	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		val = readl(addr);
		if (val != g_PeriBusSI[i_slot])
			writel((i_slot << SLOT_NUM_POS) |
				   (g_PeriBusSI[i_slot] << SI_IF_NUM_POS),
			       addr);
	}
#endif // #if (CFG_PERI_BUS_RESET == 1)

#if (CFG_PERI_BUS_MSG_ON == 1)
	printf("MI QOS Tidemark : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_IOPERI_QOS_TRDMARK));
	printf("MI QOS Control  : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_IOPERI_QOS_CTRL));

	printf("SI QOS Tidemark : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_IOPERI_QOS_TRDMARK));
	printf("SI QOS Control  : 0x%08x\r\n",
	       readl(NX_BASE_ADDR_IOPERI_QOS_CTRL));

	addr = NX_BASE_ADDR_IOPERI_AR;
	if (num_mi > 1) {
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			printf("MI %02d - AR      : 0x%08x\r\n", i_slot,
			       readl(addr));
		}

		addr = (NX_BASE_ADDR_IOPERI_AR + 0x20);
	}
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		printf("SI %02d - AR      : 0x%08x\r\n", i_slot, readl(addr));
	}

	addr = NX_BASE_ADDR_IOPERI_AW;
	if (num_mi > 1) {
		for (i_slot = 0; i_slot < num_mi; i_slot++) {
			writel((0xFF000000 | i_slot), addr);
			printf("MI %02d - AW      : 0x%08x\r\n", i_slot,
			       readl(addr));
		}

		addr = (NX_BASE_ADDR_IOPERI_AW + 0x20);
	}
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), addr);
		printf("SI %02d - AW      : 0x%08x\r\n", i_slot, readl(addr));
	}
#endif /* CFG_PERI_BUS_MSG_ON */

#if 0
    if (retry_count == 0)
    {
        retry_count++;
        goto retry_bus_reconfig;
    }
#endif

#endif // #if (CFG_BOTT_BUS_RESET | CFG_BOTT_BUS_MSG_ON | CFG_TOP_BUS_RESET |
       // CFG_TOP_BUS_MSG_ON | CFG_PERI_BUS_RESET | CFG_PERI_BUS_MI_RESET |
       // CFG_PERI_BUS_MSG_ON)

	return;
}

void set_drex_qos(void)
{
	U32 index, val, temp;

/* ------------- DREX BRB Threshold -------------- */
#if (CFG_DREX_BRB_RESET == 1)
	temp = ((NX_DREX_PORT_3_W_BRB_TH_VAL << NX_DREX_PORT_3_W_BRB_TH_SHFT) |
		(NX_DREX_PORT_2_W_BRB_TH_VAL << NX_DREX_PORT_2_W_BRB_TH_SHFT) |
		(NX_DREX_PORT_1_W_BRB_TH_VAL << NX_DREX_PORT_1_W_BRB_TH_SHFT) |
		(NX_DREX_PORT_0_W_BRB_TH_VAL << NX_DREX_PORT_0_W_BRB_TH_SHFT) |
		(NX_DREX_PORT_3_R_BRB_TH_VAL << NX_DREX_PORT_3_R_BRB_TH_SHFT) |
		(NX_DREX_PORT_2_R_BRB_TH_VAL << NX_DREX_PORT_2_R_BRB_TH_SHFT) |
		(NX_DREX_PORT_1_R_BRB_TH_VAL << NX_DREX_PORT_1_R_BRB_TH_SHFT) |
		(NX_DREX_PORT_0_R_BRB_TH_VAL << NX_DREX_PORT_0_R_BRB_TH_SHFT));
	writel(temp, (NX_VA_BASE_REG_DREX + NX_DREX_BRBRSV_CFG_OFFSET));

	temp = ((NX_DREX_PORT_3_W_BRB_EN_VAL << NX_DREX_PORT_3_W_BRB_EN_SHFT) |
		(NX_DREX_PORT_2_W_BRB_EN_VAL << NX_DREX_PORT_2_W_BRB_EN_SHFT) |
		(NX_DREX_PORT_1_W_BRB_EN_VAL << NX_DREX_PORT_1_W_BRB_EN_SHFT) |
		(NX_DREX_PORT_0_W_BRB_EN_VAL << NX_DREX_PORT_0_W_BRB_EN_SHFT) |
		(NX_DREX_PORT_3_R_BRB_EN_VAL << NX_DREX_PORT_3_R_BRB_EN_SHFT) |
		(NX_DREX_PORT_2_R_BRB_EN_VAL << NX_DREX_PORT_2_R_BRB_EN_SHFT) |
		(NX_DREX_PORT_1_R_BRB_EN_VAL << NX_DREX_PORT_1_R_BRB_EN_SHFT) |
		(NX_DREX_PORT_0_R_BRB_EN_VAL << NX_DREX_PORT_0_R_BRB_EN_SHFT));
	writel(temp, (NX_VA_BASE_REG_DREX + NX_DREX_BRBRSV_CTRL_OFFSET));
#endif

/* ------------- DREX QoS -------------- */
#if (CFG_DREX_QOS_RESET == 1)
	for (index = 0; index < NX_DREX_QOS_NUMBER; index++) {
#ifdef aarch32
		temp = (g_DrexQoS[index][1] << 16) | g_DrexQoS[index][0];
		val = readl((U32)(NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET +
				  (index << 3)));
		if (val != temp)
			writel(temp, (U32)(NX_VA_BASE_REG_DREX +
					   NX_DREX_QOS_OFFSET + (index << 3)));
#endif
#ifdef aarch64
		temp = (g_DrexQoS[index][1] << 16) | g_DrexQoS[index][0];
		val = readl((U64)(NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET +
				  (index << 3)));
		if (val != temp)
			writel(temp, (U64)(NX_VA_BASE_REG_DREX +
					   NX_DREX_QOS_OFFSET + (index << 3)));
#endif
	}
#endif

#if (CFG_DREX_QOS_MSG_ON == 1)
	printf("\r\n");
	printf("-------------- DREX QoS -------------\r\n");
	printf("\r\n");

	for (index = 0; index < NX_DREX_QOS_NUMBER; index++) {
		printf("Drex QoS %02d - : 0x%08x\r\n", index,
		       readl(NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET +
			     (index << 3)));
	}
#endif

#if (CFG_TIEOFF_QOS_ID_RESET == 1)
	// TIEOFF 77
	temp = readl(NX_VA_BASE_REG_TIEOFF + (77 << 2)) &
	       ~(0xF << NX_QOS_SHFT_USB_HW);
	temp |= (NX_QOS_ID_USB_HW << NX_QOS_SHFT_USB_HW);
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (77 << 2)));

	// TIEOFF 76
	temp = readl(NX_VA_BASE_REG_TIEOFF + (76 << 2)) &
	       ~(0xF << NX_QOS_SHFT_USB_HA);
	temp |= (NX_QOS_ID_USB_HA << NX_QOS_SHFT_USB_HA);
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (76 << 2)));

	// TIEOFF 130
	temp = ((NX_QOS_ID_A53_1_AR << NX_QOS_SHFT_A53_1_AR) |
		(NX_QOS_ID_A53_1_AW << NX_QOS_SHFT_A53_1_AW) |
		(NX_QOS_ID_A53_0_AR << NX_QOS_SHFT_A53_0_AR) |
		(NX_QOS_ID_A53_0_AW << NX_QOS_SHFT_A53_0_AW) |
		(NX_QOS_ID_CODA_AR << NX_QOS_SHFT_CODA_AR) |
		(NX_QOS_ID_CODA_W << NX_QOS_SHFT_CODA_W) |
		(NX_QOS_ID_CODA_AW << NX_QOS_SHFT_CODA_AW));
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (130 << 2)));

	// TIEOFF 74
	temp = ((NX_QOS_ID_GMAC_AR << NX_QOS_SHFT_GMAC_AR) |
		(NX_QOS_ID_GMAC_W << NX_QOS_SHFT_GMAC_W) |
		(NX_QOS_ID_GMAC_AW << NX_QOS_SHFT_GMAC_AW) |
		(NX_QOS_ID_SCALER_AR << NX_QOS_SHFT_SCALER_AR) |
		(NX_QOS_ID_SCALER_W << NX_QOS_SHFT_SCALER_W) |
		(NX_QOS_ID_SCALER_AW << NX_QOS_SHFT_SCALER_AW) |
		(NX_QOS_ID_DEINTER_AR << NX_QOS_SHFT_DEINTER_AR));
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (74 << 2)));

	// TIEOFF 73
	temp = readl(NX_VA_BASE_REG_TIEOFF + (73 << 2)) & 0xF00;
	temp |= ((NX_QOS_ID_DEINTER_W << NX_QOS_SHFT_DEINTER_W) |
		 (NX_QOS_ID_DEINTER_AW << NX_QOS_SHFT_DEINTER_AW) |
		 (NX_QOS_ID_3D_AR << NX_QOS_SHFT_3D_AR) |
		 (NX_QOS_ID_3D_W << NX_QOS_SHFT_3D_W) |
		 (NX_QOS_ID_3D_AW << NX_QOS_SHFT_3D_AW) |
		 (NX_QOS_ID_MLC1_AR << NX_QOS_SHFT_MLC1_AR));
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (73 << 2)));

	// TIEOFF 72
	temp = (NX_QOS_ID_MLC0_AR << NX_QOS_SHFT_MLC0_AR);
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (72 << 2)));

	// TIEOFF 71
	temp = ((NX_QOS_ID_CORESIGHT_AR << NX_QOS_SHFT_CORESIGHT_AR) |
		(NX_QOS_ID_CORESIGHT_W << NX_QOS_SHFT_CORESIGHT_W) |
		(NX_QOS_ID_CORESIGHT_AW << NX_QOS_SHFT_CORESIGHT_AW) |
		(NX_QOS_ID_VIP2_AR << NX_QOS_SHFT_VIP2_AR) |
		(NX_QOS_ID_VIP2_W << NX_QOS_SHFT_VIP2_W));
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (71 << 2)));

	// TIEOFF 70
	temp = ((NX_QOS_ID_VIP2_AW << NX_QOS_SHFT_VIP2_AW) |
		(NX_QOS_ID_VIP1_AR << NX_QOS_SHFT_VIP1_AR) |
		(NX_QOS_ID_VIP1_W << NX_QOS_SHFT_VIP1_W) |
		(NX_QOS_ID_VIP1_AW << NX_QOS_SHFT_VIP1_AW) |
		(NX_QOS_ID_VIP0_AR << NX_QOS_SHFT_VIP0_AR) |
		(NX_QOS_ID_VIP0_W << NX_QOS_SHFT_VIP0_W) |
		(NX_QOS_ID_VIP0_AW << NX_QOS_SHFT_VIP0_AW));
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (70 << 2)));

	// TIEOFF 69
	temp = readl(NX_VA_BASE_REG_TIEOFF + (69 << 2)) & 0x1F;
	temp |= ((NX_QOS_ID_DMAC_HW << NX_QOS_SHFT_DMAC_HW) |
		 (NX_QOS_ID_DMAC_HA << NX_QOS_SHFT_DMAC_HA) |
		 (NX_QOS_ID_SDMMC_HW << NX_QOS_SHFT_SDMMC_HW) |
		 (NX_QOS_ID_SDMMC_HA << NX_QOS_SHFT_SDMMC_HA) |
		 (NX_QOS_ID_DMAC_MP2TS_HW << NX_QOS_SHFT_DMAC_MP2TS_HW) |
		 (NX_QOS_ID_DMAC_MP2TS_HA << NX_QOS_SHFT_DMAC_MP2TS_HA));
	writel(temp, (NX_VA_BASE_REG_TIEOFF + (69 << 2)));
#endif
}
