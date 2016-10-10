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

#include "sysheader.h"

#include <nx_drex.h>
#include <nx_ddrphy.h>
#include "ddr3_sdram.h"

#define CFG_DDR_LOW_FREQ (1)

#define DDR_CA_SWAP_MODE (0) // for LPDDR3

#define DDR_WRITE_LEVELING_EN (0) // for fly-by
#define DDR_CA_CALIB_EN (1)       // for LPDDR3
#define DDR_CA_AUTO_CALIB (1)     // for LPDDR3
#define DDR_GATE_LEVELING_EN (1)  // for DDR3, great then 666MHz
#define DDR_READ_DQ_CALIB_EN (1)
#define DDR_WRITE_LATENCY_CALIB_EN (0) // for fly-by
#define DDR_WRITE_DQ_CALIB_EN (1)

#define DDR_RESET_GATE_LVL (1)
#define DDR_RESET_READ_DQ (1)
#define DDR_RESET_WRITE_DQ (1)

#if 1 //(CFG_NSIH_EN == 0)
#include "LPDDR3_K4E6E304EB_EGCE.h"
#endif

#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

extern void setMemPLL(int);
extern inline void DMC_Delay(int milisecond);

U32 g_Lock_Val;
U32 g_WR_lvl;
U32 g_GT_cycle;
U32 g_GT_code;
U32 g_RD_vwmc;
U32 g_WR_vwmc;

#if defined(MEM_TYPE_DDR3)
// inline
void SendDirectCommand(SDRAM_CMD cmd, U8 chipnum, SDRAM_MODE_REG mrx, U16 value)
{
	WriteIO32(
	    &pReg_Drex->DIRECTCMD,
	    (U32)((cmd << 24) | ((chipnum & 1) << 20) | (mrx << 16) | value));
}
#endif
#if defined(MEM_TYPE_LPDDR23)
// inline
void SendDirectCommand(SDRAM_CMD cmd, U8 chipnum, SDRAM_MODE_REG mrx, U16 value)
{
	WriteIO32(&pReg_Drex->DIRECTCMD,
		  (U32)((cmd << 24) | ((chipnum & 1) << 20) |
			(((mrx >> 3) & 0x7) << 16) | ((mrx & 0x7) << 10) |
			((value & 0xFF) << 2) | ((mrx >> 6) & 0x3)));
}
#endif

void enterSelfRefresh(void)
{
	//    union SDRAM_MR MR;
	U32 nTemp;
	U32 nChips = 0;

#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	nChips = 0x3;
#else
	nChips = 0x1;
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		nChips = 0x3;
	else
		nChips = 0x1;
#endif

	while (ReadIO32(&pReg_Drex->CHIPSTATUS) & 0xF) {
		nop();
	}

	/* Send PALL command */
	SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif
	DMC_Delay(100);

#if 0
    // Send MR16 PASR_Bank command.
    SendDirectCommand(SDRAM_CMD_MRS, 0, 16, 0);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, 16, 0);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, 16, 0);
#endif

    // Send MR17 PASR_Seg command.
    SendDirectCommand(SDRAM_CMD_MRS, 0, 17, 0);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, 17, 0);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, 17, 0);
#endif
#endif

#if 1
	do {
		/* Enter self-refresh command */
		SendDirectCommand(SDRAM_CMD_REFS, 0, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_REFS, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_REFS, 1,
					  (SDRAM_MODE_REG)CNULL, CNULL);
#endif

		nop();
		nTemp = ((ReadIO32(&pReg_Drex->CHIPSTATUS) >> 8) & nChips);
	} while (nTemp != nChips);
#else

	// for self-refresh check routine.
	while (1) {
		nTemp = ReadIO32(&pReg_Drex->CHIPSTATUS);
		if (nTemp)
			MEMMSG("ChipStatus = 0x%04x\r\n", nTemp);
	}
#endif

	// Step 52 Auto refresh counter disable
	ClearIO32(
	    &pReg_Drex->CONCONTROL,
	    (0x1
	     << 5)); // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1

	// Step 10  ACK, ACKB off
	SetIO32(&pReg_Drex->MEMCONTROL, (0x1 << 0)); // clk_stop_en[0] : Dynamic
						     // Clock Control   :: 1'b0
						     // - Always running

	DMC_Delay(1000 * 3);
}

void exitSelfRefresh(void)
{
	// Step 10    ACK, ACKB on
	ClearIO32(&pReg_Drex->MEMCONTROL, (0x1 << 0)); // clk_stop_en[0] :
						       // Dynamic Clock Control
						       // :: 1'b0  - Always
						       // running
	DMC_Delay(10);

	// Step 52 Auto refresh counter enable
	SetIO32(
	    &pReg_Drex->CONCONTROL,
	    (0x1
	     << 5)); // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1
	DMC_Delay(10);

	/* Send PALL command */
	SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif

#if 1
	while (ReadIO32(&pReg_Drex->CHIPSTATUS) & (0xF << 8)) {
		/* Exit self-refresh command */
		SendDirectCommand(SDRAM_CMD_REFSX, 0, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_REFSX, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_REFSX, 1,
					  (SDRAM_MODE_REG)CNULL, CNULL);
#endif

		nop();
	}
#endif

	DMC_Delay(1000 * 2);
}

#if (DDR_WRITE_LEVELING_EN == 1)
void DDR_Write_Leveling(void)
{
#if 0
    MEMMSG("\r\n########## Write Leveling ##########\r\n");

#else
#if defined(MEM_TYPE_DDR3)
	union SDRAM_MR MR1;
#endif
	U32 temp;

	MEMMSG("\r\n########## Write Leveling - Start ##########\r\n");

	SetIO32(&pReg_DDRPHY->PHY_CON[26 + 1],
		(0x3 << 7)); // cmd_default, ODT[8:7]=0x3
	SetIO32(&pReg_DDRPHY->PHY_CON[0], (0x1 << 16)); // wrlvl_mode[16]=1

#if defined(MEM_TYPE_DDR3)
	/* Set MPR mode enable */
	MR1.Reg = 0;
	MR1.MR1.DLL = 0; // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
	MR1.MR1.AL = MR1_nAL;
#else
	MR1.MR1.AL = pSBI->DII.MR1_AL;
#endif
	MR1.MR1.ODS1 = 0; // 00: RZQ/6, 01 : RZQ/7
	MR1.MR1.ODS0 = 1;
	MR1.MR1.QOff = 0;
	MR1.MR1.RTT_Nom2 = 0; // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6,
			      // 100: RZQ/12, 101: RZQ/8
	MR1.MR1.RTT_Nom1 = 1;
	MR1.MR1.RTT_Nom0 = 0;
	MR1.MR1.WL = 1;
#if (CFG_NSIH_EN == 0)
	MR1.MR1.TDQS = (_DDR_BUS_WIDTH >> 3) & 1;
#else
	MR1.MR1.TDQS = (pSBI->DII.BusWidth >> 3) & 1;
#endif

	SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1,
				  MR1.Reg);
#endif
#endif

#if 0
    // Send NOP command.
    SendDirectCommand(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#endif

	temp = ((0x8 << 24) | (0x8 << 17) | (0x8 << 8) |
		(0x8 << 0)); // PHY_CON30[30:24] = ctrl_wrlvl_code3,
			     // PHY_CON30[23:17] = ctrl_wrlvl_code2,
			     // PHY_CON30[14:8] = ctrl_wrlvl_code1,
			     // PHY_CON30[6:0] = ctrl_wrlvl_code0
	WriteIO32(&pReg_DDRPHY->PHY_CON[30 + 1], temp);
	MEMMSG("ctrl_wrlvl_code = 0x%08X\r\n", temp);

	// SDLL update.
	SetIO32(&pReg_DDRPHY->PHY_CON[30 + 1],
		(0x1 << 16)); // wrlvl_enable[16]=1, ctrl_wrlvl_resync
	ClearIO32(&pReg_DDRPHY->PHY_CON[30 + 1],
		  (0x1 << 16)); // wrlvl_enable[16]=0, ctrl_wrlvl_resync

	temp = ReadIO32(&pReg_DDRPHY->PHY_CON[30 + 1]); // PHY_CON30[30:24] =
							// ctrl_wrlvl_code3,
							// PHY_CON30[23:17] =
							// ctrl_wrlvl_code2,
							// PHY_CON30[14:8] =
							// ctrl_wrlvl_code1,
							// PHY_CON30[6:0] =
							// ctrl_wrlvl_code0
	MEMMSG("ctrl_wrlvl_code = 0x%08X\r\n", temp);

	ClearIO32(&pReg_DDRPHY->PHY_CON[0], (0x1 << 16)); // wrlvl_mode[16]=0
	ClearIO32(&pReg_DDRPHY->PHY_CON[26 + 1],
		  (0x3 << 7)); // cmd_default, ODT[8:7]=0x0

	MEMMSG("\r\n########## Write Leveling - End ##########\r\n");
#endif
}
#endif

#if (DDR_CA_CALIB_EN == 1)
// for LPDDR3
//*** Response for Issuing MR41 - CA Calibration Enter1
//- CA Data Pattern transfered at Rising Edge   : CA[9:0]=0x3FF     =>
//CA[8:5],CA[3:0] of Data Pattern transfered through MR41 is returned to DQ
//(CA[3:0]={DQ[6],DQ[4],DQ[2],DQ[0]}=0xF,
//CA[8:5]={DQ[14],DQ[12],DQ[10],DQ[8]}=0xF)
//- CA Data Pattern transfered at Falling Edge  : CA[9:0]=0x000     =>
//CA[8:5],CA[3:0] of Data Pattern transfered through MR41 is returned to DQ
//(CA[3:0]={DQ[7],DQ[5],DQ[3],DQ[1]}=0x0,
//CA[8:5]={DQ[15],DQ[13],DQ[11],DQ[9]}=0x0)
//- So response(ctrl_io_rdata) from MR41 is "0x5555".
//*** Response for Issuing MR48 - CA Calibration Enter2
//- CA Data Pattern transfered at Rising Edge   : CA[9], CA[4]=0x3  =>
//CA[9],CA[4] of Data Pattern transfered through MR48 is returned to DQ
//(CA[9]=DQ[8]=0x1, CA[4]=DQ[0]}=0x1)
//- CA Data Pattern transfered at Falling Edge  : CA[9], CA[4]=0x0  =>
//CA[9],CA[4] of Data Pattern transfered through MR48 is returned to DQ
//(CA[9]=DQ[9]=0x0, CA[4]=DQ[1]}=0x0)
//- So response(ctrl_io_rdata) from MR48 is "0x0101".
#define RESP_MR41 0x5555
#define RESP_MR48 0x0101

#define MASK_MR41 0xFFFF
#define MASK_MR48 0x0303

CBOOL DDR_CA_Calibration(void)
{
	CBOOL ret = CFALSE;
#if (DDR_CA_AUTO_CALIB == 1)
	U32 lock_div4 = (g_Lock_Val >> 2);
	U32 offsetd;
	U32 vwml, vwmr, vwmc;
	U32 temp, mr41, mr48;
	int find_vmw;
	int code;

	code = 0x8; // CMD SDLL Code default value "ctrl_offsetd"=0x8
	find_vmw = 0;
	vwml = vwmr = vwmc = 0;

	MEMMSG("\r\n########## CA Calibration - Start ##########\r\n");

#if (DDR_CA_SWAP_MODE == 1)
	SetIO32(&pReg_Tieoff->TIEOFFREG[3], (0x1 << 26)); // drex_ca_swap[26]=1
	SetIO32(&pReg_DDRPHY->LP_DDR_CON[2], (0x1 << 0)); // ca_swap_mode[0]=1
#endif

	SetIO32(&pReg_DDRPHY->PHY_CON[0],
		(0x1 << 16)); // ctrl_wrlvl_en(wrlvl_mode)[16]="1" (Enable)
	SetIO32(&pReg_DDRPHY->PHY_CON[2],
		(0x1 << 23)); // rdlvl_ca_en(ca_cal_mode)[23]="1" (Enable)

	SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif

	while (1) {
		temp = (ReadIO32(&pReg_DDRPHY->OFFSETD_CON) & (1 << 28)) | code;
		WriteIO32(&pReg_DDRPHY->OFFSETD_CON, temp);

		SetIO32(&pReg_DDRPHY->OFFSETD_CON,
			(0x1 << 24)); // ctrl_resync[24]=1
		ClearIO32(&pReg_DDRPHY->OFFSETD_CON,
			  (0x1 << 24)); // ctrl_resync[24]=0
		DMC_Delay(0x80);

		SendDirectCommand(SDRAM_CMD_MRS, 0, 41,
				  0xA4); //- CH0 : Send MR41 to start CA
					 //calibration for LPDDR3 : MA=0x29
					 //OP=0xA4, 0x50690
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 41,
				  0xA4); //- CH1 : Send MR41 to start CA
					 //calibration for LPDDR3 : MA=0x29
					 //OP=0xA4, 0x50690
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 41,
					  0xA4); //- CH1 : Send MR41 to start CA
						 //calibration for LPDDR3 :
						 //MA=0x29 OP=0xA4, 0x50690
#endif
#endif

		SetIO32(&pReg_Drex->CACAL_CONFIG[0],
			(0x1 << 0));    // deassert_cke[0]=1 : CKE pin is "LOW"
		temp = ((0x3FF << 12) | // dfi_address_p0[24]=0x3FF
			(tADR << 4) |
			(0x1 << 0)); // deassert_cke[0]=1 : CKE pin is "LOW"
		WriteIO32(&pReg_Drex->CACAL_CONFIG[0], temp);

		WriteIO32(&pReg_Drex->CACAL_CONFIG[1],
			  0x00000001); // cacal_csn(dfi_csn_p0)[0]=1 : generate
				       // one pulse CSn(Low and High), cacal_csn
				       // field need not to return to "0" and
				       // whenever this field is written in "1",
				       // one pulse is genrerated.
				       //        DMC_Delay(0x80);

		mr41 = ReadIO32(&pReg_Drex->CTRL_IO_RDATA) & MASK_MR41;

		ClearIO32(&pReg_Drex->CACAL_CONFIG[0],
			  (0x1 << 0)); // deassert_cke[0]=0 : CKE pin is "HIGH"
				       // - Normal operation
		DMC_Delay(0x80);

		SendDirectCommand(SDRAM_CMD_MRS, 0, 48,
				  0xC0); // CH0 : Send MR48 to start CA
					 // calibration for LPDDR3 : MA=0x30
					 // OP=0xC0, 0x60300
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 48,
				  0xC0); // CH1 : Send MR48 to start CA
					 // calibration for LPDDR3 : MA=0x30
					 // OP=0xC0, 0x60300
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 48,
					  0xC0); // CH1 : Send MR48 to start CA
						 // calibration for LPDDR3 :
						 // MA=0x30 OP=0xC0, 0x60300
#endif
#endif

		SetIO32(&pReg_Drex->CACAL_CONFIG[0],
			(0x1 << 0)); // deassert_cke[0]=1 : CKE pin is "LOW"
		WriteIO32(&pReg_Drex->CACAL_CONFIG[1],
			  0x00000001); // cacal_csn(dfi_csn_p0)[0]=1 : generate
				       // one pulse CSn(Low and High), cacal_csn
				       // field need not to return to "0" and
				       // whenever this field is written in "1",
				       // one pulse is genrerated.

		mr48 = ReadIO32(&pReg_Drex->CTRL_IO_RDATA) & MASK_MR48;

		ClearIO32(&pReg_Drex->CACAL_CONFIG[0],
			  (0x1 << 0)); // deassert_cke[0]=0 : CKE pin is "HIGH"
				       // - Normal operation

		if (find_vmw < 0x3) {
			if ((mr41 == RESP_MR41) && (mr48 == RESP_MR48)) {
				find_vmw++;
				if (find_vmw == 0x1) {
					vwml = code;
				}
				//                printf("+ %d\r\n", code);
			} else {
				find_vmw = 0x0; //- 첫 번째 PASS로부터 연속 3회
						//PASS 하지 못하면 연속 3회
						//PASS가 발생할 때까지 Searching
						//다시 시작하도록 "find_vmw" =
						//"0"으로 초기화.
				//                printf("- %d\r\n", code);
			}
		} else if ((mr41 != RESP_MR41) || (mr48 != RESP_MR48)) {
			find_vmw = 0x4;
			vwmr = code - 1;
			//            printf("-- %d\r\n", code);
			printf("mr41 = 0x%08X, mr48 = 0x%08X\r\n", mr41, mr48);
			break;
		}

		code++;

		if (code == 256) {
			MEMMSG("[Error] CA Calibration : code %d\r\n", code);

			goto ca_error_ret;
		}
	}

	lock_div4 = (g_Lock_Val >> 2);

	vwmc = (vwml + vwmr) >> 1;
	code = (int)(vwmc - lock_div4); //- (g_DDRLock >> 2) means "T/4", lock
					//value means the number of delay cell
					//for one period

	offsetd = (vwmc & 0xFF);

	ret = CTRUE;

ca_error_ret:

	temp = ReadIO32(&pReg_DDRPHY->OFFSETD_CON) & (1 << 28);

	if (ret == CFALSE) {
		WriteIO32(&pReg_DDRPHY->OFFSETD_CON, temp | 0x08);
	} else {
		WriteIO32(&pReg_DDRPHY->OFFSETD_CON, temp | offsetd);
	}

	SetIO32(&pReg_DDRPHY->OFFSETD_CON,
		(0x1 << 24)); // ctrl_resync[24]=0x1 (HIGH)
	ClearIO32(&pReg_DDRPHY->OFFSETD_CON,
		  (0x1 << 24)); // ctrl_resync[24]=0x0 (LOW)

	ClearIO32(&pReg_DDRPHY->PHY_CON[0],
		  (0x1 << 16)); // ctrl_wrlvl_en(wrlvl_mode)[16]="0"(Disable)

	// Exiting Calibration Mode of LPDDR3 using MR42
	SendDirectCommand(SDRAM_CMD_MRS, 0, 42,
			  0xA8); // CH0 : Send MR42 to exit from CA calibration
				 // mode for LPDDR3, MA=0x2A OP=0xA8, 0x50AA0
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRS, 1, 42,
			  0xA8); // CH1 : Send MR42 to exit from CA calibration
				 // mode for LPDDR3, MA=0x2A OP=0xA8, 0x50AA0
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 42,
				  0xA8); // CH1 : Send MR42 to exit from CA
					 // calibration mode for LPDDR3, MA=0x2A
					 // OP=0xA8, 0x50AA0
#endif
#endif

#else

	printf("\r\n########## CA Calibration - Start ##########\r\n");

	WriteIO32(&pReg_DDRPHY->PHY_CON[10],
		  0x37); // Set CA delay time. - Miware value OK

	SetIO32(&pReg_DDRPHY->PHY_CON[10],
		(0x1 << 24)); // ctrl_resync[24]=0x1 (HIGH)
	ClearIO32(&pReg_DDRPHY->PHY_CON[10],
		  (0x1 << 24)); // ctrl_resync[24]=0x0 (LOW)
	DMC_Delay(0x80);
#endif // #if (DDR_CA_AUTO_CALIB == 1)

	MEMMSG("\r\n########## CA Calibration - End ##########\r\n");

	return ret;
}
#endif

#if (DDR_GATE_LEVELING_EN == 1)
CBOOL DDR_Gate_Leveling(void)
{
#if defined(MEM_TYPE_DDR3)
	union SDRAM_MR MR;
#endif
	volatile U32 cal_count = 0;
	U32 temp;
	CBOOL ret = CTRUE;

	MEMMSG("\r\n########## Gate Leveling - Start ##########\r\n");

	SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif

#if defined(MEM_TYPE_DDR3)
	/* Set MPR mode enable */
	MR.Reg = 0;
	MR.MR3.MPR = 1;

	SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif // #if defined(MEM_TYPE_DDR3)
#if defined(MEM_TYPE_LPDDR23)
	temp = ReadIO32(&pReg_DDRPHY->PHY_CON[1]) & 0xFFFF0000;
	temp |= 0x00FF; // rdlvl_rddata_adj[15:0]
	//    temp |= 0x0001; // rdlvl_rddata_adj[15:0]
	WriteIO32(&pReg_DDRPHY->PHY_CON[1], temp);

	SendDirectCommand(SDRAM_CMD_MRR, 0, 32, 0x00);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRR, 1, 32, 0x00);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRR, 1, 32, 0x00);
#endif
#endif // #if defined(MEM_TYPE_LPDDR23)

	SetIO32(&pReg_DDRPHY->PHY_CON[2], (0x1 << 24)); // gate_cal_mode[24] = 1
	SetIO32(&pReg_DDRPHY->PHY_CON[0],
		(0x5 << 6)); // ctrl_shgate[8]=1, ctrl_atgate[6]=1
	ClearIO32(&pReg_DDRPHY->PHY_CON[1],
		  (0xF << 20)); // ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3:
				// 0xB, LPDDR2: 0x9
#if defined(MEM_TYPE_LPDDR23)
	SetIO32(&pReg_DDRPHY->PHY_CON[1],
		(0xB << 20)); // ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3:
			      // 0xB, LPDDR2: 0x9
#endif

	WriteIO32(&pReg_DDRPHY->PHY_CON[3],
		  (0x1 << 18)); // gate_lvl_start[18] = 1
	//    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  18) );
	//    // gate_lvl_start[18] = 1

	for (cal_count = 0; cal_count < 100; cal_count++) {
		temp = ReadIO32(&pReg_DDRPHY->PHY_CON[3]);
		if (temp & (0x1 << 26)) // rd_wr_cal_resp[26] : Wating until
					// GATE calibration is complete
		{
			break;
		}

		DMC_Delay(100);
	}

	//    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        0x0 );
	//    // gate_lvl_start[18]=0 : Stopping it after completion of GATE
	//    leveling.
	ClearIO32(&pReg_DDRPHY->PHY_CON[3],
		  (0x1 << 18)); // gate_lvl_start[18]=0 : Stopping it after
				// completion of GATE leveling.

	//------------------------------------------------------------------------------------------------------------------------

	if (cal_count == 100) {
		MEMMSG("GATE: Calibration Responese Checking : fail...!!!\r\n");

		ret = CFALSE; // Failure Case
		goto gate_err_ret;
	}

	//------------------------------------------------------------------------------------------------------------------------

	g_GT_code = ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[0]);
	g_GT_cycle = ReadIO32(&pReg_DDRPHY->CAL_GT_CYC);
//    g_GT_cycle  = ReadIO32( &pReg_DDRPHY->CAL_GT_CYC ) + 0x492;

//------------------------------------------------------------------------------------------------------------------------

#if 1
	{
		U32 i;

		//    MEMMSG("CAL_FAIL_STAT0  = 0x%08x\r\n",
		//    ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) );

		//    MEMMSG("GT VWMC0 = 0x%08X\r\n",
		//    ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[0]) );
		MEMMSG("GT VWMC0 = 0x%08X\r\n", g_GT_code);
		MEMMSG("GT VWMC1 = 0x%08X\r\n",
		       ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[1]));
		MEMMSG("GT VWMC2 = 0x%08X\r\n",
		       ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[2]));

		//    MEMMSG("GT CYC   = 0x%08X\r\n",
		//    ReadIO32(&pReg_DDRPHY->CAL_GT_CYC) );
		MEMMSG("GT CYC   = 0x%08X\r\n", g_GT_cycle);

		for (i = 0; i < 4; i++) {
			MEMINF("GT SLICE%d : %03d - %02d\r\n", i,
			       (g_GT_code >> (0x8 * i)) & 0xFF,
			       (g_GT_cycle >> (0x3 * i)) & 0x7);
		}
	}
#endif

//------------------------------------------------------------------------------------------------------------------------

gate_err_ret:

#if 0
    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x0 (LOW)
#if 0
    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
#endif
#endif

	WriteIO32(&pReg_DDRPHY->LP_CON, 0x0); // ctrl_pulld_dqs[8:0] = 0
	ClearIO32(&pReg_DDRPHY->RODT_CON, (0x1 << 16)); // ctrl_read_dis[16] = 0

#if defined(MEM_TYPE_DDR3)
	/* Set MPR mode disable */
	MR.Reg = 0;

	SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif // #if defined(MEM_TYPE_DDR3)

	MEMMSG("\r\n########## Gate Leveling - End ##########\r\n");

	//    if ( pSBI->FlyBy_Mode && (g_GT_code == 0x08080808) )
	if (g_GT_code == 0x08080808)
		ret = CFALSE;

	return ret;
}
#endif // #if (DDR_GATE_LEVELING_EN == 1)

#if (DDR_READ_DQ_CALIB_EN == 1)
CBOOL DDR_Read_DQ_Calibration(void)
{
#if defined(MEM_TYPE_DDR3)
	union SDRAM_MR MR;
#endif
	volatile U32 cal_count = 0;
	U32 temp;
	CBOOL ret = CTRUE;

	MEMMSG("\r\n########## Read DQ Calibration - Start ##########\r\n");

	SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif

#if defined(MEM_TYPE_DDR3)
	temp = ReadIO32(&pReg_DDRPHY->PHY_CON[1]) & 0xFFFF0000;
	temp |= 0xFF00; // rdlvl_rddata_adj[15:0]
	//    temp |= 0x0100; // rdlvl_rddata_adj[15:0]
	WriteIO32(&pReg_DDRPHY->PHY_CON[1], temp);

	/* Set MPR mode enable */
	MR.Reg = 0;
	MR.MR3.MPR = 1;

	SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif

	WriteIO32(&pReg_DDRPHY->LP_DDR_CON[2],
		  (0x0 << 16) |    // [31:16] ddr3_default
		      (0x0 << 1) | // [15: 1] ddr3_address
		      (0x0 << 0)); // [    0] ca_swap_mode
#endif				   // #if defined(MEM_TYPE_DDR3)
#if defined(MEM_TYPE_LPDDR23)
	temp = ReadIO32(&pReg_DDRPHY->PHY_CON[1]) & 0xFFFF0000;
	//    temp |= 0x00FF; // rdlvl_rddata_adj[15:0]
	temp |= 0x0001; // rdlvl_rddata_adj[15:0]
	WriteIO32(&pReg_DDRPHY->PHY_CON[1], temp);

	SendDirectCommand(SDRAM_CMD_MRR, 0, 32, 0x00);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRR, 1, 32, 0x00);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRR, 1, 32, 0x00);
#endif
#endif // #if defined(MEM_TYPE_LPDDR23)

	SetIO32(&pReg_DDRPHY->PHY_CON[2], (0x1 << 25)); // rd_cal_mode[25]=1

	WriteIO32(
	    &pReg_DDRPHY->PHY_CON[3],
	    (0x1 << 19)); // rd_cal_start[19]=1 : Starting READ calibration
	//    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  19) );
	//    // rd_cal_start[19]=1 : Starting READ calibration

	for (cal_count = 0; cal_count < 100; cal_count++) {
		temp = ReadIO32(&pReg_DDRPHY->PHY_CON[3]);
		if (temp & (0x1 << 26)) // rd_wr_cal_resp[26] : Wating until
					// READ calibration is complete
		{
			break;
		}

		DMC_Delay(100);
	}
	//    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  26) );

	//    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        0x0 );
	//    // rd_cal_start[19]=0 : Stopping it after completion of Read
	//    calibration.
	ClearIO32(&pReg_DDRPHY->PHY_CON[3], (0x1 << 19)); // rd_cal_start[19]=0
							  // : Stopping it after
							  // completion of Read
							  // calibration.

	//------------------------------------------------------------------------------------------------------------------------

	if (cal_count == 100) // Failure Case
	{
		MEMMSG("RD DQ CAL Status Checking error\r\n");

		ret = CFALSE;
		goto rd_err_ret;
	}

	for (cal_count = 0; cal_count < 100; cal_count++) {
		if (ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) == 0) {
			break;
		}

		DMC_Delay(100);
	}

	if (cal_count == 100) {
		MEMMSG("RD DQ: CAL_FAIL_STATUS Checking : fail...!!!\r\n");

		ret = CFALSE; // Failure Case
		goto rd_err_ret;
	}

	//------------------------------------------------------------------------------------------------------------------------

	g_RD_vwmc = ReadIO32(&pReg_DDRPHY->CAL_RD_VWMC[0]);

//------------------------------------------------------------------------------------------------------------------------

#if 0
{
    U32 vwml, vwmr;
    U32 i;

    MEMMSG("\r\n\n");
    for(i = 0; i < 8; i++)
        MEMMSG("RD%d RD_DESKEW_CON  = 0x%08x\r\n", i, ReadIO32(&pReg_DDRPHY->RD_DESKEW_CON[(i*3)]) );

    MEMMSG("\r\n");

    vwml = ReadIO32(&pReg_DDRPHY->CAL_RD_VWML[0]);
    vwmr = ReadIO32(&pReg_DDRPHY->CAL_RD_VWMR[0]);

    for(i = 0; i < 4; i++) {
        MEMINF("RD DQ SLICE%d : %03d - %03d - %03d\r\n",
            i, (vwml >> (0x8 * i)) & 0xFF, (g_RD_vwmc >> (0x8 * i)) & 0xFF, (vwmr >> (0x8 * i)) & 0xFF );
    }
    MEMMSG("\r\n");

    MEMMSG("RD DQ CAL_RD_VWMC0  = 0x%08x\r\n", g_RD_vwmc );
    MEMMSG("RD DQ CAL_RD_VWML0  = 0x%08x\r\n", vwml );
    MEMMSG("RD DQ CAL_RD_VWMR0  = 0x%08x\r\n", vwmr );

    MEMMSG("\r\n");
    MEMMSG("DQ_IO_RDATA0    = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->DQ_IO_RDATA[0]) );

    MEMMSG("\r\n");
    MEMMSG("CAL_FAIL_STAT0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) );
    MEMMSG("CAL_FAIL_STAT3  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[3]) );

    MEMMSG("\r\n");
    MEMMSG("CAL_DM_VWMC0    = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_DM_VWMC[0]) );
    MEMMSG("CAL_DM_VWML0    = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_DM_VWML[0]) );
    MEMMSG("CAL_DM_VWMR0    = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_DM_VWMR[0]) );

    MEMMSG("\r\n");
    for(i = 0; i < 8; i++)
        MEMMSG("D%d VWMC_STAT   = 0x%08X\r\n", i, ReadIO32(&pReg_DDRPHY->VWMC_STAT[(i*3)]) );

    MEMMSG("\r\n");
    for(i = 0; i < 8; i++)
        MEMMSG("D%d VWML_STAT   = 0x%08X\r\n", i, ReadIO32(&pReg_DDRPHY->VWML_STAT[(i*3)]) );

    MEMMSG("\r\n");
    for(i = 0; i < 8; i++)
        MEMMSG("D%d VWMR_STAT   = 0x%08X\r\n", i, ReadIO32(&pReg_DDRPHY->VWMR_STAT[(i*3)]) );
}
#endif

//------------------------------------------------------------------------------------------------------------------------

rd_err_ret:

#if 0
    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x0 (LOW)
#if 0
    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
#endif
#endif

#if defined(MEM_TYPE_DDR3)
	/* Set MPR mode disable */
	MR.Reg = 0;

	SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif

	MEMMSG("\r\n########## Read DQ Calibration - End ##########\r\n");

	return ret;
}
#endif // #if (DDR_READ_DQ_CALIB_EN == 1)

#if (DDR_WRITE_LEVELING_CALIB_EN == 1)
void DDR_Write_Leveling_Calibration(void)
{
	MEMMSG(
	    "\r\n########## Write Leveling Calibration - Start ##########\r\n");
}
#endif

#if (DDR_WRITE_DQ_CALIB_EN == 1)
CBOOL DDR_Write_DQ_Calibration(void)
{
	volatile U32 cal_count = 0;
	U32 temp;
	CBOOL ret = CTRUE;

	MEMMSG("\r\n########## Write DQ Calibration - Start ##########\r\n");

	// Set issue active command.
	WriteIO32(&pReg_Drex->WRTRA_CONFIG,
		  (0x0 << 16) |    // [31:16] row_addr
		      (0x0 << 1) | // [ 3: 1] bank_addr
		      (0x1 << 0)); // [    0] write_training_en

	SetIO32(&pReg_DDRPHY->PHY_CON[2],
		(0x1 << 26)); // wr_cal_mode[26] = 1, Write Training mode
	SetIO32(&pReg_DDRPHY->PHY_CON[2], (0x1 << 27)); // wr_cal_start[27] = 1

	for (cal_count = 0; cal_count < 100; cal_count++) {
		temp = ReadIO32(&pReg_DDRPHY->PHY_CON[3]);
		if (temp & (0x1 << 26)) // rd_wr_cal_resp[26] : Wating until
					// WRITE calibration is complete
		{
			break;
		}

		DMC_Delay(0x100);
	}

	ClearIO32(&pReg_DDRPHY->PHY_CON[2],
		  (0x1 << 27)); // wr_cal_start[27] = 0
	//    ClearIO32( &pReg_DDRPHY->PHY_CON[2],        (0x3    <<  26) );
	//    // wr_cal_start[27] = 0, wr_cal_mode[26] = 0

	ClearIO32(&pReg_Drex->WRTRA_CONFIG,
		  (0x1 << 0)); // write_training_en[0] = 0

	//------------------------------------------------------------------------------------------------------------------------

	if (cal_count == 100) // Failure Case
	{
		MEMMSG("WR DQ CAL Status Checking error\r\n");

		ret = CFALSE;
		goto wr_err_ret;
	}

	for (cal_count = 0; cal_count < 100; cal_count++) {
		if ((ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) |
		     ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[3])) == 0) {
			break;
		}

		DMC_Delay(100);
	}

	if (cal_count == 100) {
		MEMMSG("WR DQ: CAL_FAIL_STATUS Checking : fail...!!!\r\n");

		ret = CFALSE; // Failure Case
		goto wr_err_ret;
	}

	//------------------------------------------------------------------------------------------------------------------------

	g_WR_vwmc = ReadIO32(&pReg_DDRPHY->CAL_WR_VWMC[0]);

//------------------------------------------------------------------------------------------------------------------------
#if 0
{
    U32 vwml, vwmr;
    U32 i;

    MEMMSG("\r\n\n");
    for(i = 0; i < 8; i++)
        MEMMSG("WR%d WR_DESKEW_CON  = 0x%08x\r\n", i, ReadIO32(&pReg_DDRPHY->WR_DESKEW_CON[(i*3)]) );

    MEMMSG("\r\n");
    for(i = 0; i < 3; i++)
        MEMMSG("DM%d WR_DESKEW_CON  = 0x%08x\r\n", i, ReadIO32(&pReg_DDRPHY->DM_DESKEW_CON[i]) );

    MEMMSG("\r\n");
    MEMMSG("DQ_IO_RDATA0    = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->DQ_IO_RDATA[0]) );

    MEMMSG("\r\n");
    MEMMSG("CAL_FAIL_STAT0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) );
    MEMMSG("CAL_FAIL_STAT3  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[3]) );
    MEMMSG("\r\n");

    vwml = ReadIO32(&pReg_DDRPHY->CAL_WR_VWML[0]);
    vwmr = ReadIO32(&pReg_DDRPHY->CAL_WR_VWMR[0]);

    for(i = 0; i < 4; i++) {
        MEMINF("WR DQ SLICE%d : %03d - %03d - %03d\r\n",
            i, (vwml >> (0x8 * i)) & 0xFF, (g_WR_vwmc >> (0x8 * i)) & 0xFF, (vwmr >> (0x8 * i)) & 0xFF );
    }
    MEMMSG("\r\n");

    MEMMSG("WR CAL_DM_VWMC0 = 0x%08x\r\n", g_WR_vwmc );
    MEMMSG("WR CAL_DM_VWML0 = 0x%08x\r\n", vwml );
    MEMMSG("WR CAL_DM_VWMR0 = 0x%08x\r\n", vwmr );

    MEMMSG("\r\n");
    MEMMSG("WR DM_VWMC_STAT0    = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->DM_VWMC_STAT[0]) );
    MEMMSG("WR DM_VWML_STAT0    = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->DM_VWML_STAT[0]) );
    MEMMSG("WR DM_VWMR_STAT0    = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->DM_VWMR_STAT[0]) );

    MEMMSG("\r\n");
    MEMMSG("WR DQ CAL_WR_VWMC0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_WR_VWMC[0]) );
    MEMMSG("WR DQ CAL_WR_VWML0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_WR_VWML[0]) );
    MEMMSG("WR DQ CAL_WR_VWMR0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_WR_VWMR[0]) );

    MEMMSG("\r\n");
    for(i = 0; i < 8; i++)
        MEMMSG("D%d VWMC_STAT   = 0x%08X\r\n", i, ReadIO32(&pReg_DDRPHY->VWMC_STAT[(i*3)]) );

    MEMMSG("\r\n");
    for(i = 0; i < 8; i++)
        MEMMSG("D%d VWML_STAT   = 0x%08X\r\n", i, ReadIO32(&pReg_DDRPHY->VWML_STAT[(i*3)]) );

    MEMMSG("\r\n");
    for(i = 0; i < 8; i++)
        MEMMSG("D%d VWMR_STAT   = 0x%08X\r\n", i, ReadIO32(&pReg_DDRPHY->VWMR_STAT[(i*3)]) );
}
#endif
//------------------------------------------------------------------------------------------------------------------------

wr_err_ret:

#if 0
    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x0 (LOW)
#if 0
    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
#endif
#endif

	MEMMSG("\r\n########## Write DQ Calibration - End ##########\r\n");

	return ret;
}
#endif // #if (DDR_WRITE_DQ_CALIB_EN == 1)

U32 getVWMC_Offset(U32 code, U32 lock_div4)
{
	U32 i, ret_val;
	U8 vwmc[4];
	int offset[4];

	for (i = 0; i < 4; i++) {
		vwmc[i] = ((code >> (8 * i)) & 0xFF);
	}

	for (i = 0; i < 4; i++) {
		offset[i] = (int)(vwmc[i] - lock_div4);
		if (offset[i] < 0) {
			offset[i] *= -1;
			offset[i] |= 0x80;
		}
	}

	ret_val = (((U8)offset[3] << 24) | ((U8)offset[2] << 16) |
		   ((U8)offset[1] << 8) | (U8)offset[0]);

	return ret_val;
}

CBOOL init_LPDDR3(U32 isResume)
{
	union SDRAM_MR MR1, MR2, MR3, MR11;
	U32 DDR_WL, DDR_RL;
	U32 i;
	U32 temp;

// Set 200Mhz to PLL3.
#if (CFG_DDR_LOW_FREQ == 1)
#if defined(MEM_TYPE_LPDDR23)
	while (!DebugIsUartTxDone())
		;
	setMemPLL(0);
#endif
#endif

	MR1.Reg = 0;
	MR2.Reg = 0;
	MR3.Reg = 0;
	MR11.Reg = 0;

	MEMMSG("\r\nLPDDR3 POR Init Start\r\n");

	// Step 1. reset (Min : 10ns, Typ : 200us)
	ClearIO32(&pReg_RstCon->REGRST[0], (0x7 << 26));
	DMC_Delay(0x1000); // wait 300ms
	SetIO32(&pReg_RstCon->REGRST[0], (0x7 << 26));
	DMC_Delay(0x1000); // wait 300ms
	ClearIO32(&pReg_RstCon->REGRST[0], (0x7 << 26));
	DMC_Delay(0x1000); // wait 300ms
	SetIO32(&pReg_RstCon->REGRST[0], (0x7 << 26));
//    DMC_Delay(0x10000);                                        // wait 300ms

#if 0
    ClearIO32( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
    DMC_Delay(0x1000);                                          // wait 300ms
    SetIO32  ( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
    DMC_Delay(0x1000);                                          // wait 300ms
    ClearIO32( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
    DMC_Delay(0x1000);                                          // wait 300ms
    SetIO32  ( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
#endif
	DMC_Delay(0x10000); // wait 300ms

	while (ReadIO32(&pReg_DDRPHY->SHIFTC_CON) != 0x0492) {
		DMC_Delay(1000);
	}
	MEMMSG("PHY Version: 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->VERSION_INFO));

#if 0
    if (!g_GateCycle || !g_RDvwmc || !g_WRvwmc)
        isResume = 0;
#endif

#if 0
#if (CFG_NSIH_EN == 0)
    MEMMSG("READDELAY   = 0x%08X\r\n", READDELAY);
    MEMMSG("WRITEDELAY  = 0x%08X\r\n", WRITEDELAY);
#else
    MEMMSG("READDELAY   = 0x%08X\r\n", pSBI->DII.READDELAY);
    MEMMSG("WRITEDELAY  = 0x%08X\r\n", pSBI->DII.WRITEDELAY);
#endif
#endif

#if (CFG_NSIH_EN == 0)
	// pSBI->LvlTr_Mode    = ( LVLTR_WR_LVL | LVLTR_CA_CAL | LVLTR_GT_LVL |
	// LVLTR_RD_CAL | LVLTR_WR_CAL );
	// pSBI->LvlTr_Mode    = ( LVLTR_GT_LVL | LVLTR_RD_CAL | LVLTR_WR_CAL );
	pSBI->LvlTr_Mode = LVLTR_GT_LVL;
// pSBI->LvlTr_Mode    = 0;
#endif

#if (CFG_NSIH_EN == 0)
#if 1   // Common
	//    pSBI->DDR3_DSInfo.MR2_RTT_WR    = 2;    // RTT_WR - 0: ODT
	//    disable, 1: RZQ/4, 2: RZQ/2
	//    pSBI->DDR3_DSInfo.MR1_ODS       = 1;    // ODS - 00: RZQ/6, 01 :
	//    RZQ/7
	//    pSBI->DDR3_DSInfo.MR1_RTT_Nom   = 2;    // RTT_Nom - 001: RZQ/4,
	//    010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8

	pSBI->PHY_DSInfo.DRVDS_Byte3 = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_Byte2 = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_Byte1 = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_Byte0 = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_CK = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_CKE = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_CS = PHY_DRV_STRENGTH_240OHM;
	pSBI->PHY_DSInfo.DRVDS_CA = PHY_DRV_STRENGTH_240OHM;

	pSBI->PHY_DSInfo.ZQ_DDS = PHY_DRV_STRENGTH_48OHM;
	pSBI->PHY_DSInfo.ZQ_ODT = PHY_DRV_STRENGTH_120OHM;
#endif

#if 0 // DroneL 720Mhz
    pSBI->DDR3_DSInfo.MR2_RTT_WR    = 1;    // RTT_WR - 0: ODT disable, 1: RZQ/4, 2: RZQ/2
    pSBI->DDR3_DSInfo.MR1_ODS       = 1;    // ODS - 00: RZQ/6, 01 : RZQ/7
    pSBI->DDR3_DSInfo.MR1_RTT_Nom   = 3;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8

    pSBI->PHY_DSInfo.DRVDS_Byte3    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte2    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte1    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte0    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_CK       = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_CKE      = PHY_DRV_STRENGTH_30OHM;
    pSBI->PHY_DSInfo.DRVDS_CS       = PHY_DRV_STRENGTH_30OHM;
    pSBI->PHY_DSInfo.DRVDS_CA       = PHY_DRV_STRENGTH_30OHM;

    pSBI->PHY_DSInfo.ZQ_DDS         = PHY_DRV_STRENGTH_40OHM;
//    pSBI->PHY_DSInfo.ZQ_ODT         = PHY_DRV_STRENGTH_80OHM;
    pSBI->PHY_DSInfo.ZQ_ODT         = PHY_DRV_STRENGTH_60OHM;
#endif

#if 0 // DroneL 800Mhz
    pSBI->DDR3_DSInfo.MR2_RTT_WR    = 2;    // RTT_WR - 0: ODT disable, 1: RZQ/4, 2: RZQ/2
    pSBI->DDR3_DSInfo.MR1_ODS       = 1;    // ODS - 00: RZQ/6, 01 : RZQ/7
    pSBI->DDR3_DSInfo.MR1_RTT_Nom   = 3;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8

    pSBI->PHY_DSInfo.DRVDS_Byte3    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte2    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte1    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte0    = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_CK       = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.DRVDS_CKE      = PHY_DRV_STRENGTH_30OHM;
    pSBI->PHY_DSInfo.DRVDS_CS       = PHY_DRV_STRENGTH_30OHM;
    pSBI->PHY_DSInfo.DRVDS_CA       = PHY_DRV_STRENGTH_30OHM;

    pSBI->PHY_DSInfo.ZQ_DDS         = PHY_DRV_STRENGTH_40OHM;
    pSBI->PHY_DSInfo.ZQ_ODT         = PHY_DRV_STRENGTH_120OHM;
#endif
#endif

	MR1.LP_MR1.BL = 3;

	MR2.LP_MR2.WL_SEL = 0;
	MR2.LP_MR2.WR_LVL = 0;

#if (CFG_NSIH_EN == 0)
	DDR_WL = nWL;
	DDR_RL = nRL;

	if (MR1_nWR > 9) {
		MR1.LP_MR1.WR = (MR1_nWR - 10) & 0x7;
		MR2.LP_MR2.WRE = 1;
	} else {
		MR1.LP_MR1.WR = (MR1_nWR - 2) & 0x7;
		MR2.LP_MR2.WRE = 0;
	}

	if (MR2_RLWL < 6) {
		MR2.LP_MR2.RL_WL = 4;
	} else {
		MR2.LP_MR2.RL_WL = (MR2_RLWL - 2);
	}

	MR3.LP_MR3.DS = 2;

	MR11.LP_MR11.PD_CON = 0;
	MR11.LP_MR11.DQ_ODT =
	    2; // DQ ODT - 0: Disable, 1: Rzq/4, 2: Rzq/2, 3: Rzq/1
#else	  // #if (CFG_NSIH_EN == 0)

	DDR_WL = (pSBI->DII.TIMINGDATA >> 8) & 0xF;
	DDR_RL = (pSBI->DII.TIMINGDATA & 0xF);

	if (pSBI->DII.MR0_WR > 9) {
		MR1.LP_MR1.WR = (pSBI->DII.MR0_WR - 10) & 0x7;
		MR2.LP_MR2.WRE = 1;
	} else {
		MR1.LP_MR1.WR = (pSBI->DII.MR0_WR - 2) & 0x7;
		MR2.LP_MR2.WRE = 0;
	}

	if (pSBI->DII.MR1_AL < 6) {
		MR2.LP_MR2.RL_WL = 4;
	} else {
		MR2.LP_MR2.RL_WL = (pSBI->DII.MR1_AL - 2);
	}

	MR3.LP_MR3.DS = pSBI->LPDDR3_DSInfo.MR3_DS;

	MR11.LP_MR11.DQ_ODT = pSBI->LPDDR3_DSInfo.MR11_DQ_ODT;
	MR11.LP_MR11.PD_CON = pSBI->LPDDR3_DSInfo.MR11_PD_CON;
#endif	 // #if (CFG_NSIH_EN == 0)

	// Step 2. Select Memory type : LPDDR3
	// Check LPDDR3 MPR data and match it to PHY_CON[1]??

	temp = ((0x17 << 24) | // [28:24] T_WrWrCmd
		(0x1 << 22) |  // [23:22] ctrl_upd_mode. DLL Update control
			       // 0:always, 1: depending on ctrl_flock, 2:
			       // depending on ctrl_clock, 3: don't update
		(0x0 << 20) |  // [21:20] ctrl_upd_range
#if (CFG_NSIH_EN == 0)
#if (tWTR == 3)		      // 6 cycles
		(0x7 << 17) | // [19:17] T_WrRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#elif(tWTR == 2)	      // 4 cycles
		(0x6 << 17) |			  // [19:17] T_WrRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#endif
#endif
		(0x0 << 16) | // [   16] wrlvl_mode. Write Leveling Enable.
			      // 0:Disable, 1:Enable
		(0x0 << 14) | // [   14] p0_cmd_en. 0:Issue Phase1 Read command
			      // during Read Leveling. 1:Issue Phase0
		(0x0 << 13) | // [   13] byte_rdlvl_en. Read Leveling 0:Disable,
			      // 1:Enable
#if defined(MEM_TYPE_DDR3)
		(0x1 << 11) | // [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3,
			      // 2:LPDDR2, 3:LPDDR3
#endif
#if defined(MEM_TYPE_LPDDR23)
		(0x3 << 11) | // [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3,
			      // 2:LPDDR2, 3:LPDDR3
#endif
		(0x1 << 10) | // [   10] ctrl_wr_dis. Write ODT Disable Signal
			      // during Write Calibration. 0: not change, 1:
			      // disable
		(0x1 << 9) |  // [    9] ctrl_dfdqs. 0:Single-ended DQS,
			      // 1:Differential DQS
		//        (0x1    <<   8) |           // [    8] ctrl_shgate.
		//        0:Gate signal length=burst length/2+N, 1:Gate signal
		//        length=burst length/2-1
		(0x1 << 6) | // [    6] ctrl_atgate
		(0x0 << 4) | // [    4] ctrl_cmosrcv
		(0x0 << 3) | // [    3] ctrl_twpre
		(0x0 << 0)); // [ 2: 0] ctrl_fnc_fb. 000:Normal operation.

#if (CFG_NSIH_EN == 1)
	if ((pSBI->DII.TIMINGDATA >> 28) == 3) // 6 cycles
		temp |= (0x7 << 17);
	else if ((pSBI->DII.TIMINGDATA >> 28) == 2) // 4 cycles
		temp |= (0x6 << 17);
#endif

	WriteIO32(&pReg_DDRPHY->PHY_CON[0], temp);

#if defined(MEM_TYPE_LPDDR23)
	temp = ReadIO32(&pReg_DDRPHY->LP_DDR_CON[3]) & ~0x3FFF;
	temp |= 0x000E; // cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
	WriteIO32(&pReg_DDRPHY->LP_DDR_CON[3], temp);

	temp = ReadIO32(&pReg_DDRPHY->LP_DDR_CON[4]) & ~0x3FFF;
	temp |= 0x000F; // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
	WriteIO32(&pReg_DDRPHY->LP_DDR_CON[4], temp);
#endif // #if defined(MEM_TYPE_LPDDR23)

	MEMMSG("phy init\r\n");

	/* Set WL, RL, BL */
	WriteIO32(
	    &pReg_DDRPHY->PHY_CON[4],
	    (DDR_WL << 16) | // [20:16] T_wrdata_en (WL+1)
		(0x8 << 8) | // [12: 8] Burst Length(BL)
		((DDR_RL + 1)
		 << 0)); // [ 4: 0] Read Latency(RL), 800MHz:0xB, 533MHz:0x5

/* ZQ Calibration */
#if 0
    WriteIO32( &pReg_DDRPHY->DRVDS_CON[0],      // 100: 48ohm, 101: 40ohm, 110: 34ohm, 111: 30ohm
        (PHY_DRV_STRENGTH_30OHM <<  28) |       // [30:28] Data Slice 4
        (pSBI->PHY_DSInfo.DRVDS_Byte3 <<  25) | // [27:25] Data Slice 3
        (pSBI->PHY_DSInfo.DRVDS_Byte2 <<  22) | // [24:22] Data Slice 2
        (pSBI->PHY_DSInfo.DRVDS_Byte1 <<  19) | // [21:19] Data Slice 1
        (pSBI->PHY_DSInfo.DRVDS_Byte0 <<  16) | // [18:16] Data Slice 0
        (pSBI->PHY_DSInfo.DRVDS_CK    <<   9) | // [11: 9] CK
        (pSBI->PHY_DSInfo.DRVDS_CKE   <<   6) | // [ 8: 6] CKE
        (pSBI->PHY_DSInfo.DRVDS_CS    <<   3) | // [ 5: 3] CS
        (pSBI->PHY_DSInfo.DRVDS_CA    <<   0)); // [ 2: 0] CA[9:0], RAS, CAS, WEN, ODT[1:0], RESET, BANK[2:0]

    WriteIO32( &pReg_DDRPHY->DRVDS_CON[1],      // 100: 48ohm, 101: 40ohm, 110: 34ohm, 111: 30ohm
        (PHY_DRV_STRENGTH_30OHM <<  25) |       // [11: 9] Data Slice 8
        (PHY_DRV_STRENGTH_30OHM <<  22) |       // [ 8: 6] Data Slice 7
        (PHY_DRV_STRENGTH_30OHM <<  19) |       // [ 5: 3] Data Slice 6
        (PHY_DRV_STRENGTH_30OHM <<  16));       // [ 2: 0] Data Slice 5
#else

	WriteIO32(&pReg_DDRPHY->DRVDS_CON[0], 0x00);
	WriteIO32(&pReg_DDRPHY->DRVDS_CON[1], 0x00);
#endif

	// Driver Strength(zq_mode_dds), zq_clk_div_en[18]=Enable
	WriteIO32(
	    &pReg_DDRPHY->ZQ_CON,
	    (0x1 << 27) | // [   27] zq_clk_en. ZQ I/O clock enable.
		(pSBI->PHY_DSInfo.ZQ_DDS
		 << 24) | // [26:24] zq_mode_dds, Driver strength selection. 100
			  // : 48ohm, 101 : 40ohm, 110 : 34ohm, 111 : 30ohm
		(pSBI->PHY_DSInfo.ZQ_ODT << 21) | // [23:21] ODT resistor value.
						  // 001 : 120ohm, 010 : 60ohm,
						  // 011 : 40ohm, 100 : 30ohm
		(0x0
		 << 20) | // [   20] zq_rgddr3. GDDR3 mode. 0:Enable, 1:Disable
#if defined(MEM_TYPE_DDR3)
		(0x0 << 19) | // [   19] zq_mode_noterm. Termination. 0:Enable,
			      // 1:Disable
#endif
#if defined(MEM_TYPE_LPDDR23)
		(0x1 << 19) | // [   19] zq_mode_noterm. Termination. 0:Enable,
			      // 1:Disable
#endif
		(0x1 << 18) | // [   18] zq_clk_div_en. Clock Dividing Enable :
			      // 0, Disable : 1
		(0x0 << 15) | // [17:15] zq_force-impn
		//        (0x7    <<  12) |                       // [14:12]
		//        zq_force-impp
		(0x0 << 12) | // [14:12] zq_force-impp
		(0x30 << 4) | // [11: 4] zq_udt_dly
		(0x1 << 2) |  // [ 3: 2] zq_manual_mode. 0:Force Calibration,
			      // 1:Long cali, 2:Short cali
		(0x0 << 1) | // [    1] zq_manual_str. Manual Calibration Stop :
			     // 0, Start : 1
		(0x0 << 0)); // [    0] zq_auto_en. Auto Calibration enable

	SetIO32(&pReg_DDRPHY->ZQ_CON,
		(0x1 << 1)); // zq_manual_str[1]. Manual Calibration Start=1
	while ((ReadIO32(&pReg_DDRPHY->ZQ_STATUS) & 0x1) == 0)
		; //- PHY0: wait for zq_done
	ClearIO32(
	    &pReg_DDRPHY->ZQ_CON,
	    (0x1
	     << 1)); // zq_manual_str[1]. Manual Calibration Stop : 0, Start : 1

	ClearIO32(&pReg_DDRPHY->ZQ_CON, (0x1 << 18)); // zq_clk_div_en[18].
						      // Clock Dividing Enable :
						      // 1, Disable : 0

	// Step 5. dfi_init_start : High
	WriteIO32(&pReg_Drex->CONCONTROL,
		  (0xFFF << 16) |   // [27:16] time out level0
		      (0x3 << 12) | // [14:12] read data fetch cycles - n cclk
				    // cycles (cclk: DREX core clock)
		      //        (0x1    <<   5) |   // [  : 5] afre_en. Auto
		      //        Refresh Counter. Disable:0, Enable:1
		      //        (0x1    <<   4) |   // nexell: 0:ca swap bit, 4
		      //        & samsung drex/phy initiated bit
		      (0x0 << 1) // [ 2: 1] aclk:cclk = 1:1
		  );

#if 0
    SetIO32  ( &pReg_Drex->CONCONTROL,      (0x1    <<  28) );          // dfi_init_start[28]. DFI PHY initialization start
    while( (ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1<<3) ) == 0);       // dfi_init_complete[3]. wait for DFI PHY initialization complete
    ClearIO32( &pReg_Drex->CONCONTROL,      (0x1    <<  28) );          // dfi_init_start[28]. DFI PHY initialization clear
#endif

	// Step 3. Set the PHY for dqs pull down mode
	WriteIO32(&pReg_DDRPHY->LP_CON,
		  (0x0 << 16) |    // [24:16] ctrl_pulld_dq
		      (0xF << 0)); // [ 8: 0] ctrl_pulld_dqs.  No Gate leveling
				   // : 0xF, Use Gate leveling : 0x0(X)

	WriteIO32(&pReg_DDRPHY->RODT_CON,
		  (0x0 << 28) |     // [31:28] ctrl_readduradj
		      (0x1 << 24) | // [27:24] ctrl_readadj
		      (0x1 << 16) | // [  :16] ctrl_read_dis
		      (0x0 << 0));  // [  : 0] ctrl_read_width

	// Step 8 : Update DLL information
	SetIO32(&pReg_Drex->PHYCONTROL,
		(0x1 << 3)); // Force DLL Resyncronization
	ClearIO32(&pReg_Drex->PHYCONTROL,
		  (0x1 << 3)); // Force DLL Resyncronization

	// Step 11. MemBaseConfig
	WriteIO32(&pReg_DrexTZ->MEMBASECONFIG[0],
		  (0x040 << 16) | // chip_base[26:16]. AXI Base Address. if 0x20
				  // ==> AXI base addr of memory : 0x2000_0000
#if (CFG_NSIH_EN == 0)
		      (chip_mask << 0)); // 256MB:0x7F0, 512MB: 0x7E0,
					 // 1GB:0x7C0, 2GB: 0x780, 4GB:0x700
#else
		      (pSBI->DII.ChipMask << 0));
#endif

#if (CFG_NSIH_EN == 0)
	WriteIO32(&pReg_DrexTZ->MEMBASECONFIG[1],
		  (chip_base1 << 16) | // chip_base[26:16]. AXI Base Address. if
				       // 0x40 ==> AXI base addr of memory :
				       // 0x4000_0000, 16MB unit
		      (chip_mask << 0)); // chip_mask[10:0]. 2048 - chip size
#else
	temp = (0x40 + pSBI->DII.ChipSize);
	WriteIO32(
	    &pReg_DrexTZ->MEMBASECONFIG[1],
	    (temp << 16) | // chip_base[26:16]. AXI Base Address. if 0x40 ==>
			   // AXI base addr of memory : 0x4000_0000, 16MB unit
		(pSBI->DII.ChipMask << 0)); // chip_mask[10:0]. 2048 - chip size
#endif

	// Step 12. MemConfig
	WriteIO32(
	    &pReg_DrexTZ->MEMCONFIG[0],
	    (0x0 << 20) | // bank lsb, LSB of Bank Bit Position in Complex
			  // Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12,
			  // 5:13
		(0x0 << 19) | // rank inter en, Rank Interleaved Address Mapping
		(0x0 << 18) | // bit sel en, Enable Bit Selection for Randomized
			      // interleaved Address Mapping
		(0x0 << 16) | // bit sel, Bit Selection for Randomized
			      // Interleaved Address Mapping
		(0x2 << 12) | // [15:12] chip_map. Address Mapping Method (AXI
			      // to Memory). 0:Linear(Bank, Row, Column, Width),
			      // 1:Interleaved(Row, bank, column, width), other
			      // : reserved
#if (CFG_NSIH_EN == 0)
		(chip_col << 8) | // [11: 8] chip_col. Number of Column Address
				  // Bit. others:Reserved, 2:9bit, 3:10bit,
		(chip_row << 4) | // [ 7: 4] chip_row. Number of  Row Address
				  // Bit. others:Reserved, 0:12bit, 1:13bit,
				  // 2:14bit, 3:15bit, 4:16bit
#else
		(pSBI->DII.ChipCol << 8) | // [11: 8] chip_col. Number of Column
					   // Address Bit. others:Reserved,
					   // 2:9bit, 3:10bit,
		(pSBI->DII.ChipRow << 4) | // [ 7: 4] chip_row. Number of  Row
					   // Address Bit. others:Reserved,
					   // 0:12bit, 1:13bit, 2:14bit,
					   // 3:15bit, 4:16bit
#endif
		(0x3 << 0)); // [ 3: 0] chip_bank. Number of  Bank Address Bit.
			     // others:Reserved, 2:4bank, 3:8banks

#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	WriteIO32(
	    &pReg_DrexTZ->MEMCONFIG[1],
	    (0x0 << 20) | // bank lsb, LSB of Bank Bit Position in Complex
			  // Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12,
			  // 5:13
		(0x0 << 19) | // rank inter en, Rank Interleaved Address Mapping
		(0x0 << 18) | // bit sel en, Enable Bit Selection for Randomized
			      // interleaved Address Mapping
		(0x0 << 16) | // bit sel, Bit Selection for Randomized
			      // Interleaved Address Mapping
		(0x2 << 12) | // [15:12] chip_map. Address Mapping Method (AXI
			      // to Memory). 0 : Linear(Bank, Row, Column,
			      // Width), 1 : Interleaved(Row, bank, column,
			      // width), other : reserved
		(chip_col << 8) | // [11: 8] chip_col. Number of Column Address
				  // Bit. others:Reserved, 2:9bit, 3:10bit,
		(chip_row << 4) | // [ 7: 4] chip_row. Number of  Row Address
				  // Bit. others:Reserved, 0:12bit, 1:13bit,
				  // 2:14bit, 3:15bit, 4:16bit
		(0x3 << 0)); // [ 3: 0] chip_bank. Number of  Row Address Bit.
			     // others:Reserved, 2:4bank, 3:8banks
#endif
#else
	if (pSBI->DII.ChipNum > 1) {
		WriteIO32(
		    &pReg_DrexTZ->MEMCONFIG[1],
		    (0x0 << 20) |     // bank lsb, LSB of Bank Bit Position in
				      // Complex Interleaved Mapping 0:8, 1: 9,
				      // 2:10, 3:11, 4:12, 5:13
			(0x0 << 19) | // rank inter en, Rank Interleaved Address
				      // Mapping
			(0x0 << 18) | // bit sel en, Enable Bit Selection for
				      // Randomized interleaved Address Mapping
			(0x0 << 16) | // bit sel, Bit Selection for Randomized
				      // Interleaved Address Mapping
			(0x2 << 12) | // [15:12] chip_map. Address Mapping
				      // Method (AXI to Memory). 0 :
				      // Linear(Bank, Row, Column, Width), 1 :
				      // Interleaved(Row, bank, column, width),
				      // other : reserved
			(pSBI->DII.ChipCol
			 << 8) | // [11: 8] chip_col. Number of Column Address
				 // Bit. others:Reserved, 2:9bit, 3:10bit,
			(pSBI->DII.ChipRow
			 << 4) |     // [ 7: 4] chip_row. Number of  Row Address
				     // Bit. others:Reserved, 0:12bit, 1:13bit,
				     // 2:14bit, 3:15bit, 4:16bit
			(0x3 << 0)); // [ 3: 0] chip_bank. Number of  Row
				     // Address Bit. others:Reserved, 2:4bank,
				     // 3:8banks
	}
#endif

// Step 13. Precharge Configuration
#if 0
    WriteIO32( &pReg_Drex->PRECHCONFIG0,
        (0xF    <<  28) |           // Timeout Precharge per Port
        (0x0    <<  16));           // open page policy
    WriteIO32( &pReg_Drex->PRECHCONFIG1,    0xFFFFFFFF );           //- precharge cycle
    WriteIO32( &pReg_Drex->PWRDNCONFIG,     0xFFFF00FF );           //- low power counter
#endif
	WriteIO32(&pReg_Drex->PRECHCONFIG1, 0x00); //- precharge cycle
	WriteIO32(&pReg_Drex->PWRDNCONFIG, 0xFF);  //- low power counter

// Step 14.  AC Timing
#if (CFG_NSIH_EN == 0)
	WriteIO32(&pReg_Drex->TIMINGAREF,
		  (tREFIPB << 16) |  //- rclk (MPCLK)
		      (tREFI << 0)); //- refresh counter, 800MHz : 0x618

	WriteIO32(&pReg_Drex->ACTIMING0.TIMINGROW,
		  (tRFC << 24) | (tRRD << 20) | (tRP << 16) | (tRCD << 12) |
		      (tRC << 6) | (tRAS << 0));

	WriteIO32(&pReg_Drex->ACTIMING0.TIMINGDATA,
		  (tWTR << 28) | (tWR << 24) | (tRTP << 20) | (tPPD << 17) |
		      (W2W_C2C << 14) | (R2R_C2C << 12) | (nWL << 8) |
		      (tDQSCK << 4) | (nRL << 0));

	WriteIO32(&pReg_Drex->ACTIMING0.TIMINGPOWER,
		  (tFAW << 26) | (tXSR << 16) | (tXP << 8) | (tCKE << 4) |
		      (tMRD << 0));

#if (_DDR_CS_NUM > 1)
	WriteIO32(&pReg_Drex->ACTIMING1.TIMINGROW,
		  (tRFC << 24) | (tRRD << 20) | (tRP << 16) | (tRCD << 12) |
		      (tRC << 6) | (tRAS << 0));

	WriteIO32(&pReg_Drex->ACTIMING1.TIMINGDATA,
		  (tWTR << 28) | (tWR << 24) | (tRTP << 20) |
		      (W2W_C2C << 14) |		   // W2W_C2C
		      (R2R_C2C << 12) |		   // R2R_C2C
		      (nWL << 8) | (tDQSCK << 4) | // tDQSCK
		      (nRL << 0));

	WriteIO32(&pReg_Drex->ACTIMING1.TIMINGPOWER,
		  (tFAW << 26) | (tXSR << 16) | (tXP << 8) | (tCKE << 4) |
		      (tMRD << 0));
#endif

	//    WriteIO32( &pReg_Drex->TIMINGPZQ,   0x00004084 );     //- average
	//    periodic ZQ interval. Max:0x4084
	WriteIO32(&pReg_Drex->TIMINGPZQ,
		  tPZQ); //- average periodic ZQ interval. Max:0x4084

	WriteIO32(&pReg_Drex->WRLVL_CONFIG[0], (2 << 4)); // tWLO[7:4]
//    WriteIO32( &pReg_Drex->WRLVL_CONFIG[0],     (tWLO   <<   4) );          //
//    tWLO[7:4]
#else

	// Step 14.  AC Timing
	WriteIO32(&pReg_Drex->TIMINGAREF,
		  pSBI->DII.TIMINGAREF); //- refresh counter, 800MHz : 0x618

	WriteIO32(&pReg_Drex->ACTIMING0.TIMINGROW, pSBI->DII.TIMINGROW);
	WriteIO32(&pReg_Drex->ACTIMING0.TIMINGDATA, pSBI->DII.TIMINGDATA);
	WriteIO32(&pReg_Drex->ACTIMING0.TIMINGPOWER, pSBI->DII.TIMINGPOWER);

	if (pSBI->DII.ChipNum > 1) {
		WriteIO32(&pReg_Drex->ACTIMING1.TIMINGROW, pSBI->DII.TIMINGROW);
		WriteIO32(&pReg_Drex->ACTIMING1.TIMINGDATA,
			  pSBI->DII.TIMINGDATA);
		WriteIO32(&pReg_Drex->ACTIMING1.TIMINGPOWER,
			  pSBI->DII.TIMINGPOWER);
	}

	//    WriteIO32( &pReg_Drex->TIMINGPZQ,   0x00004084 );
	//    //- average periodic ZQ interval. Max:0x4084
	WriteIO32(
	    &pReg_Drex->TIMINGPZQ,
	    pSBI->DII.TIMINGPZQ); //- average periodic ZQ interval. Max:0x4084

	WriteIO32(&pReg_Drex->WRLVL_CONFIG[0], (2 << 4)); // tWLO[7:4]
//    WriteIO32( &pReg_Drex->WRLVL_CONFIG[0],     (tWLO   <<   4) );          //
//    tWLO[7:4]
#endif

#if 1 // fix - active
	WriteIO32(
	    &pReg_Drex->MEMCONTROL,
	    (0x0 << 29) | // [31:29] pause_ref_en : Refresh command issue Before
			  // PAUSE ACKNOLEDGE
		(0x0 << 28) | // [   28] sp_en        : Read with Short Preamble
			      // in Wide IO Memory
		(0x0 << 27) | // [   27] pb_ref_en    : Per bank refresh for
			      // LPDDR4/LPDDR3
		//        (0x0    <<  25) |           // [26:25] reserved     :
		//        SBZ
		(0x0 << 24) | // [   24] pzq_en       : DDR3 periodic ZQ(ZQCS)
			      // enable
		//        (0x0    <<  23) |           // [   23] reserved     :
		//        SBZ
		(0x3 << 20) | // [22:20] bl           : Memory Burst Length
			      // :: 3'h3  - 8
#if (CFG_NSIH_EN == 0)
		((_DDR_CS_NUM - 1) << 16) | // [19:16] num_chip : Number of
					    // Memory Chips                ::
					    // 4'h0  - 1chips
#else
		((pSBI->DII.ChipNum - 1) << 16) | // [19:16] num_chip : Number
						  // of Memory Chips
						  // :: 4'h0  - 1chips
#endif
		(0x2 << 12) | // [15:12] mem_width    : Width of Memory Data Bus
			      // :: 4'h2  - 32bits
#if defined(MEM_TYPE_DDR3)
		(0x6 << 8) | // [11: 8] mem_type     : Type of Memory
			     // :: 4'h6  - ddr3
#endif
#if defined(MEM_TYPE_LPDDR23)
		(0x7 << 8) | // [11: 8] mem_type     : Type of Memory
			     // :: 4'h7  - lpddr3
#endif
		(0x0 << 6) | // [ 7: 6] add_lat_pall : Additional Latency for
			     // PALL in cclk cycle :: 2'b00 - 0 cycle
		(0x0 << 5) | // [    5] dsref_en     : Dynamic Self Refresh
			     // :: 1'b0  - Disable
		//        (0x0    <<   4) |           // [    4] Reserved     :
		//        SBZ
		(0x0 << 2) | // [ 3: 2] dpwrdn_type  : Type of Dynamic Power
			     // Down                :: 2'b00 - Active/precharge
			     // power down
		(0x0 << 1) | // [    1] dpwrdn_en    : Dynamic Power Down
			     // :: 1'b0  - Disable
		(0x0 << 0)); // [    0] clk_stop_en  : Dynamic Clock Control
			     // :: 1'b0  - Always running
#endif

#if 0
#if (CFG_NSIH_EN == 0)
    WriteIO32( &pReg_DDRPHY->OFFSETR_CON[0],    READDELAY);
    WriteIO32( &pReg_DDRPHY->OFFSETW_CON[0],    WRITEDELAY);
#else
    WriteIO32( &pReg_DDRPHY->OFFSETR_CON[0],    pSBI->DII.READDELAY);
    WriteIO32( &pReg_DDRPHY->OFFSETW_CON[0],    pSBI->DII.WRITEDELAY);
#endif
#else

	// set READ skew
	WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], 0x08080808);

	// set WRITE skew
	WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], 0x08080808);
#endif

	// set ctrl_shiftc value.
	//    WriteIO32( &pReg_DDRPHY->SHIFTC_CON,        0x00 );

	SetIO32(&pReg_Drex->PHYCONTROL,
		(0x1 << 3)); // Force DLL Resyncronization
	ClearIO32(&pReg_Drex->PHYCONTROL,
		  (0x1 << 3)); // Force DLL Resyncronization

	SetIO32(
	    &pReg_Drex->CONCONTROL,
	    (0x1 << 28)); // dfi_init_start[28]. DFI PHY initialization start
	while ((ReadIO32(&pReg_Drex->PHYSTATUS) & (0x1 << 3)) == 0)
		; // dfi_init_complete[3]. wait for DFI PHY initialization
		  // complete
	ClearIO32(
	    &pReg_Drex->CONCONTROL,
	    (0x1 << 28)); // dfi_init_start[28]. DFI PHY initialization clear

	if (isResume == 0) {
		// Step 18, 19 :  Send NOP command.
		SendDirectCommand(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_NOP, 1,
					  (SDRAM_MODE_REG)CNULL, CNULL);
#endif
		// Step 25 : Wait for minimum 200us
		DMC_Delay(100);

		// Step 26 : Send MR63 (RESET) command.
		SendDirectCommand(SDRAM_CMD_MRS, 0, 63, 0);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 63, 0);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 63, 0);
#endif

		// Step 27 : Wait for minimum 10us
		for (i = 0; i < 20000; i++) {
			SendDirectCommand(SDRAM_CMD_MRR, 0, 0,
					  0); // 0x9 = MRR (mode register
					      // reading), MR0_Device
					      // Information
			if (ReadIO32(&pReg_Drex->MRSTATUS) &
			    (1 << 0)) // OP0 = DAI (Device Auto-Initialization
				      // Status)
			{
				break;
			}
		}

#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		for (i = 0; i < 20000; i++) {
			SendDirectCommand(SDRAM_CMD_MRR, 1, 0,
					  0); // 0x9 = MRR (mode register
					      // reading), MR0_Device
					      // Information
			if (ReadIO32(&pReg_Drex->MRSTATUS) &
			    (1 << 0)) // OP0 = DAI (Device Auto-Initialization
				      // Status)
			{
				break;
			}
		}
#endif
#else
		if (pSBI->DII.ChipNum > 1) {
			for (i = 0; i < 20000; i++) {
				SendDirectCommand(SDRAM_CMD_MRR, 1, 0,
						  0); // 0x9 = MRR (mode
						      // register reading),
						      // MR0_Device Information
				if (ReadIO32(&pReg_Drex->MRSTATUS) &
				    (1 << 0)) // OP0 = DAI (Device
					      // Auto-Initialization Status)
				{
					break;
				}
			}
		}
#endif

		// 12. Send MR10 command - DRAM ZQ calibration
		SendDirectCommand(SDRAM_CMD_MRS, 0, 10,
				  0xFF); // 0x0 = MRS/EMRS (mode register
					 // setting), MR10_Calibration, 0xFF:
					 // Calibration command after
					 // initialization
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 10, 0xFF);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 10, 0xFF);
#endif

		// 13. Wait for minimum 1us (tZQINIT).
		DMC_Delay(267); // MIN 1us

		// Step 20 :  Send MR2 command.
		SendDirectCommand(SDRAM_CMD_MRS, 0, 2, MR2.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 2, MR2.Reg);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 2, MR2.Reg);
#endif

		// Step 21 :  Send MR1 command.
		SendDirectCommand(SDRAM_CMD_MRS, 0, 1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 1, MR1.Reg);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 1, MR1.Reg);
#endif

		// Step 22 :  Send MR3 command - I/O Configuration :: Drive
		// Strength
		SendDirectCommand(SDRAM_CMD_MRS, 0, 3, MR3.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 3, MR3.Reg);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 3, MR3.Reg);
#endif

		// 14. Send MR11 command - ODT control
		SendDirectCommand(SDRAM_CMD_MRS, 0, 11, MR11.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 11, MR11.Reg);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 11, MR11.Reg);
#endif

		// Send MR16 PASR_Bank command.
		SendDirectCommand(SDRAM_CMD_MRS, 0, 16, 0xFF);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 16, 0xFF);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 16, 0xFF);
#endif

		// Send MR17 PASR_Seg command.
		SendDirectCommand(SDRAM_CMD_MRS, 0, 17, 0xFF);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
		SendDirectCommand(SDRAM_CMD_MRS, 1, 17, 0xFF);
#endif
#else
		if (pSBI->DII.ChipNum > 1)
			SendDirectCommand(SDRAM_CMD_MRS, 1, 17, 0xFF);
#endif

		DMC_Delay(267); // MIN 1us
	}

#if 0 // fix - inactive
    WriteIO32( &pReg_Drex->MEMCONTROL,
        (0x0    <<  29) |           // [31:29] pause_ref_en : Refresh command issue Before PAUSE ACKNOLEDGE
        (0x0    <<  28) |           // [   28] sp_en        : Read with Short Preamble in Wide IO Memory
        (0x0    <<  27) |           // [   27] pb_ref_en    : Per bank refresh for LPDDR4/LPDDR3
//        (0x0    <<  25) |           // [26:25] reserved     : SBZ
        (0x0    <<  24) |           // [   24] pzq_en       : DDR3 periodic ZQ(ZQCS) enable
//        (0x0    <<  23) |           // [   23] reserved     : SBZ
        (0x3    <<  20) |           // [22:20] bl           : Memory Burst Length                       :: 3'h3  - 8
#if (CFG_NSIH_EN == 0)
        ((_DDR_CS_NUM-1)        <<  16) |   // [19:16] num_chip : Number of Memory Chips                :: 4'h0  - 1chips
#else
        ((pSBI->DII.ChipNum-1)  <<  16) |   // [19:16] num_chip : Number of Memory Chips                :: 4'h0  - 1chips
#endif
        (0x2    <<  12) |           // [15:12] mem_width    : Width of Memory Data Bus                  :: 4'h2  - 32bits
#if defined(MEM_TYPE_DDR3)
        (0x6    <<   8) |           // [11: 8] mem_type     : Type of Memory                            :: 4'h6  - ddr3
#endif
#if defined(MEM_TYPE_LPDDR23)
        (0x7    <<   8) |           // [11: 8] mem_type     : Type of Memory                            :: 4'h7  - lpddr3
#endif
        (0x0    <<   6) |           // [ 7: 6] add_lat_pall : Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
        (0x0    <<   5) |           // [    5] dsref_en     : Dynamic Self Refresh                      :: 1'b0  - Disable
//        (0x0    <<   4) |           // [    4] Reserved     : SBZ
        (0x0    <<   2) |           // [ 3: 2] dpwrdn_type  : Type of Dynamic Power Down                :: 2'b00 - Active/precharge power down
        (0x0    <<   1) |           // [    1] dpwrdn_en    : Dynamic Power Down                        :: 1'b0  - Disable
        (0x0    <<   0));           // [    0] clk_stop_en  : Dynamic Clock Control                     :: 1'b0  - Always running
#endif

#if (CFG_DDR_LOW_FREQ == 1)

#if 0
    // set CA0 ~ CA9 deskew code to 7h'00
    WriteIO32( &pReg_DDRPHY->CA_DSKEW_CON[0],   0x00000000 );   // CA0 ~ CA3
    WriteIO32( &pReg_DDRPHY->CA_DSKEW_CON[1],   0x00000000 );   // CA4 ~ CA7
    WriteIO32( &pReg_DDRPHY->CA_DSKEW_CON[2],   0x00000000 );   // CA8 ~ CA9, CK, CS0
    WriteIO32( &pReg_DDRPHY->CA_DSKEW_CON[3],   0x00000000 );   // RST, CS1, CKE0, CKE1

    // Step 30: ctrl_offsetr0~3 = 0x00, ctrl_offsetw0~3 = 0x00
    WriteIO32( &pReg_DDRPHY->OFFSETR_CON,       0x00000000 );
    WriteIO32( &pReg_DDRPHY->OFFSETW_CON,       0x00000000 );

    // Step 31: ctrl_offsetd[7:0] = 0x00
    WriteIO32( &pReg_DDRPHY->OFFSETD_CON,       0x00000000 );
#else

	// set CA0 ~ CA9 deskew code to 7h'08
	WriteIO32(&pReg_DDRPHY->CA_DSKEW_CON[0], 0x08080808); // CA0 ~ CA3
	WriteIO32(&pReg_DDRPHY->CA_DSKEW_CON[1], 0x08080808); // CA4 ~ CA7
	WriteIO32(&pReg_DDRPHY->CA_DSKEW_CON[2],
		  0x08080808); // CA8 ~ CA9, CK, CS0
	WriteIO32(&pReg_DDRPHY->CA_DSKEW_CON[3],
		  0x08080808); // RST, CS1, CKE0, CKE1

	// Step 30: ctrl_offsetr0~3 = 0x08, ctrl_offsetw0~3 = 0x08
	WriteIO32(&pReg_DDRPHY->OFFSETR_CON, 0x08080808);
	WriteIO32(&pReg_DDRPHY->OFFSETW_CON, 0x08080808);

	// Step 31: ctrl_offsetd[7:0] = 0x08
	WriteIO32(&pReg_DDRPHY->OFFSETD_CON, 0x00000008);
#endif

	// Step 33 : Wait 10MPCLK
	DMC_Delay(100);

	// Step 34: ctrl_start[6] = 0
	ClearIO32(&pReg_DDRPHY->MDLL_CON[0], (0x1 << 6));

	// Step 35 : ctrl_start[6] = 1
	SetIO32(&pReg_DDRPHY->MDLL_CON[0], (0x1 << 6));

	// Step 36 : Wait 10MPCLK
	DMC_Delay(100);

	// Step 21, 22: Update DLL information
	SetIO32(&pReg_Drex->PHYCONTROL,
		(0x1 << 3)); // Force DLL Resyncronization
	ClearIO32(&pReg_Drex->PHYCONTROL,
		  (0x1 << 3)); // Force DLL Resyncronization

	// Step 23 : Wait 100ns
	DMC_Delay(10);
#endif // #if (CFG_DDR_LOW_FREQ == 1)

// Set PLL3.
#if (CFG_DDR_LOW_FREQ == 1)
#if defined(MEM_TYPE_LPDDR23)
	while (!DebugIsUartTxDone())
		;
	setMemPLL(1);
#endif
#endif

#if 1

	//    printf("\r\n########## READ/GATE Level ##########\r\n");

	//======================================================================
	//======================== Training Preparation ========================
	//======================================================================

	ClearIO32(&pReg_DDRPHY->OFFSETD_CON,
		  (0x1 << 28)); // upd_mode=0, PHY side update mode.

	SetIO32(&pReg_DDRPHY->PHY_CON[0], (0x1 << 6));  // ctrl_atgate=1
	SetIO32(&pReg_DDRPHY->PHY_CON[0], (0x1 << 14)); // p0_cmd_en=1
	SetIO32(&pReg_DDRPHY->PHY_CON[2], (0x1 << 6));  // InitDeskewEn=1
	SetIO32(&pReg_DDRPHY->PHY_CON[0], (0x1 << 13)); // byte_rdlvl_en=1

	temp = ReadIO32(&pReg_DDRPHY->PHY_CON[1]) &
	       ~(0xF << 16); // rdlvl_pass_adj=4
	temp |= (0x4 << 16);
	WriteIO32(&pReg_DDRPHY->PHY_CON[1], temp);

#if defined(MEM_TYPE_DDR3)
	WriteIO32(&pReg_DDRPHY->LP_DDR_CON[3],
		  0x105E); // cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
	WriteIO32(
	    &pReg_DDRPHY->LP_DDR_CON[4],
	    0x107F); // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
#endif
#if defined(MEM_TYPE_LPDDR23)
	WriteIO32(&pReg_DDRPHY->LP_DDR_CON[3],
		  0x000E); // cmd_active= DDR3:0x105F, LPDDDR2 or LPDDDR3:0x000E
	WriteIO32(
	    &pReg_DDRPHY->LP_DDR_CON[4],
	    0x000F); // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
#endif

	temp = ReadIO32(&pReg_DDRPHY->PHY_CON[2]) &
	       ~(0x7F << 16); // rdlvl_incr_adj=1
	temp |= (0x1 << 16);
	WriteIO32(&pReg_DDRPHY->PHY_CON[2], temp);

#if 1
#if 0
    ClearIO32( &pReg_DDRPHY->MDLL_CON[0],       (0x1    <<   5) );          // ctrl_dll_on[5]=0

    do {
        temp = ReadIO32( &pReg_DDRPHY->MDLL_CON[1] );                       // read lock value
    } while( (temp & 0x7) < 0x5 );
#else

	do {
		SetIO32(&pReg_DDRPHY->MDLL_CON[0],
			(0x1 << 5)); // ctrl_dll_on[5]=1

		do {
			temp = ReadIO32(
			    &pReg_DDRPHY->MDLL_CON[1]); // read lock value
		} while ((temp & 0x7) != 0x7);

		ClearIO32(&pReg_DDRPHY->MDLL_CON[0],
			  (0x1 << 5)); // ctrl_dll_on[5]=0

		temp = ReadIO32(&pReg_DDRPHY->MDLL_CON[1]); // read lock value
	} while ((temp & 0x7) != 0x7);
#endif

	g_Lock_Val = (temp >> 8) & 0x1FF;
#else

	showLockValue();
#endif

	temp = ReadIO32(&pReg_DDRPHY->MDLL_CON[0]) & ~(0x1FF << 7);
	temp |= (g_Lock_Val << 7);
	WriteIO32(&pReg_DDRPHY->MDLL_CON[0], temp); // ctrl_force[16:8]

#if (SKIP_LEVELING_TRAINING == 0)
	if (isResume == 0) {
#if defined(MEM_TYPE_DDR3)
		if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
			DDR_HW_Write_Leveling();
#endif

#if 0
        if (pSBI->LvlTr_Mode & LVLTR_CA_CAL)
            DDR_CA_Calibration();
#endif

		if (pSBI->LvlTr_Mode & LVLTR_GT_LVL) {
			if (DDR_Gate_Leveling() == CFALSE)
				return CFALSE;
		}

		if (pSBI->LvlTr_Mode & LVLTR_RD_CAL)
			DDR_Read_DQ_Calibration();

#if defined(MEM_TYPE_DDR3)
		if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
			DDR_Write_Latency_Calibration();
#endif

		if (pSBI->LvlTr_Mode & LVLTR_WR_CAL)
			DDR_Write_DQ_Calibration();

//----------------------------------
// Save leveling & training values.
#if 0
        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     1);                 // open alive power gate

        WriteIO32(&pReg_Alive->ALIVESCRATCHRST5,    0xFFFFFFFF);        // clear - ctrl_shiftc
        WriteIO32(&pReg_Alive->ALIVESCRATCHRST6,    0xFFFFFFFF);        // clear - ctrl_offsetC
        WriteIO32(&pReg_Alive->ALIVESCRATCHRST7,    0xFFFFFFFF);        // clear - ctrl_offsetr
        WriteIO32(&pReg_Alive->ALIVESCRATCHRST8,    0xFFFFFFFF);        // clear - ctrl_offsetw

        WriteIO32(&pReg_Alive->ALIVESCRATCHSET5,    g_GT_cycle);        // store - ctrl_shiftc
        WriteIO32(&pReg_Alive->ALIVESCRATCHSET6,    g_GT_code);         // store - ctrl_offsetc
        WriteIO32(&pReg_Alive->ALIVESCRATCHSET7,    g_RD_vwmc);         // store - ctrl_offsetr
        WriteIO32(&pReg_Alive->ALIVESCRATCHSET8,    g_WR_vwmc);         // store - ctrl_offsetw

        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     0);                 // close alive power gate
#endif
	} else {
		U32 lock_div4 = (g_Lock_Val >> 2);

		//----------------------------------
		// Restore leveling & training values.
		WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,
			  1); // open alive power gate
		DMC_Delay(100);
		g_GT_cycle = ReadIO32(
		    &pReg_Alive->ALIVESCRATCHVALUE5); // read - ctrl_shiftc
		g_GT_code = ReadIO32(
		    &pReg_Alive->ALIVESCRATCHVALUE6); // read - ctrl_offsetc
		g_RD_vwmc = ReadIO32(
		    &pReg_Alive->ALIVESCRATCHVALUE7); // read - ctrl_offsetr
		g_WR_vwmc = ReadIO32(
		    &pReg_Alive->ALIVESCRATCHVALUE8); // read - ctrl_offsetw
		//        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     0);
		//        // close alive power gate

		if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
			WriteIO32(&pReg_DDRPHY->WR_LVL_CON[0], g_WR_lvl);

#if 0
        if (pSBI->LvlTr_Mode & LVLTR_CA_CAL)
            DDR_CA_Calibration();
#endif

		if (pSBI->LvlTr_Mode & LVLTR_GT_LVL) {
			U32 i, min;
			U32 GT_cycle = 0;
			U32 GT_code = 0;

			SetIO32(&pReg_DDRPHY->PHY_CON[2],
				(0x1 << 24)); // gate_cal_mode[24] = 1
			SetIO32(
			    &pReg_DDRPHY->PHY_CON[0],
			    (0x5 << 6)); // ctrl_shgate[8]=1, ctrl_atgate[6]=1
			ClearIO32(&pReg_DDRPHY->PHY_CON[1],
				  (0xF << 20)); // ctrl_gateduradj[23:20] =
						// DDR3: 0x0, LPDDR3: 0xB,
						// LPDDR2: 0x9
			SetIO32(&pReg_DDRPHY->PHY_CON[1],
				(0xB << 20)); // ctrl_gateduradj[23:20] = DDR3:
					      // 0x0, LPDDR3: 0xB, LPDDR2: 0x9

			min = g_GT_cycle & 0x7;
			for (i = 1; i < 4; i++) {
				temp = (g_GT_cycle >> (3 * i)) & 0x7;
				if (temp < min)
					min = temp;
			}

			if (min) {
				GT_cycle = (g_GT_cycle & 0x7) - min;
				for (i = 1; i < 4; i++) {
					temp = ((g_GT_cycle >> (3 * i)) & 0x7) -
					       min;
					GT_cycle |= (temp << (3 * i));
				}

				min = ((ReadIO32(&pReg_DDRPHY->PHY_CON[1]) >>
					28) &
				       0xF) +
				      min; // ctrl_gateadj[31:28]

				temp = ReadIO32(&pReg_DDRPHY->PHY_CON[1]) &
				       0x0FFFFFFF;
				temp |= (min << 28);
				WriteIO32(&pReg_DDRPHY->PHY_CON[1], temp);
			}

			MEMMSG("min = %d\r\n", min);
			MEMMSG("GT_cycle  = 0x%08X\r\n", GT_cycle);

			GT_code = getVWMC_Offset(g_GT_code, lock_div4);
			WriteIO32(&pReg_DDRPHY->OFFSETC_CON[0], GT_code);
			WriteIO32(&pReg_DDRPHY->SHIFTC_CON, 0x00);

			ClearIO32(&pReg_DDRPHY->PHY_CON[2],
				  (0x1 << 24)); // gate_cal_mode[24] = 0
			WriteIO32(&pReg_DDRPHY->LP_CON,
				  0x0); // ctrl_pulld_dqs[8:0] = 0
			ClearIO32(&pReg_DDRPHY->RODT_CON,
				  (0x1 << 16)); // ctrl_read_dis[16] = 0
		}

		if (pSBI->LvlTr_Mode & LVLTR_RD_CAL) {
			g_RD_vwmc = getVWMC_Offset(g_RD_vwmc, lock_div4);
			WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], g_RD_vwmc);
		}

#if defined(MEM_TYPE_DDR3)
		if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
			DDR_Write_Latency_Calibration();
#endif

		if (pSBI->LvlTr_Mode & LVLTR_WR_CAL) {
			g_WR_vwmc = getVWMC_Offset(g_WR_vwmc, lock_div4);
			WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], g_WR_vwmc);
		}
	}

	SetIO32(&pReg_DDRPHY->OFFSETD_CON,
		(0x1 << 24)); // ctrl_resync[24]=0x1 (HIGH)
	ClearIO32(&pReg_DDRPHY->OFFSETD_CON,
		  (0x1 << 24)); // ctrl_resync[24]=0x0 (LOW)
#endif				// #if (SKIP_LEVELING_TRAINING == 0)

	ClearIO32(&pReg_DDRPHY->PHY_CON[0],
		  (0x3 << 13)); // p0_cmd_en[14]=0, byte_rdlvl_en[13]=0

	SetIO32(&pReg_DDRPHY->MDLL_CON[0], (0x1 << 5)); // ctrl_dll_on[5]=1
	SetIO32(&pReg_DDRPHY->PHY_CON[2], (0x1 << 12)); // DLLDeskewEn[12]=1

	SetIO32(&pReg_DDRPHY->OFFSETD_CON, (0x1 << 28)); // upd_mode=1

	SetIO32(&pReg_Drex->PHYCONTROL,
		(0x1 << 3)); // Force DLL Resyncronization
	ClearIO32(&pReg_Drex->PHYCONTROL,
		  (0x1 << 3)); // Force DLL Resyncronization
#endif			       // gate leveling

	/* Send PALL command */
	SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
	SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
	if (pSBI->DII.ChipNum > 1)
		SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL,
				  CNULL);
#endif

	WriteIO32(
	    &pReg_Drex->PHYCONTROL,
	    (0x1 << 31) | // [   31] mem_term_en. Termination Enable for memory.
			  // Disable : 0, Enable : 1
		(0x1 << 30) | // [   30] phy_term_en. Termination Enable for
			      // PHY. Disable : 0, Enable : 1
#if defined(MEM_TYPE_DDR3)
		(0x1 << 29) | // [   29] ctrl_shgate. Duration of DQS Gating
			      // Signal. gate signal length <= 200MHz : 0,
			      // >200MHz : 1
#endif
		(0x0 << 24) | // [28:24] ctrl_pd. Input Gate for Power Down.
		(0x0 << 8) |  // [    8] Termination Type for Memory Write ODT
			      // (0:single, 1:both chip ODT)
		(0x0 << 7) |  // [    7] Resync Enable During PAUSE Handshaking
		(0x0 << 4) |  // [ 6: 4] dqs_delay. Delay cycles for DQS
			      // cleaning. refer to DREX datasheet
		(0x0 << 3) |  // [    3] fp_resync. Force DLL Resyncronization :
			      // 1. Test : 0x0
		(0x0 << 2) |  // [    2] Drive Memory DQ Bus Signals
		(0x0 << 1) |  // [    1] sl_dll_dyn_con. Turn On PHY slave DLL
			      // dynamically. Disable : 0, Enable : 1
		(0x1 << 0));  // [    0] mem_term_chips. Memory Termination
			      // between chips(2CS). Disable : 0, Enable : 1

	temp =
	    (U32)((0x0 << 28) |   // [   28] dfi_init_start
		  (0xFFF << 16) | // [27:16] timeout_level0
		  (0x1 << 12) |   // [14:12] rd_fetch
		  (0x1 << 8) |    // [    8] empty
		  (0x0 << 6) |    // [ 7: 6] io_pd_con
		  (0x1 << 5) |    // [    5] aref_en - Auto Refresh Counter.
				  // Disable:0, Enable:1
		  (0x0 << 3) | // [    3] update_mode - Update Interface in DFI.
		  (0x0 << 1) | // [ 2: 1] clk_ratio
		  (0x0 << 0)); // [    0] ca_swap

	if (isResume)
		temp &= ~(0x1 << 5);

	WriteIO32(&pReg_Drex->CONCONTROL, temp);

	WriteIO32(&pReg_Drex->CGCONTROL,
		  (0x0 << 4) |     // [    4] phy_cg_en
		      (0x0 << 3) | // [    3] memif_cg_en
		      (0x0 << 2) | // [    2] scg_sg_en
		      (0x0 << 1) | // [    1] busif_wr_cg_en
		      (0x0 << 0)); // [    0] busif_rd_cg_en

	printf("\r\n");

	printf("Lock value  = %d\r\n", g_Lock_Val);

	printf("CA CAL CODE = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->PHY_CON[10]));

	printf("GATE CYC    = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->PHY_CON[3]));
	printf("GATE CODE   = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->PHY_CON[8]));

	printf("Read  DQ    = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->PHY_CON[4]));
	printf("Write DQ    = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->PHY_CON[6]));

	return CTRUE;
}
