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

#ifndef __DDR3_AC_TIMING_H__
#define __DDR3_AC_TIMING_H__

/* Support the DDR3 Clock*/
#define DDR3_MEMCLK_533MHZ	0
#define DDR3_MEMCLK_666MHZ	1
#define DDR3_MEMCLK_800MHZ	3

/* User Define - DDR3 Device Specifiacation */
#define MEM_CLK			DDR3_MEMCLK_800MHZ

#define _DDR_CS_NUM		CONFIG_DDR3_CS_NUM				// CS
#define _DDR_BUS_WIDTH		CONFIG_DDR3_BUS_WIDTH				// 16bit, 8bit

#define _DDR_ROW_NUM		CONFIG_DDR3_ROW_NUM
#define _DDR_COL_NUM		CONFIG_DDR3_COLUMN_NUM
#define _DDR_BANK_NUM		CONFIG_DDR3_BANK_NUM

/* Refer to Memory(DREX, DPHY) Datasheet - Register Set*/
#define DDR3_MEM_SIZE		(0x40000000 >> 24)

#if 1
#define DDR3_MEM_MASK		(0x800 - DDR3_MEM_SIZE)				// Capacity per nCS: 2G=0x780, 1G=0x7C0(Tabletx2, VTK)
#else
#define DDR3_MEM_MASK		(0x800 - (DDR3_MEM_SIZE << 1))			// for Timing Calculation.
#endif

#define DDR3_CS_NUM		(_DDR_CS_NUM)
#define DDR_BUS_WIDTH		(_DDR_BUS_WIDTH)
#define DDR3_ROW_NUM		(_DDR_ROW_NUM - 12)				// ROW address bit : 15bit : 3(Tabletx2), 16bit : 4(Tabletx4, Elcomtech)
#define DDR3_COL_NUM		(_DDR_COL_NUM -  7)				// Column Address Bit. 2:9bit, 3:10bit, others:Reserved
#define DDR3_BANK_NUM		(_DDR_BANK_NUM)					// Bank bit : 3:8bank, 2:4bank
#define DDR3_CS0_BASEADDR	(0x040)
#define DDR3_CS1_BASEADDR	(DDR3_CS0_BASEADDR + DDR3_MEM_SIZE)

#define RDFETCH			0x1						// CONCONTROL.rd_fetch[14:12]

/* DDR3 AC Timing */
#if (MEM_CLK == DDR3_MEMCLK_800MHZ)
#define nCWL			8			// CAS Write Latency(CWL).
#define nCL			(11 + 0)		// CAS Latency(CL). Sometimes plus is needed.

#define MR0_nWR 		0x6

#define tPZQ			0x401B

#define tREFIPB			0x62
#define tREFI			0x618

#define tRFC			0x68			// 104
#define tRRD			0x3			// 3
#define tRP			0x6			// 6
#define tRCD			0x6			// 6
#define tRC			0x14			// 20
#define tRAS			0xE			// 14

#define tWTR			0x3			// 3
#define tWR			0x6			// 6
#define tRTP			0x3			// 3
#define tPPD			0x0			// 0
#define W2W_C2C			0x1			// 1
#define R2R_C2C			0x1			// 1
#define tDQSCK			0x0			// 0

#define tFAW			0x10			// 16
#define tXSR			0x100			// 256
#define tXP			0xA			// 10
#define tCKE			0x2			// 2
#define tMRD			0x6			// 6

#endif  //#if (MEM_CLK == DDR3_MEMCLK_800MHZ)


#if (MEM_CLK == DDR3_MEMCLK_666MHZ)
#define nCW			7			// CAS Write Latency(CWL).
#define nCL			(9 + 0)			// CAS Latency(CL). Sometimes plus is needed.

#define MR0_nWR			0x5

#define tPZQ			0x401B

#define tREFIPB			0x52
#define tREFI			0x513

#define tRFC			0x57
#define tRRD			0x3
#define tRP			0x5
#define tRCD			0x6
#define tRC			0x11
#define tRAS			0xD

#define tWTR			0x3
#define tWR			0x5
#define tRTP			0x3
#define tPPD			0x0			// 0:LPDDR3-1600, 1:LPDDR3-1866/2133
#define W2W_C2C			0x1
#define R2R_C2C			0x1
#define tDQSCK			0x0			// DDR3 : 0

#define tFAW			0xF
#define tXSR			0x100
#define tXP			0x8
#define tCKE			0x2
#define tMRD			0x6
#endif  //#if (MEM_CLK == DDR3_MEMCLK_666MHZ)

#if (MEM_CLK == DDR3_MEMCLK_533MHZ)
#define nCWL			6			// CAS Write Latency(CWL).
#define nCL			(7 + 0)			// CAS Latency(CL). Sometimes plus is needed.

#define MR0_nWR			0x4

#define tPZQ			0x4010

#define tREFIPB			0x41
#define tREFI			0x410

#define tRFC			0x46
#define tRRD			0x3
#define tRP			0x4
#define tRCD			0x4
#define tRC			0xE
#define tRAS			0xA

#define tWTR			0x2
#define tWR			0x4
#define tRTP			0x2
#define tPPD			0x0			// 0:LPDDR3-1600, 1:LPDDR3-1866/2133
#define W2W_C2C			0x1
#define R2R_C2C			0x1
#define tDQSCK			0x0			// DDR3 : 0

#define tFAW			0xE
#define tXSR			0x100
#define tXP			0x7
#define tCKE			0x2
#define tMRD			0x6
#endif  //#if (MEM_CLK == DDR3_MEMCLK_533MHZ)

/* User Define - DDR3 Device Mode Registers */
#define MR1_nAL         	0               // Posted CAS additive latency.  0 : Disable, 1 : CL - 1, 2 : CL - 2

#if (MR1_nAL > 0)
#define nAL             	(nCL - MR1_nAL)
#else
#define nAL             	0
#endif

#define nWL             	(nAL + nCWL)
#define nRL             	(nAL + nCL)

/*
  * Cas Latency for
  * 0:5ck(tCK>2.5ns), 1:6ck(2.5ns>tCK>1.875ns), 2:7ck(1.875ns>tCK>2.5ns), 3:8ck(1.5ns>tCK>1.25ns),
  * 4:9ck(1.25ns>tCK>1.07ns), 5:10ck(1.07ns>tCK>0.935ns), 6:11ck(0.935ns>tCK>0.833ns), 7:12ck(0.833ns>tCK>0.75ns)
  */
#define MR2_nCWL        	(nCWL - 5)      // CAS Write Latency(CWL).

#define	nMR1_AL			0
#define nMR1_ODS		0
#define nMR1_RTT_Nom		2

#define nMR2_RTT_WR		1

#endif  //#ifndef __DDR3_AC_TIMING_H__