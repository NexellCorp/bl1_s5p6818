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
#ifndef __SYS_BUS_H__
#define __SYS_BUS_H__

#include "nx_type.h"
#include "printf.h"
#include "debug.h"
#include <nx_debug2.h>
#include "nx_peridot.h"
#include <nx_tieoff.h>

#include <nx_drex.h>
#include "secondboot.h"

//------------------------------------------------------------------------------
// MISC
//
#define IO_ADDRESS(x) (x)
#if 0
#define readb(addr)                                                            \
	({                                                                     \
		U8 _v = ReadIO8(addr);                                         \
		_v;                                                            \
	})
#define readw(addr)                                                            \
	({                                                                     \
		U16 _v = ReadIO16(addr);                                       \
		_v;                                                            \
	})
#define readl(addr)                                                            \
	({                                                                     \
		U32 _v = ReadIO32(addr);                                       \
		_v;                                                            \
	})
#else
#define readb(addr) ReadIO8(addr)
#define readw(addr) ReadIO16(addr)
#define readl(addr) ReadIO32(addr)
#endif
#if 0
#define writeb(data, addr)                                                     \
	({                                                                     \
		U8 *_v = (U8 *)addr;                                           \
		WriteIO8(_v, data);                                            \
	})
#define writew(data, addr)                                                     \
	({                                                                     \
		U16 *_v = (U16 *)addr;                                         \
		WriteIO16(_v, data);                                           \
	})
#define writel(data, addr)                                                     \
	({                                                                     \
		U32 *_v = (U32 *)addr;                                         \
		WriteIO32(_v, data);                                           \
	})
#else
#define writeb(data, addr) WriteIO8(addr, data)
#define writew(data, addr) WriteIO16(addr, data)
#define writel(data, addr) WriteIO32(addr, data)
#endif
#define u8 U8
#define u16 U16
#define u32 U32
#define u64 U64

//------------------------------------------------------------------------------
// DREX BRB
//
#define NX_DREX_PORT_3_W_BRB_EN_SHFT (7)
#define NX_DREX_PORT_2_W_BRB_EN_SHFT (6)
#define NX_DREX_PORT_1_W_BRB_EN_SHFT (5)
#define NX_DREX_PORT_0_W_BRB_EN_SHFT (4)
#define NX_DREX_PORT_3_R_BRB_EN_SHFT (3)
#define NX_DREX_PORT_2_R_BRB_EN_SHFT (2)
#define NX_DREX_PORT_1_R_BRB_EN_SHFT (1)
#define NX_DREX_PORT_0_R_BRB_EN_SHFT (0)

#define NX_DREX_PORT_3_W_BRB_TH_SHFT (28)
#define NX_DREX_PORT_2_W_BRB_TH_SHFT (24)
#define NX_DREX_PORT_1_W_BRB_TH_SHFT (20)
#define NX_DREX_PORT_0_W_BRB_TH_SHFT (16)
#define NX_DREX_PORT_3_R_BRB_TH_SHFT (12)
#define NX_DREX_PORT_2_R_BRB_TH_SHFT (8)
#define NX_DREX_PORT_1_R_BRB_TH_SHFT (4)
#define NX_DREX_PORT_0_R_BRB_TH_SHFT (0)

//------------------------------------------------------------------------------
//  TIEOFF
//

// TIEOFF 77
#define NX_QOS_SHFT_USB_HW (0)

// TIEOFF 76
#define NX_QOS_SHFT_USB_HA (26)

// TIEOFF 130
#define NX_QOS_SHFT_A53_1_AR (24)
#define NX_QOS_SHFT_A53_1_AW (20) // A53
#define NX_QOS_SHFT_A53_0_AR (16)
#define NX_QOS_SHFT_A53_0_AW (12) // A53
#define NX_QOS_SHFT_CODA_AR (8)
#define NX_QOS_SHFT_CODA_W (4)
#define NX_QOS_SHFT_CODA_AW (0) // CODA

// TIEOFF 74
#define NX_QOS_SHFT_GMAC_AR (24)
#define NX_QOS_SHFT_GMAC_W (20)
#define NX_QOS_SHFT_GMAC_AW (16) // GMAC
#define NX_QOS_SHFT_SCALER_AR (12)
#define NX_QOS_SHFT_SCALER_W (8)
#define NX_QOS_SHFT_SCALER_AW (4) // Scaler
#define NX_QOS_SHFT_DEINTER_AR (0)

// TIEOFF 73
#define NX_QOS_SHFT_DEINTER_W (28)
#define NX_QOS_SHFT_DEINTER_AW (24) // Deinterlace
#define NX_QOS_SHFT_3D_AR (20)
#define NX_QOS_SHFT_3D_W (16)
#define NX_QOS_SHFT_3D_AW (12)  // VR
#define NX_QOS_SHFT_MLC1_AR (8) // MLC1

// TIEOFF 72
#define NX_QOS_SHFT_MLC0_AR (27) // MLC0

// TIEOFF 71
#define NX_QOS_SHFT_CORESIGHT_AR (16)
#define NX_QOS_SHFT_CORESIGHT_W (12)
#define NX_QOS_SHFT_CORESIGHT_AW (8) // CoreSight
#define NX_QOS_SHFT_VIP2_AR (4)
#define NX_QOS_SHFT_VIP2_W (0)

// TIEOFF 70
#define NX_QOS_SHFT_VIP2_AW (24) // VIP2
#define NX_QOS_SHFT_VIP1_AR (20)
#define NX_QOS_SHFT_VIP1_W (16)
#define NX_QOS_SHFT_VIP1_AW (12) // VIP1
#define NX_QOS_SHFT_VIP0_AR (8)
#define NX_QOS_SHFT_VIP0_W (4)
#define NX_QOS_SHFT_VIP0_AW (0) // VIP0

// TIEOFF 69
#define NX_QOS_SHFT_DMAC_HW (25)
#define NX_QOS_SHFT_DMAC_HA (21)
#define NX_QOS_SHFT_SDMMC_HW (17)
#define NX_QOS_SHFT_SDMMC_HA (13)
#define NX_QOS_SHFT_DMAC_MP2TS_HW (9)
#define NX_QOS_SHFT_DMAC_MP2TS_HA (5)

//------------------------------------------------------------------------------
//  PL301
//
#define PERIBUS_SI_SLOT_TOP (3)

#define NX_DREX_QOS_NUMBER (16)

#define NX_DREX_QOS_OFFSET (0x60)
#define NX_DREX_BRBRSV_CTRL_OFFSET (0x100)
#define NX_DREX_BRBRSV_CFG_OFFSET (0x104)

#define NX_PA_BASE_REG_TIEOFF (0xC0011000)
#define NX_PA_BASE_REG_DREX (0xC00E0000)
#define NX_PA_BASE_REG_DDRPHY (0xC00E1000)

#define NX_VA_BASE_REG_TIEOFF IO_ADDRESS(NX_PA_BASE_REG_TIEOFF)
#define NX_VA_BASE_REG_DREX IO_ADDRESS(NX_PA_BASE_REG_DREX)
#define NX_VA_BASE_REG_DDRPHY IO_ADDRESS(NX_PA_BASE_REG_DDRPHY)

#define NX_PL301_QOS_TRDMARK_OFFSET (0x400)
#define NX_PL301_QOS_CTRL_OFFSET (0x404)
#define NX_PL301_AR_OFFSET (0x408)
#define NX_PL301_AW_OFFSET (0x40C)

#define NX_PA_BASE_REG_PL301_IOPERI (0xC0090000)
#define NX_PA_BASE_REG_PL301_BOTT (0xC0091000)
#define NX_PA_BASE_REG_PL301_TOP (0xC0092000)
#define NX_PA_BASE_REG_PL301_DISP (0xC0093000)
#define NX_PA_BASE_REG_PL301_SFR (0xC0094000)
#define NX_PA_BASE_REG_PL301_STATIC (0xC0095000)

#define NX_VA_BASE_REG_PL301_IOPERI IO_ADDRESS(NX_PA_BASE_REG_PL301_IOPERI)
#define NX_VA_BASE_REG_PL301_BOTT IO_ADDRESS(NX_PA_BASE_REG_PL301_BOTT)
#define NX_VA_BASE_REG_PL301_TOP IO_ADDRESS(NX_PA_BASE_REG_PL301_TOP)
#define NX_VA_BASE_REG_PL301_DISP IO_ADDRESS(NX_PA_BASE_REG_PL301_DISP)
#define NX_VA_BASE_REG_PL301_SFR IO_ADDRESS(NX_PA_BASE_REG_PL301_SFR)
#define NX_VA_BASE_REG_PL301_STATIC IO_ADDRESS(NX_PA_BASE_REG_PL301_STATIC)

#define NX_BASE_ADDR_IOPERI_QOS_TRDMARK                                        \
	(NX_VA_BASE_REG_PL301_IOPERI + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_IOPERI_QOS_CTRL                                           \
	(NX_VA_BASE_REG_PL301_IOPERI + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_IOPERI_AR                                                 \
	(NX_VA_BASE_REG_PL301_IOPERI + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_IOPERI_AW                                                 \
	(NX_VA_BASE_REG_PL301_IOPERI + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_BOTT_QOS_TRDMARK                                          \
	(NX_VA_BASE_REG_PL301_BOTT + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_BOTT_QOS_CTRL                                             \
	(NX_VA_BASE_REG_PL301_BOTT + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_BOTT_AR (NX_VA_BASE_REG_PL301_BOTT + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_BOTT_AW (NX_VA_BASE_REG_PL301_BOTT + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_TOP_QOS_TRDMARK                                           \
	(NX_VA_BASE_REG_PL301_TOP + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_TOP_QOS_CTRL                                              \
	(NX_VA_BASE_REG_PL301_TOP + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_TOP_AR (NX_VA_BASE_REG_PL301_TOP + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_TOP_AW (NX_VA_BASE_REG_PL301_TOP + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_DISP_QOS_TRDMARK                                          \
	(NX_VA_BASE_REG_PL301_DISP + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_DISP_QOS_CTRL                                             \
	(NX_VA_BASE_REG_PL301_DISP + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_DISP_AR (NX_VA_BASE_REG_PL301_DISP + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_DISP_AW (NX_VA_BASE_REG_PL301_DISP + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_SFR_QOS_TRDMARK                                           \
	(NX_VA_BASE_REG_PL301_SFR + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_SFR_QOS_CTRL                                              \
	(NX_VA_BASE_REG_PL301_SFR + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_SFR_AR (NX_VA_BASE_REG_PL301_SFR + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_SFR_AW (NX_VA_BASE_REG_PL301_SFR + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_STATIC_QOS_TRDMARK                                        \
	(NX_VA_BASE_REG_PL301_STATIC + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_STATIC_QOS_CTRL                                           \
	(NX_VA_BASE_REG_PL301_STATIC + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_STATIC_AR                                                 \
	(NX_VA_BASE_REG_PL301_STATIC + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_STATIC_AW                                                 \
	(NX_VA_BASE_REG_PL301_STATIC + NX_PL301_AW_OFFSET)

#define SI_IF_NUM_POS 0
#define SLOT_NUM_POS 24

// Interface slot number of  IOPERI_BUS.
#define NX_BUS_IOPERI_SI_SLOT_MP2TS 0 // DMAC0, DMAC1, MP2TS
#define NX_BUS_IOPERI_SI_SLOT_USB 1   // USB HOST, OTG
#define NX_BUS_IOPERI_SI_SLOT_SDMMC 2 // SDMMC 0~2
#define NX_BUS_IOPERI_SI_SLOT_GMAC 3

// Interface slot number of  BOTTOM_BUS.
#define NX_BUS_BOTTOM_SI_SLOT_MALI 0
#define NX_BUS_BOTTOM_SI_SLOT_CODA 1
#define NX_BUS_BOTTOM_SI_SLOT_SCALER 2
#define NX_BUS_BOTTOM_SI_SLOT_DEINTERLACE 3

// Interface slot number of  TOP_BUS.
#define NX_BUS_TOP_SI_SLOT_VIP_0 0
#define NX_BUS_TOP_SI_SLOT_VIP_1 1
#define NX_BUS_TOP_SI_SLOT_VIP_2 2
#define NX_BUS_TOP_SI_SLOT_IOPERI 3

// Interface slot number of  DISPLAY_BUS.
#define NX_BUS_DISPLAY_SI_SLOT_DISPLAY_0 0
#define NX_BUS_DISPLAY_SI_SLOT_DISPLAY_1 1

// Interface slot number of  SFR_BUS.
#define NX_BUS_SFR_SI_SLOT_CCI400 0
#define NX_BUS_SFR_SI_SLOT_MP2TS 1 // DMAC0, DMAC1, MP2TS

// Interface slot number of  STATIC_BUS.
#define NX_BUS_STATIC_SI_SLOT_CCI400 0
#define NX_BUS_STATIC_SI_SLOT_IOPERI_0 1
#define NX_BUS_STATIC_SI_SLOT_IOPERI_1 2

#endif // #ifndef __SYS_BUS_H__
