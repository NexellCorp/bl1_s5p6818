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
#ifndef __S5P6818_AVN_REF_H__
#define __S5P6818_AVN_REF_H__

#include <clock.h>

/* clock (PLLx) configuration */
#define OSC_HZ					(24000000)
#define OSC_KHZ					(OSC_HZ/1000)
#define OSC_MHZ					(OSC_KHZ/1000)

#define CLKSRC_PLL_0				0
#define CLKSRC_PLL_1				1
#define CLKSRC_PLL_2				2
#define CLKSRC_PLL_3				3

#define CLKSRC_UART				CLKSRC_PLL_2
#define CLKSRC_SDMMC				CLKSRC_PLL_2
#define CLKSRC_SPI				CLKSRC_PLL_0

/* system clock macro */
#define CONFIG_S5P_PLL0_FREQ			800
#define CONFIG_S5P_PLL1_FREQ			800
#define CONFIG_S5P_PLL2_FREQ			614
#define CONFIG_S5P_PLL3_FREQ			800

// CPUDVOREG
#define CONFIG_S5P_PLLx_DVO0			((CLKSRC_PLL_1 << CLKSRC_BITPOS) | \
						((1 - 1) << DVO0_BITPOS)	 | \
	                			((4 - 1) << DVO1_BITPOS))
// CPUDVOREG
#define CONFIG_S5P_PLLx_DVO7			((CLKSRC_PLL_1 << CLKSRC_BITPOS) | \
	                			((1 - 1) << DVO0_BITPOS)	 | \
	                			((4 - 1) << DVO1_BITPOS))
// BUSDVOREG
#define CONFIG_S5P_PLLx_DVO1			((CLKSRC_PLL_0 << CLKSRC_BITPOS) | \
	                			((2 - 1) << DVO0_BITPOS)	 | \
	                			((2 - 1) << DVO1_BITPOS))
// MEMDVOREG
#define CONFIG_S5P_PLLx_DVO2			((CLKSRC_PLL_3 << CLKSRC_BITPOS) | \
						((1 - 1) << DVO0_BITPOS)	 | \
						((1 - 1) << DVO1_BITPOS) 	 | \
						((2 - 1) << DVO2_BITPOS) 	 | \
						((2 - 1) << DVO3_BITPOS))

#define CONFIG_S5P_PLLx_DVO3			((CLKSRC_PLL_0 << CLKSRC_BITPOS) | \
						((2 - 1) << DVO0_BITPOS)	 | \
						((2 - 1) << DVO1_BITPOS))


#define CONFIG_S5P_PLLx_DVO4			((CLKSRC_PLL_0 << CLKSRC_BITPOS) | \
						((2 - 1) << DVO0_BITPOS)	 | \
						((2 - 1) << DVO1_BITPOS))

#define CONFIG_S5P_PLLx_DVO5			((CLKSRC_PLL_0 << CLKSRC_BITPOS) | \
						((2 - 1) << DVO0_BITPOS)	 | \
						((2 - 1) << DVO1_BITPOS))

#define CONFIG_S5P_PLLx_DVO6			((CLKSRC_PLL_0 << CLKSRC_BITPOS) | \
						((8 - 1) << DVO0_BITPOS))

#define CONFIG_S5P_PLLx_DVO8			((CLKSRC_PLL_3 << CLKSRC_BITPOS) | \
						((2 - 1) << DVO0_BITPOS)	 | \
						((2 - 1) << DVO1_BITPOS))

/* sdram ddr3 configuration */
#define CONFIG_DDR3_CS_NUM			 1				// 1 : 1CS, 2: 2CS
#define CONFIG_DDR3_BANK_NUM			 3				// 3: 8 Bank
#define CONFIG_DDR3_ROW_NUM			15
#define CONFIG_DDR3_COLUMN_NUM			10

#define CONFIG_DDR3_BUS_WIDTH			16

/* device(dram) drive strength configuration */
#define CONFIG_DRAM_MR1_ODS			0				// MR1_ODS - 0: RZQ/6, 1 : RZQ/7
#define CONFIG_DRAM_MR1_RTT_Nom			3				// MR1_RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
#define CONFIG_DRAM_MR2_RTT_WR			1				// MR2_RTT_WR - 0: ODT disable, 1: RZQ/4, 2: RZQ/2

/* memory controller(dphy) drive strengh configuration  */
#define CONFIG_DPHY_DRVDS_BYTE0			4
#define CONFIG_DPHY_DRVDS_BYTE1			4
#define CONFIG_DPHY_DRVDS_BYTE2			4
#define CONFIG_DPHY_DRVDS_BYTE3			4
#define CONFIG_DPHY_DRVDS_CK			6
#define CONFIG_DPHY_DRVDS_CKE			6
#define CONFIG_DPHY_DRVDS_CS			6
#define CONFIG_DPHY_DRVDS_CA			6

#define CONFIG_DPHY_ZQ_DDS			4
#define CONFIG_DPHY_ZQ_ODT			1

/* ddr3 leveling & training configuration */
#define CONFIG_DDR3_WRITE_LVL_EN		0				// 0: Disable, 1: Enable
#define CONFIG_DDR3_READ_DQ_EN			1				// 0: Disable, 1: Enable
#define CONFIG_DDR3_WRITE_DQ_EN			1				// 0: Disable, 1: Enable
#define CONFIG_DDR3_GATE_LVL_EN			1				// 0: Disable, 1: Enable

#define CONFIG_DDR3_LVLTR_EN			(CONFIG_DDR3_WRITE_LVL_EN << 0) |	\
						(CONFIG_DDR3_GATE_LVL_EN  << 2)	|	\
						(CONFIG_DDR3_READ_DQ_EN	  << 3) |	\
						(CONFIG_DDR3_WRITE_DQ_EN  << 4)

/* serial console configuration */
#define CONFIG_S5P_SERIAL
#define CONFIG_S5P_SERIAL_INDEX			0
#define CONFIG_S5P_SERIAL_CLOCK			50000000

#define CONFIG_S5P_SERIAL_SRCCLK		0
#define CONFIG_S5P_SERIAL_DIVID			4

#define CONFIG_BAUDRATE				115200

#define CONFIG_UART_CLKGEN_CLOCK_HZ		0

/* pmic(power management ic) configuration */
#define NXE2000_PMIC_ENABLE			1

#define AUTO_VOLTAGE_CONTROL			1
#define ARM_VOLTAGE_CONTROL_SKIP		0

#endif // #ifndef __S5P6818_AVN_REF_H__
