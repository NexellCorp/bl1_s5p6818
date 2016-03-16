/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : DDR3_K4B8G1646B_MCK0
 *      File            : DDR3_K4B8G1646B_MCK0.h
 *      Description     :
 *      Author          : Kook
 *      History         :
 */
#ifndef __DDR3_K4B8G1646B_MCK0_H__
#define __DDR3_K4B8G1646B_MCK0_H__

// Clock List
#define DDR3_533 0
#define DDR3_666 1
#define DDR3_800 3

// User Define
//#define MEM_CLK         DDR3_533
//#define MEM_CLK         DDR3_666
#define MEM_CLK DDR3_800

//#define _DDR_CS_NUM     2               // nCS Number : Tablet=1, VTK=2
#define _DDR_CS_NUM 1     // nCS Number : Tablet=1, VTK=2
#define _DDR_BUS_WIDTH 16 // 16bit, 8bit

#define _DDR_ROW_NUM 15
#define _DDR_COL_NUM 10
#define _DDR_BANK_NUM 8

// Refer to Memory Datasheet
#define cs_size (0x40000000 >> 24)

#define chip_row                                                               \
	(_DDR_ROW_NUM - 12) // ROW address bit : 15bit : 3(Tabletx2), 16bit :
			    // 4(Tabletx4, Elcomtech)
#define chip_col                                                               \
	(_DDR_COL_NUM -                                                        \
	 7)	   // Column Address Bit. 2:9bit, 3:10bit, others:Reserved
#define chip_bank (3) // Bank bit : 3:8bank, 2:4bank
#define chip_base0 0x040
#define chip_base1 (chip_base0 + cs_size)

#if 1
#define chip_mask                                                              \
	(0x800 - cs_size) // Capacity per nCS: 2G=0x780, 1G=0x7C0(Tabletx2, VTK)
#else
#define chip_mask (0x800 - (cs_size << 1)) // for Timing Calculation.
#endif

#define RDFETCH 0x1 // CONCONTROL.rd_fetch[14:12]

#if 1
#define READDELAY 0x0C0C0C0C  //- ctrl_offsetr, 0x0C0C0C0C, 0x18181818
#define WRITEDELAY 0x02020202 //- ctrl_offsetw, 0x04040404, 0x02020202
#else
#define READDELAY 0x08080808
#define WRITEDELAY 0x08080808
#endif

// Option
#define CONFIG_ODTOFF_GATELEVELINGON 0 // Not support yet.

// AC Timing
#if (MEM_CLK == DDR3_800)

#define nCWL 8       // CAS Write Latency(CWL).
#define nCL (11 + 0) // CAS Latency(CL). Sometimes plus is needed.

#define MR0_nWR 0x6

#define tPZQ 0x401B

#define tREFIPB 0x62
#define tREFI 0x618

#define tRFC 0x68
#define tRRD 0x3
#define tRP 0x6
#define tRCD 0x6
#define tRC 0x14
#define tRAS 0xF

#define tWTR 0x3
#define tWR 0x6
#define tRTP 0x3
#define tPPD 0x0
#define W2W_C2C 0x1
#define R2R_C2C 0x1
#define tDQSCK 0x0

#define tFAW 0x10
#define tXSR 0x100
#define tXP 0xA
#define tCKE 0x2
#define tMRD 0x6

#endif //#if (MEM_CLK == DDR3_800)

#if (MEM_CLK == DDR3_666)

#define nCWL 7      // CAS Write Latency(CWL).
#define nCL (9 + 0) // CAS Latency(CL). Sometimes plus is needed.

#define MR0_nWR 0x5

#define tPZQ 0x4017

#define tREFIPB 0x52
#define tREFI 0x513

#define tRFC 0x57
#define tRRD 0x3
#define tRP 0x5
#define tRCD 0x6
#define tRC 0x11
#define tRAS 0xD

#define tWTR 0x3
#define tWR 0x5
#define tRTP 0x3
#define tPPD 0x0 // 0:LPDDR3-1600, 1:LPDDR3-1866/2133
#define W2W_C2C 0x1
#define R2R_C2C 0x1
#define tDQSCK 0x0 // DDR3 : 0

#define tFAW 0xF
#define tXSR 0x100
#define tXP 0x8
#define tCKE 0x2
#define tMRD 0x6

#endif //#if (MEM_CLK == DDR3_666)

#if (MEM_CLK == DDR3_533)

#define nCWL 6      // CAS Write Latency(CWL).
#define nCL (7 + 0) // CAS Latency(CL). Sometimes plus is needed.

#define MR0_nWR 0x4

#define tPZQ 0x4010

#define tREFIPB 0x41
#define tREFI 0x410

#define tRFC 0x46
#define tRRD 0x3
#define tRP 0x4
#define tRCD 0x4
#define tRC 0xE
#define tRAS 0xA

#define tWTR 0x2
#define tWR 0x4
#define tRTP 0x2
#define tPPD 0x0 // 0:LPDDR3-1600, 1:LPDDR3-1866/2133
#define W2W_C2C 0x1
#define R2R_C2C 0x1
#define tDQSCK 0x0 // DDR3 : 0

#define tFAW 0xE
#define tXSR 0x100
#define tXP 0x7
#define tCKE 0x2
#define tMRD 0x6

#endif //#if (MEM_CLK == DDR3_533)

#define MR1_nAL 0 // Posted CAS additive latency.
		  // 0 : Disable
		  // 1 : CL - 1
		  // 2 : CL - 2

#if (MR1_nAL > 0)
#define nAL (nCL - MR1_nAL)
#else
#define nAL 0
#endif

#define nWL (nAL + nCWL)
#define nRL (nAL + nCL)

#define MR2_nCWL (nCWL - 5) // CAS Write Latency(CWL).
// 0:5ck(tCK>2.5ns), 1:6ck(2.5ns>tCK>1.875ns), 2:7ck(1.875ns>tCK>2.5ns),
// 3:8ck(1.5ns>tCK>1.25ns),
// 4:9ck(1.25ns>tCK>1.07ns), 5:10ck(1.07ns>tCK>0.935ns),
// 6:11ck(0.935ns>tCK>0.833ns), 7:12ck(0.833ns>tCK>0.75ns)

#endif //#ifndef __DDR3_K4B8G1646B_MCK0_H__
