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
 *      Module          : DDR SDRAM
 *      File            : ddr3_sdram.h
 *      Description     :
 *      Author          : Hans
 *      History         : 2014.08.21 Hans Create
 */

#ifndef __NX_DDR3_SDRAM_H__
#define __NX_DDR3_SDRAM_H__

typedef enum {
	SDRAM_MODE_REG_MR0 = 0,
	SDRAM_MODE_REG_MR1 = 1,
	SDRAM_MODE_REG_MR2 = 2,
	SDRAM_MODE_REG_MR3 = 3,
	SDRAM_MODE_REG_MAX_MRn = 0xFF
} SDRAM_MODE_REG;

struct SDRAM_MR0 {
	volatile U16 BL : 2;  // 0 Burst Length	( 00: Fixed BL8, 01: 4 or
			      // 8 (A12), 10: Fixed BC4, 11: Reserved)
	volatile U16 CL0 : 1; // 2 CAS Latency 0 bit (0000: Reserved, 0010: 5,
			      // 0100: 6, 0110: 7, 1000: 8, 1010: 9, 1100:10,
			      // 1110:11, 0001:12, 0011:13
	volatile U16
	    BT : 1; // 3 READ Burst Type (0: Sequential(nibble), 1: Interleaved)
	volatile U16 CL1 : 3;  // 4 CAS Latency 0 bit
	volatile U16 SBZ0 : 1; // 7
	volatile U16 DLL : 1;  // 8 DLL Reset ( 0: No, 1: Yes)
	volatile U16 WR : 3; // 9 Write Recovery (000: Reserved, 001: 5, 010: 6,
			     // 011: 7, 100: 8, 101: 10, 110: 12, 111: 14)
	volatile U16 PD : 1; // 12 Precharge PD (0:DLL off (slow exit), 1:DLL on
			     // (fast exit)
	volatile U16 SBZ1 : 3; // 13 ~ 15
};
struct SDRAM_MR1 {
	volatile U16 DLL : 1;      // 0 DLL Enable (0: enable, 1: disable)
	volatile U16 ODS0 : 1;     // 1 Output Drive Strenghth 0 bit (00:
				   // RZQ/6(40ohm), 01: RZQ/7(34ohm), 10, 11:
				   // reserved)
	volatile U16 RTT_Nom0 : 1; // 2
	volatile U16 AL : 2;       // 3 Additive Latency	(00: disable, 01:
				   // AL=CL-1, 10: AL=CL-2, 11: Reserved)
	volatile U16 ODS1 : 1;     // 5 Output Drive Strenghth 1 bit
	volatile U16 RTT_Nom1 : 1; // 6
	volatile U16 WL : 1;   // 7 Write Levelization (0: disable, 1: enable)
	volatile U16 SBZ0 : 1; // 8
	volatile U16 RTT_Nom2 : 1; // 9 (000: disable, 001: RZQ/4(60ohm), 010:
				   // RZQ/2(120ohm), 011: RZQ/6(40ohm), 100:
				   // RZQ/12(20ohm), 101: RZQ/8(30ohm) 110, 111:
				   // Reserved)
	volatile U16 SBZ1 : 1;     // 10
	volatile U16 TDQS : 1;     // 11 TDQS (0: disable, 1: enable)
	volatile U16 QOff : 1;     // 12 Q Off (0: enabled, 1: disabled)
	volatile U16 SBZ2 : 3;     // 13 ~ 15
};

struct SDRAM_MR2 {
	volatile U16 SBZ0 : 3; // 0
	volatile U16 CWL : 3; // 3 CAS Write Latency (000: 5CK(tCK>=2.5ns), 001:
			      // 6CK(2.5ns>tCK>=1.875ns, 010:
			      // 7CK(1.875ns>tCK>=1.25ns), 011:
			      // 8CK(1.5ns>tCK>=1.25ns), 100:
			      // 9CK(1.25ns>tCK>=1.07ns))
	volatile U16 ASR : 1; // 6 Auto Self Refresh(option) (0:
			      // disabled:Manual, 1:Enabled: Automatic)
	volatile U16 SRT : 1; // 7 Self Refresh Temperature (0: Normal(0 to 85
			      // degree), 1: Extended(0 to 95 degree)
	volatile U16 SBZ1 : 1;   // 8
	volatile U16 RTT_WR : 2; // 9 Dynamic ODT (00: Rtt disabled, 01: RZQ/4,
				 // 10: RZQ/2, 11: reserved)
	volatile U16 SBZ2 : 5;   // 11 ~ 15
};

struct SDRAM_MR3 {
	volatile U16 MPR_RF : 2; // 0 MPR Read Function (00: Predefined pattern,
				 // 01, 10, 11: reserved)
	volatile U16 MPR : 1;    // 2 MPR Enable (0: Normal DRAM operation, 1:
				 // Dataflow from MPR)
	volatile U16 SBZ0 : 13;  // 3 ~ 15
};

struct LPDDR3_MR0 {
	volatile U16 DAI : 1;   // 0 Device Auto-Initialization Status( 0: DAI
				// still in progress)
	volatile U16 _RFU0 : 2; // 1 ~ 2
	volatile U16 RZQI : 2;  // 3 ~ 4 RZQ Information
	volatile U16 _RFU1 : 1; // 5
	volatile U16 WL : 1;    // 6
	volatile U16 RL3 : 1;   // 7
	volatile U16 _reserved : 8; // 8 ~ 15
};
struct LPDDR3_MR1 {
	volatile U16 BL : 3;	// 0 ~ 2
	volatile U16 _RFU0 : 2;     // 3 ~ 4
	volatile U16 WR : 3;	// 5 ~ 7
	volatile U16 _reserved : 8; // 8 ~ 15
};

struct LPDDR3_MR2 {
	volatile U16 RL_WL : 4;     // 0 ~ 3
	volatile U16 WRE : 1;       // 4
	volatile U16 _RFU0 : 1;     // 5
	volatile U16 WL_SEL : 1;    // 6
	volatile U16 WR_LVL : 1;    // 7
	volatile U16 _reserved : 8; // 8 ~ 15
};

struct LPDDR3_MR3 {
	volatile U16 DS : 4;	// 0 ~ 3
	volatile U16 _RFU0 : 4;     // 4 ~ 7
	volatile U16 _reserved : 8; // 8 ~ 15
};

struct LPDDR3_MR11 {
	volatile U16 DQ_ODT : 2;    // 0 ~ 1
	volatile U16 PD_CON : 1;    // 2
	volatile U16 _RFU0 : 5;     // 3 ~ 7
	volatile U16 _reserved : 8; // 8 ~ 15
};

union SDRAM_MR {
	struct SDRAM_MR0 MR0;
	struct SDRAM_MR1 MR1;
	struct SDRAM_MR2 MR2;
	struct SDRAM_MR3 MR3;

	struct LPDDR3_MR0 LP_MR0;
	struct LPDDR3_MR1 LP_MR1;
	struct LPDDR3_MR2 LP_MR2;
	struct LPDDR3_MR3 LP_MR3;
	struct LPDDR3_MR11 LP_MR11;

	volatile U16 Reg;
};

#endif // __NX_DDR3_SDRAM_H__
