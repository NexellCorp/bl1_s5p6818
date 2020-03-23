/*
 * Copyright (C) 2019  Nexell Co., Ltd.
 * Author: Sungwoo, Park <swpark@nexell.co.kr>
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
#ifndef __S5P6818_CON_SVMA_H__
#define __S5P6818_CON_SVMA_H__

#include <clock.h>
#include <freq.h>

/* system clock
 * Total 4 PLLs
 * See datasheet 4.9
 */
#ifdef	Q100
#define CONFIG_S5P_PLL0_FREQ	600  /* clock source for bus, memory, ogl, mpeg */
#define CONFIG_S5P_PLL1_FREQ	800  /* clock source for cpu */
#define CONFIG_S5P_PLL2_FREQ	614  /* clock source for i2s 0 */
#define CONFIG_S5P_PLL3_FREQ	800 /* clock source for i2s 1 */
#else
#define CONFIG_S5P_PLL0_FREQ	800  /* clock source for bus, memory, ogl, mpeg */
#define CONFIG_S5P_PLL1_FREQ	800  /* clock source for cpu */
#define CONFIG_S5P_PLL2_FREQ	614  /* clock source for i2s 0 */
#define CONFIG_S5P_PLL3_FREQ	800 /* clock source for i2s 1 */
#endif	/*Q100*/

#define CPU_CLUSTER0_CLK_SOURCE	CLKSRC_PLL_1
#define CPU_CLUSTER1_CLK_SOURCE	CLKSRC_PLL_1
#define BUS_CLK_SOURCE		CLKSRC_PLL_0
#define MEM_CLK_SOURCE		CLKSRC_PLL_3
#define OGL_CLK_SOURCE		CLKSRC_PLL_0
#define MPEG_CLK_SOURCE		CLKSRC_PLL_0
#define DISP_CLK_SOURCE		CLKSRC_PLL_0
#define HDMI_CLK_SOURCE		CLKSRC_PLL_0
#define CCI_CLK_SOURCE		CLKSRC_PLL_0

/* CPU cluster clock
 * FCLK: 800MHz
 * HCLK: 200MHz
 */
#define CONFIG_S5P_PLLx_DVO0	((CPU_CLUSTER0_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((1 - 1) << DVO0_BITPOS)		    | \
				 ((4 - 1) << DVO1_BITPOS))
#define CONFIG_S5P_PLLx_DVO7	((CPU_CLUSTER1_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((1 - 1) << DVO0_BITPOS)		    | \
				 ((4 - 1) << DVO1_BITPOS))

/* BUS clock
 * BCLK: 400MHz
 * PCLK: 200MHz
 */
#define CONFIG_S5P_PLLx_DVO1	((BUS_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((2 - 1) << DVO0_BITPOS)	   | \
				 ((2 - 1) << DVO1_BITPOS))

/* MEMORY clock
 * MDCLK(Memory DLL Clock): 800MHz
 * MCLK(Memory DDR Clock) : 800MHz
 * MBCLK(MCU BUS Clock)   : 400MHz
 * MPCLK(MCU PERI CLOCK)  : 200MHz
 */
#define CONFIG_S5P_PLLx_DVO2	((MEM_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((1 - 1) << DVO0_BITPOS)	   | \
				 ((1 - 1) << DVO1_BITPOS)	   | \
				 ((2 - 1) << DVO2_BITPOS)	   | \
				 ((2 - 1) << DVO3_BITPOS))

/* OGL clock
 * BCLK: 400MHz
 * PCLK: 200MHz
 */
#define CONFIG_S5P_PLLx_DVO3	((OGL_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((2 - 1) << DVO0_BITPOS)	   | \
				 ((2 - 1) << DVO1_BITPOS))

/* MPEG clock
 * BCLK: 400MHz
 * PCLK: 200MHz
 */
#define CONFIG_S5P_PLLx_DVO4	((MPEG_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((2 - 1) << DVO0_BITPOS)	    | \
				 ((2 - 1) << DVO1_BITPOS))

/* DISP clock
 * BCLK: 400MHz
 * PCLK: 200MHz
 */
#define CONFIG_S5P_PLLx_DVO5	((DISP_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((2 - 1) << DVO0_BITPOS)	    | \
				 ((2 - 1) << DVO1_BITPOS))

/* HDMI clock
 * PCLK: 100MHz
 */
#define CONFIG_S5P_PLLx_DVO6	((HDMI_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((8 - 1) << DVO0_BITPOS))


/* CCI clock
 * BCLK: 400MHz
 * PCLK: 200MHz
 */
#define CONFIG_S5P_PLLx_DVO8	((CCI_CLK_SOURCE << CLKSRC_BITPOS) | \
				 ((2 - 1) << DVO0_BITPOS)	   | \
				 ((2 - 1) << DVO1_BITPOS))

/* DDR3 Configuration */
#define CONFIG_DDR3_MEMCLK	800
#define CONFIG_DDR3_CS_NUM	2
#define CONFIG_DDR3_BANK_NUM	3
#define CONFIG_DDR3_ROW_NUM	15
#define CONFIG_DDR3_COLUMN_NUM	10
#define CONFIG_DDR3_BUS_WIDTH	16

#define CONFIG_DDR3_CHIP_PERSIZE ((((1 << CONFIG_DDR3_ROW_NUM)/8) * \
				   (1 << CONFIG_DDR3_COLUMN_NUM) *  \
				   (1 << CONFIG_DDR3_BANK_NUM) *    \
				   CONFIG_DDR3_BUS_WIDTH))

#define CONFIG_DDR3_CS_PERSIZE	 (CONFIG_DDR3_CHIP_PERSIZE * \
				  (32/CONFIG_DDR3_BUS_WIDTH))

#define CONFIG_DDR3_MEMSIZE	 ((((1 << CONFIG_DDR3_ROW_NUM)/8) * \
				   (1 << CONFIG_DDR3_COLUMN_NUM) *  \
				   (1 << CONFIG_DDR3_BANK_NUM) *    \
				   (CONFIG_DDR3_CS_NUM * 32)))

/* DDR3 Driver Strength Configuration */
#define CONFIG_DRAM_MR1_ODS     0
#define CONFIG_DRAM_MR1_RTT_Nom 2
#define CONFIG_DRAM_MR2_RTT_WR  2

#define CONFIG_DPHY_DRVDS_BYTE0 6
#define CONFIG_DPHY_DRVDS_BYTE1 6
#define CONFIG_DPHY_DRVDS_BYTE2 6
#define CONFIG_DPHY_DRVDS_BYTE3 6
#define CONFIG_DPHY_DRVDS_CK    6
#define CONFIG_DPHY_DRVDS_CKE   6
#define CONFIG_DPHY_DRVDS_CS    6
#define CONFIG_DPHY_DRVDS_CA    6

#define CONFIG_DPHY_ZQ_DDS      5
#define CONFIG_DPHY_ZQ_ODT      2

#define CONFIG_DDR3_WRITE_LVL_EN 0
#define CONFIG_DDR3_READ_DQ_EN   0
#define CONFIG_DDR3_WRITE_DQ_EN  0
#define CONFIG_DDR3_GATE_LVL_EN  1

#define CONFIG_DDR3_LVLTR_EN     (CONFIG_DDR3_WRITE_LVL_EN << 0) | \
	                         (CONFIG_DDR3_GATE_LVL_EN  << 2) | \
                                 (CONFIG_DDR3_READ_DQ_EN   << 3) | \
                                 (CONFIG_DDR3_WRITE_DQ_EN  << 4)

/* Debug Port */
#define CONFIG_S5P_SERIAL_INDEX  0
#define CONFIG_BAUDRATE          115200

/* PMIC Config */
#define NXE2000_PMIC_ENABLE
#define AUTO_VOLTAGE_CONTROL     1
#define ARM_VOLTAGE_CONTROL_SKIP 0

#endif /* __S5P6818_CON_SVMA_H__ */
