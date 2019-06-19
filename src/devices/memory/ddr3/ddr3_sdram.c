/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Deokjin, Lee <truevirtue@nexell.co.kr>
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

#include <sysheader.h>

#include <drex.h>
#include <ddrphy.h>
#include <ddr3_ac_timing.h>
#include <ddr3_sdram.h>

#define CFG_ODT_OFF 			(0)
#define CFG_FLY_BY 			(0)
#define CFG_8BIT_DESKEW 		(0)
#define CFG_UPDATE_DREX_SIDE 		(1) 	// 0 : PHY side,  1: Drex side
#define SKIP_LEVELING_TRAINING 		(0)

//#define USE_HEADER
#define DDR_RW_CAL 			(0)

/* Hardware Memory Calibration */
#define DDR_WRITE_LEVELING_EN 		(1)	// for fly-by
#define DDR_GATE_LEVELING_EN 		(1)	// for DDR3, great then 800MHz
#define DDR_READ_DQ_CALIB_EN 		(1)
#define DDR_WRITE_DQ_CALIB_EN 		(1)

#define DDR_RESET_GATE_LVL 		(1)
#define DDR_RESET_READ_DQ 		(1)
#define DDR_RESET_WRITE_DQ 		(1)

#define DDR_MEMINFO_SHOWLOCK		(0)	// DDR Memory Show Lock Value

#define MEM_CALIBRATION_INFO		(1)
#define MEM_CALIBRATION_BITINFO		(0)
#define DM_CALIBRATION_INFO		(0)

#ifdef aarch32
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");
#endif

#ifdef aarch64
#define nop() __asm__ __volatile__("mov\tx0,x0\t\r\n nop\n\t");
#endif

/* External Function */
extern void DMC_Delay(int milisecond);

extern struct s5p6818_resetgen_reg *const g_rstgen_reg;

struct s5p6818_drex_sdram_reg *const g_drex_reg =
    (struct s5p6818_drex_sdram_reg * const)PHY_BASEADDR_DREX_MODULE_CH0_APB;
struct s5p6818_ddrphy_reg *const g_ddrphy_reg =
    (struct s5p6818_ddrphy_reg * const)PHY_BASEADDR_DREX_MODULE_CH1_APB;
struct s5p6818_drex_tz_reg *const g_drextz_reg =
    (struct s5p6818_drex_tz_reg * const)PHY_BASEADDR_DREX_TZ_MODULE;

unsigned int g_Lock_Val;
unsigned int g_WR_lvl;
unsigned int g_GT_cycle;
unsigned int g_GT_code;
unsigned int g_RD_vwmc;
unsigned int g_WR_vwmc;

struct dram_device_info g_ddr3_info;

static void get_dram_information(struct dram_device_info *me)
{
	int byte = 8;

	/* Nexell Step XX. Memory Address (for Write Training (DRAM)) */
	me->bank_num	= (DDR3_BANK_NUM);
	me->row_num	= (DDR3_ROW_NUM + 12);
	me->column_num	= (DDR3_COL_NUM +  7);

	me->column_size	= (1 << me->column_num)/byte;				// Unit : Byte
	me->row_size	= (1 << me->row_num);
	me->bank_size	= (me->row_size * me->column_size);
#if 1
	me->chip_size	= (((me->bank_size * (1 << me->bank_num))
				* DDR3_BUS_WIDTH)/ 1024 / 1024);		// Unit: MB
	me->sdram_size	= (me->chip_size * (DDR3_CS_NUM * 32 / DDR3_BUS_WIDTH));
#else
	me->chip_size	= (CONFIG_DDR3_CHIP_PERSIZE/1024/1024);			// Unit: MB
	me->sdram_size	= (CONFIG_DDR3_MEMSIZE/1024/1024);
#endif

#if 0
	MEMMSG("############## [SDRAM] Memory Specification ###############\r\n");
	MEMMSG("[Bit] Bank Address   : %d \r\n", me->bank_num);
	MEMMSG("[Bit] Column Address : %d \r\n", me->column_num);
	MEMMSG("[Bit] Row Address    : %d \r\n", me->row_num);
	MEMMSG("[Bit] Data Line      : %d \r\n", DDR3_BUS_WIDTH);
	MEMMSG("[BYTE] Column    Size: %d \r\n", me->column_size);
	MEMMSG("[BYTE] Row(Page) Size: %d \r\n", me->row_size);
	MEMMSG("[BYTE] Bank      Size: %d \r\n", me->bank_size);
#if 1
	MEMMSG("[MB]   Chip      Size: %d \r\n", me->chip_size);
	MEMMSG("[MB]   SDRAM     Size: %d \r\n", me->sdram_size);
#endif
	MEMMSG("############################################################\r\n");
#endif
}

#if (SUPPORT_KERNEL_3_4 == 0)
void ddr3_save_information(void)
{
	unsigned int value = 0;

	/*
	  * memory infomration
	  * (4bit << 12) | (4bot << 8) | (3bit << 4) | (1bit << 2) | (2bit << 0)
	  */
	value = ((nCWL << 12) | (MR1_nAL << 8) | (CONFIG_DRAM_MR1_RTT_Nom << 4) |
			(CONFIG_DRAM_MR1_ODS << 2) | (DDR3_CS_NUM << 0));

	mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG, 1);

	/* Save the Memory information for ATF suspend */
	mmio_write_32(&pReg_Alive->ALIVESCRATCHRST6, 0xFFFFFFFF);
	mmio_write_32(&pReg_Alive->ALIVESCRATCHSET6, value);
}
#endif

#if DDR_MEMINFO_SHOWLOCK
struct phy_lock_info {
	unsigned int val;
	unsigned int count;
	unsigned int lock_count[5];
};

void show_lock_value(void)
{
	struct phy_lock_info lock_info[20];
	unsigned int fFound = 0;
	unsigned int status, lock_val;
	unsigned int temp, i, j;

	MEMMSG("[MSG] Waiting for DDR3 Lock value! \r\n");

	for (i = 0; i < 20; i++) {
		lock_info[i].val	= 0;
		lock_info[i].count	= 0;

		for (j = 0; j < 5; j++)
			lock_info[i].lock_count[j = 0;
	}

	for (i = 0; i < 1000000; i++) {
		temp = mmio_read_32( &g_ddrphy_reg->MDLL_CON[1] );
		status = temp & 0x7;
		lock_val = (temp >> 8) & 0x1FF;         // Read Lock value

		fFound = 0;
		for (j = 0; lock_info[j].val != 0; j++) {
			if (lock_info[j].val == lock_val) {
				fFound = 1;
				lock_info[j].count++;
				if (status)
					lock_info[j].lock_count[(status >> 1)]++;
				else
					lock_info[j].lock_count[4]++;
			}
		}

		if (j == 20)
			break;

		if (fFound == 0) {
			lock_info[j].val   = lock_val;
			lock_info[j].count = 1;

			if (status)
				lock_info[j].lock_count[(status>>1)] = 1;
			else
				lock_info[j].lock_count[4] = 1;
		}

		DMC_Delay(10);
	}

	MEMMSG("\r\n");
	MEMMSG("----------------------------------------\r\n");
	MEMMSG(" Show lock values : %d			\r\n", g_Lock_Val );
	MEMMSG("----------------------------------------\r\n");

	MEMMSG("lock_val,   hit       bad, not bad,   good, better,   best\r\n");

	for (i = 0; lock_info[i].val; i++) {
		MEMMSG("[%6d, %6d] - [%6d", lock_info[i].val,
			lock_info[i].count, lock_info[i].lock_count[4]);
		for (j = 0; j < 4; j++)
			MEMMSG(", %6d", lock_info[i].lock_count[j]);
		MEMMSG("]\r\n");
	}
}
#endif

#if 0
void DUMP_PHY_REG(void)
{
    unsigned int     *pAddr = (unsigned int *)&g_ddrphy_reg->PHY_CON[0];
    unsigned int     temp;
    unsigned int     i;

    for (i = 0; i < (0x3AC>>2); i++)
    {
        temp = mmio_read_32( pAddr + i );

        if ( (i & 3) == 0 ) {
            MEMMSG("\r\n0x%08X :", (i<<2));
        }

        MEMMSG(" %08x", temp);
    }
    MEMMSG("\r\n");
}
#endif

void send_directcmd(SDRAM_CMD cmd, U8 chipnum, SDRAM_MODE_REG mrx, unsigned short value)
{
	mmio_write_32(&g_drex_reg->DIRECTCMD, (unsigned int)((cmd<<24) |
		((chipnum & 1)<<20) | (mrx<<16) | value));
}

void enter_self_refresh(void)
{
	union SDRAM_MR MR;
	unsigned int nTemp;
	unsigned int nChips = 0;

#if (DDR3_CS_NUM > 1)
	nChips = 0x3;
#else
	nChips = 0x1;
#endif
	while (mmio_read_32(&g_drex_reg->CHIPSTATUS) & 0xF)
		nop();

	/* Step 01. Send PALL Command */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
	DMC_Delay(100);

	/* Step 02. (DRAM) ODT OFF */
	MR.Reg = 0;
	MR.MR2.RTT_WR = 0;							// 0: disable, 1: RZQ/4 (60ohm), 2: RZQ/2 (120ohm)
	MR.MR2.SRT = 0;								// self refresh normal range, if (ASR == 1) SRT = 0;
	MR.MR2.ASR = 1;								// auto self refresh enable
	MR.MR2.CWL = (nCWL - 5);
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR.Reg);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif

	/* Step 03. (DRAM) Set the Drive Strength */
	MR.Reg = 0;
	MR.MR1.DLL = 1;								// 0: Enable, 1 : Disable
	MR.MR1.AL = MR1_nAL;
	MR.MR1.ODS1 = (CONFIG_DRAM_MR1_ODS >> 1) & 1;
	MR.MR1.ODS0 = (CONFIG_DRAM_MR1_ODS >> 0) & 1;
	MR.MR1.RTT_Nom2 = (CONFIG_DRAM_MR1_RTT_Nom >> 2) & 1;
	MR.MR1.RTT_Nom1 = (CONFIG_DRAM_MR1_RTT_Nom >> 1) & 1;
	MR.MR1.RTT_Nom0 = (CONFIG_DRAM_MR1_RTT_Nom >> 0) & 1;
	MR.MR1.QOff = 0;
	MR.MR1.WL = 0;

#if 0
	MR.MR1.TDQS     = (_DDR3_BUS_WIDTH>>3) & 1;
#endif

	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR.Reg);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif

	/* Step 04. Enter Self-Refresh Command */
	send_directcmd(SDRAM_CMD_REFS, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_REFS, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

	/*  Step 05. Check the Busy State */
	do {
		nTemp = (mmio_read_32(&g_drex_reg->CHIPSTATUS) & nChips);
	} while (nTemp);

	/* Step 06. Check the Sel-Refresh State (FSM) */
	do {
		nTemp = ((mmio_read_32(&g_drex_reg->CHIPSTATUS) >> 8) & nChips);
	} while (nTemp != nChips);

	/* Step 05. Disable the Auto refresh Counter */
	mmio_clear_32(&g_drex_reg->CONCONTROL, (0x1 << 5));			// afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1

	/* Step  06. Disable the Dynamic Clock */
	mmio_set_32(&g_drex_reg->MEMCONTROL, (0x1 << 0));			// clk_stop_en[0] : Dynamic Clock Control   :: 1'b0 - Always running
}

void exit_self_refresh(void)
{
	union SDRAM_MR MR;

	/* Step  01. Enable the Dynamic Clock */
	mmio_clear_32(&g_drex_reg->MEMCONTROL, (0x1 << 0));			// clk_stop_en[0] : Dynamic Clock Control :: 1'b0  - Always running
	DMC_Delay(10);

	/* Step 02. Enable the Auto refresh counter */
	mmio_set_32(&g_drex_reg->CONCONTROL, (0x1 << 5));			// afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1
	DMC_Delay(10);

	/* Step 03. Send PALL command */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

	/* Step 04. Set the Drive Strength */
	MR.Reg = 0;
	MR.MR1.DLL = 0;								// 0: Enable, 1 : Disable
	MR.MR1.AL = MR1_nAL;
	MR.MR1.ODS1 = (CONFIG_DRAM_MR1_ODS >> 1) & 1;
	MR.MR1.ODS0 = (CONFIG_DRAM_MR1_ODS >> 0) & 1;
	MR.MR1.RTT_Nom2 = (CONFIG_DRAM_MR1_RTT_Nom >> 2) & 1;
	MR.MR1.RTT_Nom1 = (CONFIG_DRAM_MR1_RTT_Nom >> 1) & 1;
	MR.MR1.RTT_Nom0 = (CONFIG_DRAM_MR1_RTT_Nom >> 0) & 1;
	MR.MR1.QOff = 0;
	MR.MR1.WL = 0;
#if 0
	MR.MR1.TDQS     = (_DDR3_BUS_WIDTH>>3) & 1;
#endif

	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR.Reg);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif

	/* Step 05. Set the ODT On */
	MR.Reg = 0;
	MR.MR2.RTT_WR = CONFIG_DRAM_MR2_RTT_WR;
	MR.MR2.SRT = 0;								// self refresh normal range
	MR.MR2.ASR = 0;								// auto self refresh disable
	MR.MR2.CWL = (nCWL - 5);
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR.Reg);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif

	/* Step 06. Exit the Self-Refresh Command */
	send_directcmd(SDRAM_CMD_REFSX, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_REFSX, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

	/* Step 07. Check the Self-Refresh State (FSM) */
#if 0
	while(mmio_read_32(&g_drex_reg->CHIPSTATUS) & (0xF << 8))
		nop();
#endif
}

#if (SKIP_LEVELING_TRAINING == 0)

#if ((DDR_WRITE_LEVELING_EN == 1) && (MEM_CALIBRATION_INFO == 1))
void hw_write_leveling_information(void)
{
	int wl_calibration;
	int wl_dll_value[4];

	int max_slice = 4, slice;

	wl_calibration = mmio_read_32(&g_ddrphy_reg->WR_LVL_CON[0]);
	MEMMSG("SLICE %03d %03d %03d %03d\r\n     ", 0, 1, 2, 3);
	for (slice = 0; slice < max_slice; slice++) {
		wl_dll_value[slice] = (wl_calibration >> (slice * 8)) & 0xFF;
		MEMMSG(" %03X", wl_dll_value[slice]);
	}
	MEMMSG("\r\n");
}
#endif

#if (DDR_WRITE_LEVELING_EN == 1)
/*************************************************************
 * Must be S5P6818
 * Hardware Write Leveling sequence in S5P6818
 * must go through the following steps:
 *
 * Step 01. Send ALL Precharge command. (Suspend/Resume/Option)
 *	    - Set "cmd_default[8:7] =2'b11" (LPDDR_CON4[8:7]) to enable
 *	      "ODT[1:0]" signals during Write Leveling.
 * Step 02. Set the MR1 Register for Write Leveling Mode.
 * Step 03. Memory Controller should configure Memory in Write Level Mode.
 * Step 04. Configure PHY in Write Level mode
 *	     - Enable "wrlvl_mode" in PHY_CON[16] = '1' "
 * Step 05.  Start Write Leveling.
 *	     - Set the "wrlvel_start = 1'b1" (=PHY_CON3[16])
 * Step 06. Waiting for (DRAM)Response.
 *	     - Wait until "wrlvel_resp = 1'b1" (=PHY_CON3[24])
 * Step 07. Finish Write Leveling.
 *	     - Set the "wrlvel_Start=1'b0" (=PHY_CON3[16])
 * Step 08. Configure PHY in normal mode
 *	     - Disable "wrlvel_mode" in PHY_CON0[16]
 * Step 09. Disable ODT[1:0]
 *	     - Set "cmd_default[8:7]=2'b00' (LPDDR_CON4[8:7]).
 * Step 10. Disable Memory in Write Leveling Mode
 * Step 11. Update ALL SDLL Resync.
 * Step 12-0. Hardware Write Leveling Information
 * Step 12-1. It adjust the duration cycle of "ctrl_read" on a
 *	        clock cycle base. (subtract delay)
 *************************************************************/
int ddr_hw_write_leveling(void)
{
	union SDRAM_MR MR1;

	volatile unsigned int cal_count = 0;
	unsigned int response;
	int ret = 0;

	MEMMSG("\r\n########## Write Leveling - Start ##########\r\n");

	/* Step 01. Send ALL Precharge command. */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);

//	DMC_Delay(0x100);

	/* Step 02. Set the MR1 Register for Write Leveling Mode */
	MR1.Reg		 = 0;
	MR1.MR1.DLL	 = 0;							// 0: Enable, 1 : Disable
	MR1.MR1.AL	 = MR1_nAL;
#if 1
	MR1.MR1.ODS1 	 = 0;							// 00: RZQ/6, 01 : RZQ/7
	MR1.MR1.ODS0	 = 1;
#else
	MR1.MR1.ODS1 = 0; // 00: RZQ/6, 01 : RZQ/7
	MR1.MR1.ODS0 = 0;
#endif
	MR1.MR1.QOff	 = 0;
	MR1.MR1.RTT_Nom2 = 0;							// RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
	MR1.MR1.RTT_Nom1 = 1;
	MR1.MR1.RTT_Nom0 = 1;
	MR1.MR1.WL	 = 1;							// Write leveling enable

	/* Step 03. Memory controller settings for the Write Leveling Mode. */
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);

//	DMC_Delay(0x100);

	/* Step 03-02. Enable the ODT[1:0] (Signal High) */
	mmio_set_32  (&g_ddrphy_reg->LP_DDR_CON[4], (0x3 << 7) );          	// cmd_default, ODT[8:7]=0x3

	/* Step 04. Configure PHY in Write Level mode */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[0], (0x1 << 16));

	/* Step 05. Start Write Leveling. */
	mmio_write_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 16));

	/* Step 06. Waiting for (DRAM)Response. */
	for (cal_count = 0; cal_count < 100; cal_count++) {
		response = mmio_read_32(&g_ddrphy_reg->PHY_CON[3]);
		if (response & (0x1 << 24))
			break;

		MEMMSG("WRITE LVL: Waiting wrlvl_resp...!!!\r\n");
		DMC_Delay(100);
	}

	/* Step 07. Finish Write Leveling. */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 16));

	/* Step 08. Configure PHY in normal mode */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[0], (0x1 << 16));

	if (cal_count == 100) {
		MEMMSG("WRITE LVL: Leveling Responese Checking : fail...!!!\r\n");
		ret = -1; 	// Failure Case
	}
	g_WR_lvl = mmio_read_32(&g_ddrphy_reg->WR_LVL_CON[0]);

	/* Step 09. Disable the ODT[1:0] */
	mmio_clear_32(&g_ddrphy_reg->LP_DDR_CON[4], (0x3 << 7));          	// cmd_default, ODT[8:7]=0x0

	/* Step 10. Disable Memory in Write Leveling Mode */
	MR1.MR1.WL      = 0;
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);

	DMC_Delay(0x100);

	/* Step 11. Update ALL SDLL Resync. */
	mmio_set_32  (&g_ddrphy_reg->OFFSETD_CON, (0x1  << 24));		// ctrl_resync[24]=0x1 (HIGH)
	mmio_clear_32(&g_ddrphy_reg->OFFSETD_CON, (0x1  << 24));		// ctrl_resync[24]=0x0 (LOW)

	/* Step 12-0. Hardware Write Leveling Information */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 0));	    		// reg_mode[7:0]=0x1
#if (MEM_CALIBRATION_INFO == 1)
	hw_write_leveling_information();
#endif
	/* Step 12-0. Leveling Information Register Mode Off (Not Must) */
//	mmio_clear_32(&g_ddrphy_reg->PHY_CON[3],  (0xFF << 0));		// reg_mode[7:0]=0x0

	/* Step 12-1. It adjust the duration cycle of "ctrl_read" on a clock cycle base. (subtract delay) */
	mmio_set_32  (&g_ddrphy_reg->RODT_CON,    (0x1  << 28));		// ctrl_readduradj [31:28]

	MEMMSG("\r\n########## Write Leveling - End ##########\r\n");

	return ret;
}
#endif // #if (DDR_WRITE_LEVELING_EN == 1)

#if ((DDR_GATE_LEVELING_EN == 1) && (MEM_CALIBRATION_INFO == 1))
void gate_leveling_information(void)
{
	unsigned int slice, max_slice = 4;
	unsigned int status, gate_center[4], gate_cycle[4], lock_value;

	status = mmio_read_32(&g_ddrphy_reg->CAL_FAIL_STAT[0]);
	g_GT_code = mmio_read_32(&g_ddrphy_reg->CAL_GT_VWMC[0]);
	for(slice = 0; slice < max_slice; slice++)
		gate_center[slice] = g_GT_code >> (slice*8) & 0xFF;

	g_GT_cycle = mmio_read_32(&g_ddrphy_reg->CAL_GT_CYC);
	for(slice = 0; slice < max_slice; slice++)
		gate_cycle[slice] = g_GT_cycle >> (slice*3) & 0x7;

	lock_value = (mmio_read_32(&g_ddrphy_reg->MDLL_CON[1]) >> 8) & 0x1FF;

	MEMMSG("\r\n####### Gate Leveling - Information #######\r\n");

	MEMMSG("Gate Leveling %s!! \r\n", status ? "Failed" : "Success");
	MEMMSG("Gate Level Center    : %2d, %2d, %2d, %2d\r\n",
		gate_center[0], gate_center[1], gate_center[2], gate_center[3]);
	MEMMSG("Gate Level Cycle     : %d, %d, %d, %d\r\n",
		gate_cycle[0], gate_cycle[1], gate_cycle[2], gate_cycle[3]);
	MEMMSG("Gate Delay           : %d, %d, %d, %d\r\n",
		(gate_cycle[0])*lock_value + gate_center[0],
		(gate_cycle[1])*lock_value + gate_center[1],
		(gate_cycle[2])*lock_value + gate_center[2],
		(gate_cycle[3])*lock_value + gate_center[3]);
	MEMMSG("###########################################\r\n");
}
#endif

#if (DDR_GATE_LEVELING_EN == 1)
/*************************************************************
 * Must be S5P6818
 * Gate Leveling sequence in S5P6818
 * must go through the following steps:
 *
 * Step 01. Send ALL Precharge command.
 * Step 02. Set the Memory in MPR Mode (MR3:A2=1)
 * Step 03. Set the Gate Leveling Mode.
 *	    -> Enable "gate_cal_mode" in PHY_CON2[24]
 *	    -> Enable "ctrl_shgate" in PHY_CON0[8]
 *	    -> Set "ctrl_gateduradj[3:0] (=PHY_CON1[23:20]) (DDR3: 4'b0000")
 * Step 04. Waiting for Response.
 *	    -> Wait until "rd_wr_cal_resp"(=PHYT_CON3[26])
 * Step 05.  End the Gate Leveling
 *	     -> Disable "gate_lvl_start(=PHY_CON3[18]"
 *	         after "rd_wr_cal_resp"(=PHY_CON3)is disabled.
 * Step 06. Disable DQS Pull Down Mode.
 *	     -> Set the "ctrl_pulld_dqs[8:0] = 0"
 * Step 07. Step 07. Disable the Memory in MPR Mode (MR3:A2=0)
 *************************************************************/
int ddr_gate_leveling(void)
{
	union SDRAM_MR MR;

	volatile int cal_count = 0;
	volatile int response;
	int ret = 0;

	MEMMSG("\r\n########## Gate Leveling - Start ##########\r\n");

	/* Step 01. Send ALL Precharge command. */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

	/* Step 02. Set the Memory in MPR Mode (MR3:A2=1) */
	MR.Reg = 0;
	MR.MR3.MPR = 1;
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);

	/* Step 03. Set the Gate Leveling Mode. */
	/* Step 03-1. Enable "gate_cal_mode" in PHY_CON2[24] */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[2], (0x1 << 24));			// gate_cal_mode[24] = 1
	/* Step 03-2. Enable "ctrl_shgate" in PHY_CON0[8] */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[0], (0x5 <<  6));			// ctrl_shgate[8]=1, ctrl_atgate[6]=1
	/* Step 03-3. Set "ctrl_gateduradj[3:0] (=PHY_CON1[23:20]) (DDR3: 4'b0000") */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[1], (0xF << 20));			// ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9

	/* Step 04. Wait for Response */
	mmio_write_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 18));			// gate_lvl_start[18] = 1
	for (cal_count = 0; cal_count < 100; cal_count++) {
		response = mmio_read_32(&g_ddrphy_reg->PHY_CON[3]);
		if (response & (0x1 << 26))
			break;
		DMC_Delay(100);
	}
	/* Step 05. End the Gate Leveling */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 18));			// gate_lvl_start[18]=0 : Stopping it after completion of GATE leveling.
	if (cal_count == 100) {
		MEMMSG("Gate: Calibration Responese Checking : fail...!!!\r\n");
		ret = -1; // Failure Case
		goto gate_err_ret;
	}
	g_GT_code = mmio_read_32(&g_ddrphy_reg->CAL_GT_VWMC[0]);
	g_GT_cycle = mmio_read_32(&g_ddrphy_reg->CAL_GT_CYC);

#if (MEM_CALIBRATION_INFO == 1)
	gate_leveling_information();
#endif

gate_err_ret:
	/* Step 06. Set the PHY for dqs pull down mode (Disable) */
	mmio_write_32(&g_ddrphy_reg->LP_CON, 0x0);				// ctrl_pulld_dqs[8:0] = 0

	/* Step 07. Disable the Memory in MPR Mode (MR3:A2=0) */
	MR.Reg = 0;
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);

	MEMMSG("\r\n########## Gate Leveling - End ##########\r\n");

	if (g_GT_code == 0x08080808)
		ret = -1;

	return ret;
}
#endif // #if (DDR_GATE_LEVELING_EN == 1)

#if ((DDR_READ_DQ_CALIB_EN == 1) && (MEM_CALIBRATION_INFO == 1))
void read_dq_calibration_information(void)
{
	unsigned int dq_fail_status, dq_calibration;
	unsigned int vwml[4], vwmr[4], vwmc[4];
//	unsiged int Deskew[4];
#if (MEM_CALIBRATION_BITINFO == 1)
	unsigned char bit_vwml[32], bit_vwmr[32];
	unsigned char bit_vwmc[32], bit_deskew[32];
	unsigned int max_bit_line = 8, bit_line;
#endif

#if (DM_CALIBRATION_INFO == 1)
	unsigned int DM_VWML[4], DM_VWMR[4], DM_VWMC[4];
#endif
	unsigned int max_slice = 4, slice;

	/* Check whether each slice by failure. */
	for(slice = 0; slice < max_slice; slice++) {
		dq_fail_status = (mmio_read_32(&g_ddrphy_reg->CAL_FAIL_STAT[0]) >> (slice*8)) & 0xF;
		if (dq_fail_status == 1)
			break;
	}

	if (dq_fail_status == 0) {
		/* Vaile Window Margin Left */
		dq_calibration = mmio_read_32(&g_ddrphy_reg->CAL_RD_VWML[0]);
		for(slice = 0; slice < max_slice; slice++)
			vwml[slice] = (dq_calibration >> (slice*8)) & 0xFF;

		/* Vaile Window Margin Right */
		dq_calibration = mmio_read_32(&g_ddrphy_reg->CAL_RD_VWMR[0]);
		for(slice = 0; slice < max_slice; slice++)
			vwmr[slice] = (dq_calibration >> (slice*8)) & 0xFF;

		/* Vaile Window Margin Center */
		dq_calibration = mmio_read_32(&g_ddrphy_reg->CAL_RD_VWMC[0]);
		for(slice = 0; slice < max_slice; slice++)
			vwmc[slice] = (dq_calibration >> (slice*8)) & 0xFF;
#if (MEM_CALIBRATION_BITINFO == 1)
		/* Check correction value for each slice for each bit line. */
		for(slice = 0; slice < max_slice; slice++) {
			/*  */
			for(bit_line = 0; bit_line < max_bit_line; bit_line++) {
				bit_deskew[bit_line] = (mmio_read_32(&g_ddrphy_reg->RD_DESKEW_CON[slice*3]) >> (8*0)) & 0xFF;
				bit_vwmc[bit_line] = (mmio_read_32(&g_ddrphy_reg->VWMC_STAT[slice*3]) >> (8*1)) & 0xFF;
				bit_vwml[bit_line] = (mmio_read_32(&g_ddrphy_reg->VWML_STAT[slice*3]) >> (8*2)) & 0xFF;
				bit_vwmr[bit_line] = (mmio_read_32(&g_ddrphy_reg->VWMR_STAT[slice*3]) >> (8*3)) & 0xFF;

				bit_deskew[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->RD_DESKEW_CON[slice*3+1]) >> (8*0)) & 0xFF;
				bit_vwmc[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->VWMC_STAT[slice*3+1]) >> (8*1)) & 0xFF;
				bit_vwml[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->VWML_STAT[slice*3+1]) >> (8*2)) & 0xFF;
				bit_vwmr[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->VWMR_STAT[slice*3+1]) >> (8*3)) & 0xFF;
			}
		}
#endif

#if (DM_CALIBRATION_INFO == 1)
		/* DM Vaile Window Margin Left */
		DM_VWML[slice] = mmio_read_32(&g_ddrphy_reg->CAL_DM_VWML[0]);
		for(slice = 0; slice < max_slice; slice++)
			DM_VWML[slice] = (dq_calibration >> (slice*8)) & 0xFF;
		/* DM Vaile Window Margin Right */
		DM_VWMR[slice] = mmio_read_32(&g_ddrphy_reg->CAL_DM_VWMR[0]);
		for(slice = 0; slice < max_slice; slice++)
			DM_VWMR[slice] = (dq_calibration >> (slice*8)) & 0xFF;
		/* DM Vaile Window Margin Center */
		DM_VWMC[slice] = mmio_read_32(&g_ddrphy_reg->CAL_DM_VWMC[0]);
		for(slice = 0; slice < max_slice; slice++)
			DM_VWMC[slice] = (dq_calibration >> (slice*8)) & 0xFF;
#endif
	}

	MEMMSG("\r\n#### Read DQ Calibration - Information ####\r\n");

	MEMMSG("Read DQ Calibration %s!! \r\n",
			(dq_fail_status == 0) ? "Success" : "Failed" );
	if (dq_fail_status == 0) {
		unsigned int range;
		for(slice = 0; slice < max_slice; slice++) {
			range = vwmr[slice] - vwml[slice];
			MEMMSG("SLICE%02d: %02d ~ %02d ~ %02d (range: %d) \r\n",
					slice, vwml[slice], vwmc[slice], vwmr[slice], range);
		}
#if (MEM_CALIBRATION_BITINFO == 1)
		MEMMSG("     \tLeft\tCenter\tRight\tDeSknew \r\n");
		for(slice = 0; slice < max_slice; slice++) {
			for(bit_line = 0; bit_line < max_bit_line; bit_line++) {
				unsigned int line_num = (slice*max_bit_line) + bit_line;
				MEMMSG("DQ%02d: \t%02d\t\t%02d\t%02d\t%02d\r\n",
						line_num, bit_vwml[line_num], bit_vwmc[line_num],
						bit_vwmr[line_num], bit_deskew[line_num]);
			}
		}
#endif

#if (DM_CALIBRATION_INFO == 1)
		MEMMSG("[DM] \tLeft\tCenter\tRight\tDeSknew \r\n");
		for(slice = 0; slice < max_slice; slice++) {
			MEMMSG("SLICE%02d: %d ~ %d ~ %d (range: %d) \r\n",
					slice, (DM_VWML[slice]>>(8*slice))&0xFF,
					(DM_VWMC[slice]>>(8*slice))&0xFF, (DM_VWMR[slice]>>(8*slice))&0xFF);
		}
#endif
		MEMMSG("###########################################\r\n");
	} // if (dq_fail_status == 0)
}
#endif

#if (DDR_READ_DQ_CALIB_EN == 1)
/*************************************************************
 * Must be S5P6818
 * Read DQ Calibration sequence in S5P6818
 * must go through the following steps:
 *
 * Step 01. Send Precharge ALL Command
 * Step 02. Set the Memory in MPR Mode (MR3:A2=1)
 * Step 03. Set Read Leveling Mode.
 * 	     -> Enable "rd_cal_mode" in PHY_CON2[25]
 * Step 04. Start the Read DQ Calibration
 *	     -> Enable "rd_cal_start"(=PHY_CON3[19]) to do rad leveling.
 * Step 05. Wait for Response.
 *	     -> Wait until "rd_wr_cal_resp"(=PHY_CON3[26]) is set.
 * Step 06. End the Read DQ Calibration
 *	     -> Set "rd_cal_start=0"(=PHY_CON3[19]) after
 	          "rd_wr_cal_resp"(=PHY_CON3[26]) is enabled.
 * Step 07. Disable the Memory in MPR Mode (MR3:A2=0)
 *************************************************************/
int ddr_read_dq_calibration(void)
{
	union SDRAM_MR MR;

	volatile int cal_count = 0;
	volatile int status, response;
	int ret = 0;

	MEMMSG("\r\n########## Read DQ Calibration - Start ##########\r\n");

#if (CFG_8BIT_DESKEW == 1)
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[0], (0x1 << 13));			// byte_rdlvl_en[13]=0, for Deskewing
#endif

	/* Step 01. Send Precharge ALL Command */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

	/* Step 02. Set the Memory in MPR Mode (MR3:A2=1) */
	MR.Reg = 0;
	MR.MR3.MPR = 1;
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);

	/* Step 03. Set Read Leveling Mode. */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[2], (0x1 << 25));			// rd_cal_mode[25]=1

	/* Step 04. Statr the Read DQ Calibration */
	mmio_write_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 19));			// rd_cal_start[19]=1 : Starting READ calibration

	/* Step 05. Wait for Response. (check value : 1) */
	for (cal_count = 0; cal_count < 100; cal_count++) {
		response = mmio_read_32(&g_ddrphy_reg->PHY_CON[3]);
		if (response & (0x1 << 26))
			break;
		DMC_Delay(100);
	}

	/* Step 06. End the Read DQ Calibration */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 19));			// rd_cal_start[19]=0

	/* Step XX-0. check to success or failed (timeout) */
	if (cal_count == 100) { // Failure Case
		MEMMSG("RD DQ CAL Status Checking error\r\n");
		ret = -1;
		goto rd_err_ret;
	}

	/* Step XX-1. check to success or failed (status) */
	status = mmio_read_32(&g_ddrphy_reg->CAL_FAIL_STAT[0]);			//dq_fail_status[31:0] : Slice 0 ~Slice3
	if (status != 0) {
		MEMMSG("Read DQ Calibration Status: 0x%08X \r\n", status);
		ret = -1;
		goto rd_err_ret;
	}
	g_RD_vwmc = mmio_read_32(&g_ddrphy_reg->CAL_RD_VWMC[0]);

rd_err_ret:
	/* Step 07. Disable the Memory in MPR Mode (MR3:A2=0) */
	MR.Reg = 0;
	send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);

#if (MEM_CALIBRATION_INFO == 1)
	read_dq_calibration_information();
#endif
	MEMMSG("\r\n########## Read DQ Calibration - End ##########\r\n");

	return ret;
}
#endif // #if (DDR_READ_DQ_CALIB_EN == 1)

#if ((DDR_WRITE_LEVELING_EN == 1) && (MEM_CALIBRATION_INFO == 1))
void write_latency_information(void)
{
//	unsigned int latency;
	unsigned int latency_plus;
	unsigned int status;

	unsigned int max_slice = 4, slice;

	status = mmio_read_32(&g_ddrphy_reg->CAL_WL_STAT) & 0xF;
	if (status != 0) {
//		latency = (mmio_read_32(&g_ddrphy_reg->PHY_CON[4]) >> 16) & 0x1F;
		latency_plus = (mmio_read_32(&g_ddrphy_reg->PHY_CON[5]) >> 0) & 0x7;

		for (slice = 0; slice < max_slice; slice++)
			MEMMSG("[SLICE%02d] Write Latency Cycle : %d\r\n",
				slice, latency_plus >> (slice*3) & 0x7);
		MEMMSG("0: Half Cycle, 1: One Cycle, 2: Two Cycle \r\n");
	}

	MEMMSG("Write Latency Calibration %s(ret=0x%08X)!! \r\n",
			(status == 0xF) ? "Success" : "Failed", status);
}
#endif // #if ((DDR_WRITE_LEVELING_EN == 1) && (MEM_CALIBRATION_INFO == 1))

#if (DDR_WRITE_LEVELING_EN == 1)
/*************************************************************
 * Must be S5P6818
 * Write Latency Calibration sequence in S5P6818
 * must go through the following steps:
 *
 * Step 01. Set Write Latency(=ctrl_wrlat) before Write Latency Calibration.
 * Step 02. Set issue Active command.
 * Step 03. Set the colum address.
 * Step 04. Set the Write Latency Calibration Mode & Start
 *	     - Set the "wl_cal_mode=1 (=PHY_CON3[20])"
 *	     - Set the "wl_cal_start=1 (=PHY_CON3[21])"
 * Step 05.  Start Write Leveling.
 *	     - Set the "wrlvel_start = 1'b1" (=PHY_CON3[16])
 * Step 06. Wait until the for Write Latency Calibtion complete.
 *	     - Wait until "wl_cal_resp" (=PHY_CON3[27])
 * Step 07. Check the success or not.
 * Step 08. Check the success or not.
 *	     -> Read Status (=CAL_WL_STAT)
 *************************************************************/
int ddr_write_latency_calibration(void)
{
//	volatile int bank = 0;
	volatile int row = 0, column = 0;
	volatile int cal_count;
	volatile int response, done_status = 0;
	int ret = 0;

	MEMMSG("\r\n########## Write Latency Calibration - Start ##########\r\n");

#if (CFG_8BIT_DESKEW == 1)
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[0], (0x1 << 13));			// byte_rdlvl_en[13]=1
#endif

#if 0	/* Step 01. Set Write Latency(=ctrl_wrlat) before Write Latency Calibration.*/
	int DDR_AL = 0, DDR_WL, DDR_RL;
	if (MR1_nAL > 0)
		DDR_AL = nCL - MR1_nAL;

	DDR_WL = (DDR_AL + nCWL);
	DDR_RL = (DDR_AL + nCL);
	DDR_RL = DDR_RL;
	mmio_set_32(&g_ddrphy_reg->PHY_CON[4], (DDR_WL << 16));
#endif // Step 00.

	/* Step 02. Set issue Active command. */
	mmio_write_32(&g_drex_reg->WRTRA_CONFIG,
			(row    << 16) |					// [31:16] row_addr
			(0x0    <<  1) |					// [ 3: 1] bank_addr
			(0x1    <<  0));					// [    0] write_training_en
	mmio_clear_32(&g_drex_reg->WRTRA_CONFIG, (0x1 <<  0));			// [   0]write_training_en[0] = 0

	/* Step 03. Set the colum address*/
	mmio_set_32  (&g_ddrphy_reg->LP_DDR_CON[2], (column <<  1));		// [15: 1] ddr3_address

	/* Step 04. Set the Write Latency Calibration Mode & Start */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[3], (0x1 << 20));			// wl_cal_mode[20] = 1
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[3], (0x1 << 21));			// wl_cal_start[21] = 1

	/* Step 05. Wait until the for Write Latency Calibtion complete. */
	for (cal_count = 0; cal_count < 100; cal_count++) {
		response = mmio_read_32( &g_ddrphy_reg->PHY_CON[3] );
		if ( response & (0x1 << 27) )					// wl_cal_resp[27] : Wating until WRITE LATENCY calibration is complete
			break;
		DMC_Delay(0x100);
	}

	/* Step 06. After the completion Write Latency Calibration and clear. */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 21));			// wl_cal_start[21] = 0

	/* Step 07. Check the success or not. */
	if (cal_count == 100) {                                                 // Failure Case
		MEMMSG("WR Latency CAL Status Checking error\r\n");
		ret = -1;
	}

	/* Step XX.  Check the Write Latency Information. */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[3], (0x1 << 0));	   		// reg_mode[7:0]=0x1

#if (MEM_CALIBRATION_INFO == 1)
	write_latency_information();
#endif
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[3], (0xFF << 0));			// reg_mode[7:0]=0x0

	/* Step 08. Check the success or not. (=CAL_WL_STAT) */
	done_status = mmio_read_32(&g_ddrphy_reg->CAL_WL_STAT) & 0xF;
	if (done_status != 0xF) {
		MEMMSG("Write Latency Calibration Not Complete!! (ret=%08X) \r\n",
			done_status);
		ret = -1;
	}

	MEMMSG("\r\n########## Write Latency Calibration - End ##########\r\n");

	return ret;
}
#endif // #if (DDR_WRITE_LEVELING_EN == 1)

#if ((DDR_WRITE_DQ_CALIB_EN == 1) && (MEM_CALIBRATION_INFO == 1))
void write_dq_calibration_information(void)
{
	unsigned int dq_fail_status, dq_calibration;
	unsigned int vwml[4], vwmr[4], vwmc[4];
//	unsigned int Deskew[4];
#if (MEM_CALIBRATION_BITINFO == 1)
	unsigned char bit_vwml[32], bit_vwmr[32];
	unsigned char bit_vwmc[32], bit_deskew[32];
	unsigned int max_bit_line = 8, bit_line;
#endif

#if (DM_CALIBRATION_INFO == 1)
	unsigned int DM_VWML[4], DM_VWMR[4], DM_VWMC[4];
#endif
	unsigned int max_slice = 4, slice;

	/* Check whether each slice by failure. */
	for(slice = 0; slice < max_slice; slice++) {
		dq_fail_status = (mmio_read_32(&g_ddrphy_reg->CAL_FAIL_STAT[0]) >> (slice*8)) & 0xF;
		if (dq_fail_status == 1)
			break;
	}

	if (dq_fail_status == 0) {
		/* Vaile Window Margin Left */
		dq_calibration = mmio_read_32(&g_ddrphy_reg->CAL_WR_VWML[0]);
		for(slice = 0; slice < max_slice; slice++)
			vwml[slice] = (dq_calibration >> (slice*8)) & 0xFF;

		/* Vaile Window Margin Right */
		dq_calibration = mmio_read_32(&g_ddrphy_reg->CAL_WR_VWMR[0]);
		for(slice = 0; slice < max_slice; slice++)
			vwmr[slice] = (dq_calibration >> (slice*8)) & 0xFF;

		/* Vaile Window Margin Center */
		dq_calibration = mmio_read_32(&g_ddrphy_reg->CAL_WR_VWMC[0]);
		for(slice = 0; slice < max_slice; slice++)
			vwmc[slice] = (dq_calibration >> (slice*8)) & 0xFF;
#if (MEM_CALIBRATION_BITINFO == 1)
		/* Check correction value for each slice for each bit line. */
		for(slice = 0; slice < max_slice; slice++) {
			/*  */
			for(bit_line = 0; bit_line < max_bit_line; bit_line++) {
				bit_deskew[bit_line] = (mmio_read_32(&g_ddrphy_reg->WR_DESKEW_CON[slice*3]) >> (8*0)) & 0xFF;
				bit_vwmc[bit_line] = (mmio_read_32(&g_ddrphy_reg->VWMC_STAT[slice*3]) >> (8*1)) & 0xFF;
				bit_vwml[bit_line] = (mmio_read_32(&g_ddrphy_reg->VWML_STAT[slice*3]) >> (8*2)) & 0xFF;
				bit_vwmr[bit_line] = (mmio_read_32(&g_ddrphy_reg->VWMR_STAT[slice*3]) >> (8*3)) & 0xFF;

				bit_deskew[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->WR_DESKEW_CON[slice*3+1]) >> (8*0)) & 0xFF;
				bit_vwmc[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->VWMC_STAT[slice*3+1]) >> (8*1)) & 0xFF;
				bit_vwml[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->VWML_STAT[slice*3+1]) >> (8*2)) & 0xFF;
				bit_vwmr[bit_line+4] = (mmio_read_32(&g_ddrphy_reg->VWMR_STAT[slice*3+1]) >> (8*3)) & 0xFF;
			}
		}
#endif

#if (DM_CALIBRATION_INFO == 1)
		/* DM Value Window Margin Left */
		DM_VWML[slice] = mmio_read_32(&g_ddrphy_reg->CAL_DM_VWML[0]);
		for(slice = 0; slice < max_slice; slice++)
			DM_VWML[slice] = (dq_calibration >> (slice*8)) & 0xFF;
		/* DM Value Window Margin Right */
		DM_VWMR[slice] = mmio_read_32(&g_ddrphy_reg->CAL_DM_VWMR[0]);
		for(slice = 0; slice < max_slice; slice++)
			DM_VWMR[slice] = (dq_calibration >> (slice*8)) & 0xFF;
		/* DM Value Window Margin Center */
		DM_VWMC[slice] = mmio_read_32(&g_ddrphy_reg->CAL_DM_VWMC[0]);
		for(slice = 0; slice < max_slice; slice++)
			DM_VWMC[slice] = (dq_calibration >> (slice*8)) & 0xFF;
#endif
	}

	MEMMSG("\r\n### Write DQ Calibration - Information ####\r\n");

	MEMMSG("Write DQ Calibration %s!! \r\n",
			(dq_fail_status == 0) ? "Success" : "Failed" );
	if (dq_fail_status == 0) {
		unsigned int range;
		for(slice = 0; slice < max_slice; slice++) {
			range = vwmr[slice] - vwml[slice];
			MEMMSG("SLICE%02d: %d ~ %d ~ %d (range: %d) \r\n",
					slice, vwml[slice], vwmc[slice], vwmr[slice], range);
		}
#if (MEM_CALIBRATION_BITINFO == 1)
		MEMMSG("     \tLeft\tCenter\tRight\tDeSknew \r\n");
		for(slice = 0; slice < max_slice; slice++) {
			for(bit_line = 0; bit_line < max_bit_line; bit_line++) {
				unsigned int line_num = (slice*max_bit_line) + bit_line;
				MEMMSG("DQ%02d: \t%d\t\t%d\t%d\t%d\r\n",
						line_num, bit_vwml[line_num], bit_vwmc[line_num],
						bit_vwmr[line_num], bit_deskew[line_num]);
			}
		}
#endif

#if (DM_CALIBRATION_INFO == 1)
		MEMMSG("[DM] \tLeft\tCenter\tRight\tDeSknew \r\n");
		for(slice = 0; slice < max_slice; slice++) {
			MEMMSG("SLICE%02d: %d ~ %d ~ %d (range: %d) \r\n",
					slice, (DM_VWML[slice]>>(8*slice))&0xFF,
					(DM_VWMC[slice]>>(8*slice))&0xFF, (DM_VWMR[slice]>>(8*slice))&0xFF);
		}
#endif
		MEMMSG("###########################################\r\n");
	} // if (dq_fail_status == 0)
}
#endif // #if ((DDR_WRITE_DQ_CALIB_EN == 1) && (MEM_CALIBRATION_INFO == 1))

#if (DDR_WRITE_DQ_CALIB_EN == 1)
/*************************************************************
 * Must be S5P6818
 * Write DQ Calibration sequence in S5P6818
 * must go through the following steps:
 *
 * Step 01. Set Write Latency(=ctrl_wrlat) before Write Latency Calibration.
 * Step 02. Set issue Active command.
 * Step 03. Set the colum address
 * Step 04. Write DQ Calibration (Unit:Byte/Bit), (Pattern)
 *	     - Set "PHY_CON1[15:0]=0x0100" and "byte_rdlvl_en=1(=PHY_CON0[13]).
 *	     - Set "PHY_CON1[15:0]=0xFF00" and "byte_rdlvl_en=0(=PHY_CON0[13]).
 *                for Deskewing.
 * Step 05. Set Write Training Mode.
 *	     -> Set "wr_cal_mode=1"(=PHY_CON2[26]).
 * Step 06. Write DQ Calibration Start
 *	     -> Set "wr_cal_start=1" in PHY_CON2[27] to do Write DQ Calibration
 * Step 07. Waiting for Response
 *	     -> Wait until "rd_wr_cal_resp(=PHY_CON3[26]" is set.
 * Step 08. End the Write DQ Calibration
 *	     -> Set "wr_cal_start=0" in PHY_CON2[27] to do Write DQ Calibration
 	         after wrwr_cal_resp(=PHY_CON3[26]" is set.
 * Step 09. Check to Success or Failed. (timeout & status)
 *************************************************************/
int ddr_write_dq_calibration(void)
{
	volatile int bank = 0, row = 0, column = 0;
	volatile int cal_count = 0;
	volatile int status, response;
	int ret = 0;

	/* Step XX. Send All Precharge Command */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
	MEMMSG("\r\n########## Write DQ Calibration - Start ##########\r\n");

#if (CFG_8BIT_DESKEW == 1)
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[0], (0x1 << 13));          // byte_rdlvl_en[13]=0, for Deskewing
#endif

#if 0	/* Step 01. Set Write Latency(=ctrl_wrlat) before Write Latency Calibration.*/
	int DDR_AL = 0, DDR_WL, DDR_RL;
	if (MR1_nAL > 0)
		DDR_AL = nCL - MR1_nAL;

	DDR_WL = (DDR_AL + nCWL);
	DDR_RL = (DDR_AL + nCL);
	DDR_RL = DDR_RL;
	mmio_set_32(&g_ddrphy_reg->PHY_CON[4], (DDR_WL << 16));
#endif //

	/* Step 02. Set issue Active command. */
	mmio_write_32(&g_drex_reg->WRTRA_CONFIG,
			(row  << 16) |						// [31:16] row_addr
			(bank <<  1) |						// [ 3: 1] bank_addr
			(0x1  <<  0));						// [    0] write_training_en
	mmio_clear_32(&g_drex_reg->WRTRA_CONFIG, (0x1 <<  0));			// [   0]write_training_en[0] = 0

	/* Step 03. Set the colum address */
	mmio_set_32  (&g_ddrphy_reg->LP_DDR_CON[2],
			(0x0    << 16) |					// [31:16] ddr3_default
			(column <<  1) |					// [15: 1] ddr3_address
			(0x0    <<  0));					// [    0] ca_swap_mode
#if 0
	/* Step 04-0. Set "PHY_CON1[15:0]=0x0100" and "byte_rdlvl_en=1(=PHY_CON0[13]). */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[1], 0x0100);
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[0], (1 << 13));
#else
	/* Step 04-1. Set "PHY_CON1[15:0]=0xFF00" and "byte_rdlvl_en=0(=PHY_CON0[13]). */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[1], 0xFF00);
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[0], (1 << 13));
#endif

	/* Step 05. Set Write Training Mode  */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[2], (0x1 << 26));			// wr_cal_mode[26] = 1, Write Training mode
	/* Step 06. Write DQ Calibration Start */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[2], (0x1 << 27));			// wr_cal_start[27] = 1

	/* Step 07. Waiting for Response */
	for (cal_count = 0; cal_count < 100; cal_count++) {
		response = mmio_read_32(&g_ddrphy_reg->PHY_CON[3]);
		if (response & (0x1 << 26))					// rd_wr_cal_resp[26] : Wating until WRITE calibration is complet
			break;
		DMC_Delay(0x100);
	}

	/* Step 09. End the Write DQ Calibration */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[2], (0x1 << 27));			// wr_cal_start[27] = 0

	/* Step XX-0. check to success or failed (timeout) */
	if (cal_count == 100) {							// Failure Case
		MEMMSG("WR DQ CAL Status Checking error\r\n");
		ret = -1;
		goto wr_err_ret;
	}

	/* Step XX-1. check to success or failed (status) */
	status = mmio_read_32(&g_ddrphy_reg->CAL_FAIL_STAT[0]);			//dq_fail_status[31:0] : Slice 0 ~Slice3
	if (status != 0) {
		MEMMSG("Write DQ Calibration Status: 0x%08X \r\n", status);
		ret = -1;
		goto wr_err_ret;
	}
	g_WR_vwmc = mmio_read_32(&g_ddrphy_reg->CAL_WR_VWMC[0]);

wr_err_ret:
	/* Step 10. Display Write DQ Clibration information*/
#if (MEM_CALIBRATION_INFO == 1)
	write_dq_calibration_information();
#endif

	MEMMSG("\r\n########## Write DQ Calibration - End ##########\r\n");

	return ret;
}
#endif // #if (DDR_WRITE_DQ_CALIB_EN == 0)

#endif // #if (SKIP_LEVELING_TRAINING == 0)

static int resetgen_sequence(void)
{
	int retry = 0x10;

	/* Step 01. Reset (DPHY, DREX, DRAM)  (Min: 10ns, Typ: 200us) */
	do {
#if 0
		reset_con(RESETINDEX_OF_DREX_MODULE_CRESETn, CTRUE);
		reset_con(RESETINDEX_OF_DREX_MODULE_ARESETn, CTRUE);
		reset_con(RESETINDEX_OF_DREX_MODULE_nPRST,   CTRUE);
		DMC_Delay(0x100);                           // wait 300ms
		reset_con(RESETINDEX_OF_DREX_MODULE_CRESETn, CFALSE);
		reset_con(RESETINDEX_OF_DREX_MODULE_ARESETn, CFALSE);
		reset_con(RESETINDEX_OF_DREX_MODULE_nPRST,   CFALSE);
		DMC_Delay(0x1000);                          // wait 300ms

		reset_con(RESETINDEX_OF_DREX_MODULE_CRESETn, CTRUE);
		reset_con(RESETINDEX_OF_DREX_MODULE_ARESETn, CTRUE);
		reset_con(RESETINDEX_OF_DREX_MODULE_nPRST,   CTRUE);
		DMC_Delay(0x100);                           // wait 300ms
		reset_con(RESETINDEX_OF_DREX_MODULE_CRESETn, CFALSE);
		reset_con(RESETINDEX_OF_DREX_MODULE_ARESETn, CFALSE);
		reset_con(RESETINDEX_OF_DREX_MODULE_nPRST,   CFALSE);
		DMC_Delay(0x1000);                          // wait 300ms
		DMC_Delay(0xF000);
#else
		mmio_clear_32(&g_rstgen_reg->regrst[0], (0x7 << 26));
		DMC_Delay(0x1000);						// wait 300ms
		mmio_set_32(&g_rstgen_reg->regrst[0], (0x7 << 26));
		DMC_Delay(0x1000);						// wait 300ms
		mmio_clear_32(&g_rstgen_reg->regrst[0], (0x7 << 26));
		DMC_Delay(0x1000);						// wait 300ms
		mmio_set_32(&g_rstgen_reg->regrst[0], (0x7 << 26));
	//	DMC_Delay(0x10000);						// wait 300ms
#if 0
		mmio_clear_32(&pReg_Tieoff->TIEOFFREG[3], (0x1 << 31));
		DMC_Delay(0x1000);                              // wait 300ms
		mmio_set_32  (&pReg_Tieoff->TIEOFFREG[3], (0x1 << 31));
		DMC_Delay(0x1000);                              // wait 300ms
		mmio_clear_32(&pReg_Tieoff->TIEOFFREG[3], (0x1 << 31));
		DMC_Delay(0x1000);                              // wait 300ms
		mmio_set_32  (&pReg_Tieoff->TIEOFFREG[3], (0x1 << 31));
#endif
		DMC_Delay(0x10000);			// wait 300ms
#endif
		/* Step 01-1. Check the Reset State (Phy Version) */
	} while ((mmio_read_32(&g_ddrphy_reg->SHIFTC_CON) != 0x0492) && (retry--));

	MEMMSG("PHY Version: 0x%08X\r\n", mmio_read_32(&g_ddrphy_reg->VERSION_INFO));

	if (retry <= 0)
		return -1;

	return 0;
}

/*************************************************************
 * Must be S5P6818
 * (DDR3) memory initialize sequence in S5P6818
 * must go through the following steps:
 *
 * Step 01. Select Memory Type (=PHY_CON0[12:11]).
 *	     - ctrl_ddr_mode= 2'b11 (LPDDR3), 2'b10 (LPDDR2), 2'b00 (DDR2), 2'b01 (DDR3)
 * Step 02. Set Read Latency(RL), Burst Length(BL), Write Latency(WL)
 * Step 03. Set Write Latency(WL), Read Latency(RL), Burts Length(BL)
 * Step 04. ZQ Calibration (please refer)
 *	     - Enable and Disable "zq_clk_div_en" in ZQ_CON0[18]
 *	     - Enable "zq_manual_str" in ZQ_CON0[1]
 *	     - Wait until "zq_cal_done" (ZQ_CON1[0]) is enabled.
 *	     - Disable "zq_manual_str" in ZQ_CON0[1]\
 * Step 05. Memory Controller should assert "dfi_init_start" from LOW to HIGH
 * Step 06. Memory Controller should wait until "dfi_init_complete" is set.
 *	     - DLL Lock will be processed.
 * Step 07. Enable DQS pull down mode
 *	     - Set "ctrl_pulld_dqs=9'h1FF" (=LP_CON0[8:0]) in case of using 72bit PHY.
 *	     - Please be careful that DQS pull down can be disabled only after Gate Leveling is done.
 * Step 08. Update DLL information
 * ---------------------------------------------------------------------------------
 *				Drex Controller
 * ---------------------------------------------------------------------------------
 * Step 09. Set the Memory Control and Phy Control0 (MemControl and PhyControl0)
 * Step 10. Memory Base Config 0/1 (if need) - DREX (Optional)
 * Step 11. Memory Config 0/1 (if need) - DREX (Optional)
 * Step 12. Set the Precharge Config and Power Down Config Register - DREX
 * Step 13. Set the Access Parmeters (AC) Timing.
 * Step 14. Set the Qos Control 0~15 and Qos Config 0 ~15 Registers. - DREX (Currently not is use)
 * Step 15. Confirm that after RESET# is de-asserted, 500us have passed before CKE
 *	     becomes active.
 * Step 16. Confirm that clocks(CK, CK#) needto be started and stabilzed for at least
 *	     10ns or 5tCK. (which is larger)
 * ---------------------------------------------------------------------------------
 * Step 17. Memory Controller should assert "dfi_ctrlupd_req" after "dfi_init_complete" is set
 *	     - Please keep "ctrl-initiated update" mode until finishing leveling and tranining.
 * Step XX. Start Memory Initialization by memory controller
 * ---------------------------------------------------------------------------------
 * Step 18. Send NOP Command using the DirectCmd Register to assert and to hold CKE to a logic high level.
 * Step XX. Wait for tXPR(max(5nCK,tRFC(min)10ns)) or set tXP to tXPR value before step 17.
 *	     if the system set tXP to tXPR, than the system must set tXP to proper value before
 *	     normal memory operation.
 * Step 19. Issue EMRS2 command using the DirectCmd register to program the operating parameters.
 * Step 20. Issue EMRS3 command using the DirectCmd register to program the operating parameters.
 * Step 21. Issue EMRS command using the DirectCmd register to program the enable the memory DLL.
 * 	     Issue MRS command using the DirectCmd register to program the enable the memory DLL.
 * Step 22. Issue EMRS2 command using the DirectCmd register to program the operating parameters
 *	     Without resetting the memory DLL.
 * Step 23. Issue a ZQINIT command using the DirectCmd register.
 * Step XX. if there are more external memory chips, perform steps 17 ~ 23 procedures for other
 *	     memory device.
 * ---------------------------------------------------------------------------------
 * 				Gate & Training (if need)
 * ---------------------------------------------------------------------------------
 * Step 24 - 26. Skip the following steps if Leveling and Tranining are not required. (Optional features)
 * 	    - Enable "ctrl_atgate" in PHY_CON0[6]
 *	    - Enable "p0_cmd_en" in PHY_CON0[14]
 *	    - Enable "InitDeskewEn" in PHY_CON2[6]
 *	    - Enable "byte_rdlvl_en" in PHY_CON0[13]
 * Step 28. The time to determine the VMP(Vaild Margin Period) Read Training
 *	    - Recommended that "rdlvl_pass_adj=4" in PHY_CON1[19:16]
 * Step 29. 				(When using DDR3)
 *	    - Set "cmd_active=14'h105E" as default value (=LP_DDR_CON3[13:0])
 *	    - Set "cmd_default=14'h107F" as default value (=LP_DDR_CON4[13:0])
 * Step 30. Recommand the "rdlvel_incr_adj=7'h01" for the best margin.
 * Step 31. DLL On/OFF and Read DLL Lock Value
 *	    - Read "ctrl_lock_value[8:0]" in MDLL_CON[16:8]
 *	    - Update "ctrl_force[8:0]" in MDLL_CON0[15:7] by the value of "ctrl_lock_value[8:0]
 * Step 32. DDR Controller Calibration
 *	    - Write Leveling (for Fly-by)
 *	    - Gate Leveling (only for 800Mhz)
 *	    - Read DQ Calibration
 *	    - Write Latency Calibration (Must Write Leveling)
 *	    - Write DQ Calibration
 * Step 33. DLL Turn On
 *	    - Set "ctrl_dll_on=1" (=MDLL_CON0[5])
 * Step 34. Deskew Code is update.
 *	    - DLLDeskewEn=1" (=PHY_CON2[12]) to compensate Voltage, Temperature
 	       variation during.
 * Step 35. Phy Update Mode (0: MC-Initiated, 1: PHY-Intiated)
 * 	    - Set "upd_mode=0" (=OFFSETD_CON0[28]) for PHY_Initiated Update.
 * 		- if Ctrl-Initiated Update is used set "upd_mode=1" (refer )
  * ---------------------------------------------------------------------------------
 * Step 36. Enable and Disable "ctrl_resync"(=OFFSETD_CON0[24]" to make sure
 *	     All SDLL is update.
 * Step 37. Disable PHY gateing control through PHY APB I/F
 *	     if necessary(ctrl_Atgate, refer to PHY manual)	 (Current not is use)
 * Step 38. Send PALL command
 * Step 39. Set the (Phy Control and Memory Control)
 * Step 40. Set the Controller Control
 * Step 41. Set the Clock Gating Control
 *************************************************************/
int ddr3_initialize(unsigned int is_resume)
{
	union SDRAM_MR MR0, MR1, MR2, MR3;
	unsigned int DDR_WL, DDR_RL;
	unsigned int DDR_AL1, DDR_AL2;
	unsigned int DDR3_LvlTr;
	unsigned int temp;

	MEMMSG("\r\nDDR3 POR Init Start\r\n");

	/* Nexell Step XX. Get DRAM Information. */
	get_dram_information((struct dram_device_info*)&g_ddr3_info);

	/* Step 01. Reset (DPHY, DREX, DRAM)  (Min: 10ns, Typ: 200us) */
	if (resetgen_sequence() < 0) {
		MEMMSG("(DPHY, DREX) Controller Reset Failed! \r\n");
		return -1;
	}

	DDR3_LvlTr = CONFIG_DDR3_LVLTR_EN;

	DDR_AL1 = 0;
	DDR_AL2 = 0;
	if (MR1_nAL > 0) {
		DDR_AL1 = nCL - MR1_nAL;
		DDR_AL2 = nCWL - MR1_nAL;
	}

	DDR_WL = (DDR_AL2 + nCWL);
	DDR_RL = (DDR_AL1 + nCL);

	/* temporary code according to suspend/resume policy. */
//	if (is_resume == 0) {
	{
		MR2.Reg		= 0;
		MR2.MR2.RTT_WR	= CONFIG_DRAM_MR2_RTT_WR;
		MR2.MR2.SRT	= 0;						// self refresh normal range
		MR2.MR2.ASR	= 0;						// auto self refresh disable

		MR2.MR2.CWL	= (nCWL - 5);

		MR3.Reg 	 = 0;
		MR3.MR3.MPR 	 = 0;
		MR3.MR3.MPR_RF 	 = 0;

		MR1.Reg		 = 0;
		MR1.MR1.DLL 	 = 0;						// 0: Enable, 1 : Disable

		MR1.MR1.AL	 = MR1_nAL;

		MR1.MR1.ODS1	 = (CONFIG_DRAM_MR1_ODS & 0x2 >> 1);
		MR1.MR1.ODS0	 = (CONFIG_DRAM_MR1_ODS & 0x1 >> 0);
		MR1.MR1.RTT_Nom2 = (CONFIG_DRAM_MR1_RTT_Nom & 0x4 >> 2);
		MR1.MR1.RTT_Nom1 = (CONFIG_DRAM_MR1_RTT_Nom & 0x2 >> 1);
		MR1.MR1.RTT_Nom0 = (CONFIG_DRAM_MR1_RTT_Nom & 0x1 >> 0);
		MR1.MR1.QOff	 = 0;
		MR1.MR1.WL	 = 0;

		if (nCL > 11)
			temp = ((nCL - 12) << 1) + 1;
		else
			temp = ((nCL - 4) << 1);

		MR0.Reg		= 0;
		MR0.MR0.BL	= 0;
		MR0.MR0.BT	= 1;
		MR0.MR0.CL0	= (temp & 0x1);
		MR0.MR0.CL1	= ((temp >> 1) & 0x7);
		MR0.MR0.DLL	= 1; // 1;
		MR0.MR0.WR	= MR0_nWR;

		MR0.MR0.PD	= 0; // 1;
	} // if (is_resume == 0)

	/* Step 02. Select Memory type : DDR3 */
	// Check DDR3 MPR data and match it to PHY_CON[1]??
	temp = ((0x17 << 24) |							// [28:24] T_WrWrCmd
		(0x1  << 22) |							// [23:22] ctrl_upd_mode. DLL Update control 0:always, 1: depending on ctrl_flock, 2: depending on ctrl_clock, 3: don't update
		(0x0  << 20) |							// [21:20] ctrl_upd_range
#if (tWTR == 3)									// 6 cycles
		(0x7  << 17) |							// [19:17] T_WrRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#elif(tWTR == 2)								// 4 cycles
		(0x6  << 17) |							// [19:17] T_WrRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#endif
		(0x0  << 16) |							// [   16] wrlvl_mode. Write Leveling Enable. 0:Disable, 1:Enable
		(0x0  << 14) |							// [   14] p0_cmd_en. 0:Issue Phase1 Read command during Read Leveling. 1:Issue Phase0
		(0x0  << 13) |							// [   13] byte_rdlvl_en. Read Leveling 0:Disable, 1:Enable
		(0x1  << 11) |							// [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3, 2:LPDDR2, 3:LPDDR3
		(0x1  << 10) |							// [   10] ctrl_wr_dis. Write ODT Disable Signal during Write Calibration. 0: not change, 1: disable
		(0x1  <<  9) |							// [    9] ctrl_dfdqs. 0:Single-ended DQS, 1:Differential DQS
//		(0x1  <<  8) |							// [    8] ctrl_shgate. 0:Gate signal length=burst length/2+N, 1:Gate signal length=burst length/2-1
		(0x1  <<  6) |							// [    6] ctrl_atgate
		(0x0  <<  4) |							// [    4] ctrl_cmosrcv
		(0x0  <<  3) |							// [    3] ctrl_twpre
		(0x0  <<  0));							// [ 2: 0] ctrl_fnc_fb. 000:Normal operation.

	mmio_write_32(&g_ddrphy_reg->PHY_CON[0], temp);

#if 0
	mmio_set_32  ( &g_ddrphy_reg->OFFSETD_CON, (0x1 << 28) );		// upd_mode[28]=1, DREX-initiated Update Mode
	mmio_clear_32( &g_ddrphy_reg->OFFSETD_CON, (0x1 << 28) );		// upd_mode[28]=0, PHY-initiated Update Mode
#endif

	temp = mmio_read_32(&g_ddrphy_reg->LP_DDR_CON[3]) & ~0x3FFF;
	temp |= 0x105E;								// cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
	mmio_write_32(&g_ddrphy_reg->LP_DDR_CON[3], temp);

	temp = mmio_read_32(&g_ddrphy_reg->LP_DDR_CON[4]) & ~0x3FFF;
	temp |= 0x107F;								// cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
	mmio_write_32(&g_ddrphy_reg->LP_DDR_CON[4], temp);

	MEMMSG("[DDR]Phy Initialize\r\n");

	/* Step 03. Set Write Latency(WL), Read Latency(RL), Burts Length(BL) */
	mmio_write_32(&g_ddrphy_reg->PHY_CON[4],
			(DDR_WL << 16) |	// [20:16] ctrl_wrlat		// T_wrdata_en (WL+1)
			(0x08  <<  8) |		// [12: 8] Burst Length(BL)
			(DDR_RL <<  0));	// [ 4: 0] Read Latency(RL), 800MHz:0xB, 533MHz:0x5

	/* Step 04. ZQ Calibration */
#if 0
	mmio_write_32( &g_ddrphy_reg->DRVDS_CON[0],      // 100: 48ohm, 101: 40ohm, 110: 34ohm, 111: 30ohm
		(PHY_DRV_STRENGTH_30OHM  <<  28) |      // [30:28] Data Slice 4
		(CONFIG_DPHY_DRVDS_BYTE3 <<  25) |	// [27:25] Data Slice 3
		(CONFIG_DPHY_DRVDS_BYTE2 <<  22) |	// [24:22] Data Slice 2
		(CONFIG_DPHY_DRVDS_BYTE1 <<  19) |	// [21:19] Data Slice 1
		(CONFIG_DPHY_DRVDS_BYTE0 <<  16) |	// [18:16] Data Slice 0
		(CONFIG_DPHY_DRVDS_CK    <<   9) |	// [11: 9] CK
		(CONFIG_DPHY_DRVDS_CKE   <<   6) |	// [ 8: 6] CKE
		(CONFIG_DPHY_DRVDS_CS    <<   3) |	// [ 5: 3] CS
		(CONFIG_DPHY_DRVDS_CA    <<   0));	// [ 2: 0] CA[9:0], RAS, CAS, WEN, ODT[1:0], RESET, BANK[2:0]

	mmio_write_32( &g_ddrphy_reg->DRVDS_CON[1],      // 100: 48ohm, 101: 40ohm, 110: 34ohm, 111: 30ohm
		(PHY_DRV_STRENGTH_30OHM <<  25) |       // [11: 9] Data Slice 8
		(PHY_DRV_STRENGTH_30OHM <<  22) |       // [ 8: 6] Data Slice 7
		(PHY_DRV_STRENGTH_30OHM <<  19) |       // [ 5: 3] Data Slice 6
		(PHY_DRV_STRENGTH_30OHM <<  16));       // [ 2: 0] Data Slice 5
#else
	mmio_write_32(&g_ddrphy_reg->DRVDS_CON[0], 0x00);
	mmio_write_32(&g_ddrphy_reg->DRVDS_CON[1], 0x00);
#endif

	// Driver Strength(zq_mode_dds), zq_clk_div_en[18]=Enable
	mmio_write_32(&g_ddrphy_reg->ZQ_CON, (0x1 << 27) |			// [   27] zq_clk_en. ZQ I/O clock enable.
		(CONFIG_DPHY_ZQ_DDS << 24) |					// [26:24] zq_mode_dds, Driver strength selection. 100 : 48ohm, 101 : 40ohm, 110 : 34ohm, 111 : 30ohm
		(CONFIG_DPHY_ZQ_ODT << 21) |					// [23:21] ODT resistor value. 001 : 120ohm, 010 : 60ohm,  011 : 40ohm, 100 : 30ohm
		(0x0 << 20) |							// [   20] zq_rgddr3. GDDR3 mode. 0:Enable, 1:Disable
		(0x0 << 19) |							// [   19] zq_mode_noterm. Termination. 0:Enable, 1:Disable
		(0x1 << 18) |							// [   18] zq_clk_div_en. Clock Dividing Enable : 0, Disable : 1
		(0x0 << 15) |							// [17:15] zq_force-impn
//		(0x7 << 12) |							// [14:12] zq_force-impp
		(0x0 << 12) |							// [14:12] zq_force-impp
		(0x30 << 4) |							// [11: 4] zq_udt_dly
		(0x1  << 2) |							// [ 3: 2] zq_manual_mode. 0:Force Calibration, 1:Long cali, 2:Short cali
		(0x0 << 1) |							// [    1] zq_manual_str. Manual Calibration Stop : 0, Start : 1
		(0x0 << 0));							// [    0] zq_auto_en. Auto Calibration enable

	mmio_set_32(&g_ddrphy_reg->ZQ_CON, (0x1 << 1));				// zq_manual_str[1]. Manual Calibration Start=1
	while ((mmio_read_32(&g_ddrphy_reg->ZQ_STATUS) & 0x1) == 0);		//- PHY0: wait for zq_done
	mmio_clear_32(&g_ddrphy_reg->ZQ_CON, (0x1 << 1));			// zq_manual_str[1]. Manual Calibration Stop : 0, Start : 1

	mmio_clear_32(&g_ddrphy_reg->ZQ_CON, (0x1 << 18));			// zq_clk_div_en[18]. Clock Dividing Enable: 1, Disable : 0

	/* Step 05. Assert "dfi_init_start" from Low to High */
	mmio_write_32(&g_drex_reg->CONCONTROL,
				(0xFFF << 16) |				// [27:16] time out level0
				(0x3	<< 12) |				// [14:12] read data fetch cycles - n cclk cycles (cclk: DREX core clock)
//				(0x1    <<   5) |					// [  : 5] afre_en. Auto Refresh Counter. Disable:0, Enable:1
//				(0x1    <<   4) |					// nexell: 0:ca swap bit, 4 & samsung drex/phy initiated bit
				(0x0   <<  1));					// [ 2: 1] aclk:cclk = 1:1

	mmio_set_32(&g_drex_reg->CONCONTROL, (0x1 << 28));			// dfi_init_start[28]. DFI PHY initialization start

	/* Step 06. Wait until "dfi_init_complete" is set. (DLL lock will be processed.) */
	while ((mmio_read_32(&g_drex_reg->PHYSTATUS) & (0x1 << 3)) == 0);	// dfi_init_complete[3]. wait for DFI PHY initialization complete
	mmio_clear_32(&g_drex_reg->CONCONTROL, (0x1 << 28));			// dfi_init_start[28]. DFI PHY initialization clear

	/* Step 07. Set the PHY for dqs pull down mode */
	mmio_write_32(&g_ddrphy_reg->LP_CON,
				(0x0 << 16) |					// [24:16] ctrl_pulld_dq
				(0xF <<  0));					// [ 8: 0] ctrl_pulld_dqs.  No Gate leveling : 0xF, Use Gate leveling : 0x0(X)
#if 0	/* Must Step ? */
	/* Memory Controller should assert "dfi_ctrlupd_req" after "dfi_init_complete" is set */
	/* Please keep "ctrl-initiated update" mode until finishing leveling and trining. */
	/* Step XX.  Phy Update Mode (0: MC-Initiated, 1: PHY-Initiated) */
	mmio_set_32  (&g_ddrphy_reg->OFFSETD_CON, (0x1 << 28));			// upd_mode=1
#endif

	/* [Drex] Step 08 : Update DLL information */
	mmio_set_32  (&g_drex_reg->PHYCONTROL,(0x1 << 3));			// Force DLL Resyncronization
	mmio_clear_32(&g_drex_reg->PHYCONTROL, (0x1 << 3));			// Force DLL Resyncronization

#if 1
	/* [Drex] Step 09-01. Set the Memory Control(MemControl)  */
	mmio_write_32(&g_drex_reg->MEMCONTROL,
		    	(0x0 << 29) |						// [31:29] pause_ref_en : Refresh command issue Before PAUSE ACKNOLEDGE
			(0x0 << 28) |						// [   28] sp_en        : Read with Short Preamble in Wide IO Memory
			(0x0 << 27) |						// [   27] pb_ref_en    : Per bank refresh for LPDDR4/LPDDR3
//			(0x0 << 25) |           					// [26:25] reserved : SBZ
			(0x0 << 24) |						// [   24] pzq_en       : DDR3 periodic ZQ(ZQCS) enable
//			(0x0  <<  23) |						// [   23] reserved     :SBZ
			(0x3 << 20) |						// [22:20] bl : Memory Burst Length :: 3'h3  - 8
			((DDR3_CS_NUM - 1) << 16) |				// [19:16] num_chip : Number of Memory Chips :: 4'h0  - 1chips
			(0x2 << 12) |						// [15:12] mem_width    : Width of Memory Data Bus :: 4'h2  - 32bits
			(0x6 <<  8) |						// [11: 8] mem_type     : Type of Memory :: 4'h6  - ddr3
			(0x0 <<  6) |						// [ 7: 6] add_lat_pall : Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
			(0x0 <<  5) |						// [    5] dsref_en     : Dynamic Self Refresh :: 1'b0  - Disable
//			(0x0 <<   4) |						// [    4] Reserved : SBZ
			(0x0 <<  2) |						// [ 3: 2] dpwrdn_type  : Type of Dynamic Power Down :: 2'b00 - Active/precharge power down
			(0x0 <<  1) |						// [    1] dpwrdn_en    : Dynamic Power Down :: 1'b0  - Disable
			(0x0 <<  0));						// [    0] clk_stop_en  : Dynamic Clock Control :: 1'b0  - Always running
#if 1
	/* [Drex] Step 09-02. Set the (Phy Control and Memory Control) */
	mmio_write_32(&g_drex_reg->PHYCONTROL,
		    	(0x1 << 31) |						// [   31] mem_term_en. Termination Enable for memory. Disable : 0, Enable : 1
			(0x1 << 30) |						// [   30] phy_term_en. Termination Enable for PHY. Disable : 0, Enable : 1
			(0x1 << 29) |						// [   29] ctrl_shgate. Duration of DQS Gating Signal. gate signal length <= 200MHz : 0, > 200MHz : 1
			(0x0 << 24) |						// [28:24] ctrl_pd. Input Gate for Power Down.
			(0x0 <<  8) |						// [    8] Termination Type for Memory Write ODT (0:single, 1:both chip ODT)
			(0x0 <<  7) |						// [    7] Resync Enable During PAUSE Handshaking
			(0x0 <<  4) |						// [ 6: 4] dqs_delay. Delay cycles for DQS cleaning. refer to DREX datasheet
			(0x0 <<  3) |						// [    3] fp_resync. Force DLL Resyncronization : 1. Test : 0x0
			(0x0 <<  2) |						// [    2] Drive Memory DQ Bus Signals
			(0x0 <<  1) |						// [    1] sl_dll_dyn_con. Turn On PHY slave DLL dynamically. Disable : 0, Enable : 1
			(0x1 << 0));						// [    0] mem_term_chips. Memory Termination between chips(2CS). Disable : 0, Enable : 1
#endif
#endif
	mmio_set_32  (&g_drex_reg->PHYCONTROL, (0x1 << 3));			// Force DLL Resyncronization
	mmio_clear_32(&g_drex_reg->PHYCONTROL, (0x1 << 3));			// Force DLL Resyncronization

	/* [Drex] Step 10. Memory Base Config */
	mmio_write_32(&g_drextz_reg->MEMBASECONFIG[0],
			(DDR3_CS0_BASEADDR << 16) |				// chip_base[26:16]. AXI Base Address. if 0x20 ==> AXI base addr of memory : 0x2000_0000
			(DDR3_CS_MEMMASK   << 0));				// 256MB:0x7F0, 512MB: 0x7E0, 1GB:0x7C0, 2GB: 0x780, 4GB:0x700
	mmio_write_32(&g_drextz_reg->MEMBASECONFIG[1],
			(DDR3_CS1_BASEADDR << 16) |				// chip_base[26:16]. AXI Base Address. if 0x40 ==> AXI base addr of memory:  0x4000_0000, 16MB unit
			(DDR3_CS_MEMMASK   << 0));				// chip_mask[10:0]. 2048 - chip size

	/* [Drex] Step 11. Memory Config */
	mmio_write_32(&g_drextz_reg->MEMCONFIG[0],
				(0x0 << 20) |					// bank lsb, LSB of Bank Bit Position in Complex Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12, 5:13
				(0x0 << 19) |					// rank inter en, Rank Interleaved Address Mapping
				(0x0 << 18) |					// bit sel en, Enable Bit Selection for Randomized interleaved Address Mapping
				(0x0 << 16) |					// bit sel, Bit Selection for Randomized Interleaved Address Mapping
				(0x2 << 12) |					// [15:12] chip_map. Address Mapping Method (AXI to Memory). 0:Linear(Bank, Row, Column, Width), 1:Interleaved(Row, bank, column, width), other : reserved
				(DDR3_COL_NUM << 8) |				// [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
				(DDR3_ROW_NUM << 4) |				// [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit,  2:14bit, 3:15bit, 4:16bit
				(DDR3_BANK_NUM << 0));				// [ 3: 0] chip_bank. Number of  Bank Address Bit. others:Reserved, 2:4bank, 3:8banks
#if (DDR3_CS_NUM > 1)
	mmio_write_32(&g_drextz_reg->MEMCONFIG[1],
				(0x0 << 20) |					// bank lsb, LSB of Bank Bit Position in Complex Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12, 5:13
				(0x0 << 19) |					// rank inter en, Rank Interleaved Address Mapping
				(0x0 << 18) |					// bit sel en, Enable Bit Selection for Randomized interleaved Address Mapping
				(0x0 << 16) |					// bit sel, Bit Selection for Randomized Interleaved Address Mapping
				(0x2 << 12) |					// [15:12] chip_map. Address Mapping Method (AXI to Memory). 0 : Linear(Bank, Row, Column, Width), 1 : Interleaved(Row, bank, column, width), other : reserved
				(DDR3_COL_NUM << 8) |				// [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
				(DDR3_ROW_NUM << 4) |				 // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
				(DDR3_BANK_NUM << 0));				// [ 3: 0] chip_bank. Number of  Row Address Bit.  others:Reserved, 2:4bank, 3:8banks
#endif

	/* [Drex] Step 12. Precharge Configuration */
#if 0
	mmio_write_32( &g_drex_reg->PRECHCONFIG0,
			(0xF <<  28) |						// Timeout Precharge per Port
			(0x0 <<  16));						// open page policy
	mmio_write_32( &g_drex_reg->PRECHCONFIG1, 0xFFFFFFFF);			//- precharge cycle
	mmio_write_32( &g_drex_reg->PWRDNCONFIG,  0xFFFF00FF);			//- low power counter
#endif
	mmio_write_32(&g_drex_reg->PRECHCONFIG1, 0x00);				//- precharge cycle
	mmio_write_32(&g_drex_reg->PWRDNCONFIG,  0xFF);				//- low power counter

	/* [Drex] Step 13.  Set the Access(AC) Timing */
	mmio_write_32(&g_drex_reg->TIMINGAREF,
			(tREFIPB << 16) |					//- rclk (MPCLK)
			(tREFI   << 0));					//- refresh counter, 800MHz : 0x618

	mmio_write_32(&g_drex_reg->ACTIMING0.TIMINGROW,
			(tRFC << 24) | (tRRD << 20) | (tRP << 16) | (tRCD << 12) |
			(tRC << 6) | (tRAS << 0));

	mmio_write_32(&g_drex_reg->ACTIMING0.TIMINGDATA,
			(tWTR << 28) | (tWR << 24) | (tRTP << 20) | (tPPD << 17) |
			(W2W_C2C << 14) | (R2R_C2C << 12) | (nWL << 8) |
			(tDQSCK << 4) | (nRL << 0));

	mmio_write_32(&g_drex_reg->ACTIMING0.TIMINGPOWER,
			(tFAW << 26) | (tXSR << 16) | (tXP << 8) | (tCKE << 4) |
			(tMRD << 0));
#if (DDR3_CS_NUM > 1)
	mmio_write_32(&g_drex_reg->ACTIMING1.TIMINGROW,
			(tRFC << 24) | (tRRD << 20) | (tRP << 16) | (tRCD << 12) |
			(tRC << 6) | (tRAS << 0));

	mmio_write_32(&g_drex_reg->ACTIMING1.TIMINGDATA,
			(tWTR << 28) | (tWR << 24) | (tRTP << 20) |
			(W2W_C2C << 14) |					// W2W_C2C
			(R2R_C2C << 12) |					// R2R_C2C
			(nWL << 8) | (tDQSCK << 4) |				// tDQSCK
			(nRL << 0));

	mmio_write_32(&g_drex_reg->ACTIMING1.TIMINGPOWER,
			(tFAW << 26) | (tXSR << 16) | (tXP << 8) | (tCKE << 4) |
			(tMRD << 0));
#endif

//	mmio_write_32(&g_drex_reg->TIMINGPZQ,   0x00004084);			//- average periodic ZQ interval. Max:0x4084
	mmio_write_32(&g_drex_reg->TIMINGPZQ, tPZQ);				//- average periodic ZQ interval. Max:0x4084

	mmio_write_32(&g_drex_reg->WRLVL_CONFIG[0], (2 << 4));			// tWLO[7:4]
//	mmio_write_32( &g_drex_reg->WRLVL_CONFIG[0], (tWLO <<  4));		// tWLO[7:4]

	/* temporary code according to suspend/resume policy. */
//	if (is_resume == 0) {
	{
		/* Step 18 :  Send NOP command. */
		send_directcmd(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
		send_directcmd(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
		/* Step 19 :  Send MR2 command. */
		send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR2.Reg);
#if (DDR3_CS_NUM > 1)
		send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR2.Reg);
#endif
		/* Step 20 :  Send MR3 command. */
		send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR3.Reg);
#if (DDR3_CS_NUM > 1)
		send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR3.Reg);
#endif
		/* Step 21 :  Send MR1 command. */
		send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (DDR3_CS_NUM > 1)
		send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1,  MR1.Reg);
#endif
		/* Step 22 :  Send MR0 command. */
		send_directcmd(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR0, MR0.Reg);
#if (DDR3_CS_NUM > 1)
		send_directcmd(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR0, MR0.Reg);
#endif

		/* Step 23 : Send ZQ Init command */
		send_directcmd(SDRAM_CMD_ZQINIT, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
		send_directcmd(SDRAM_CMD_ZQINIT, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
		DMC_Delay(100);
	} // if (is_resume)

#if 1	/* Skip the following steps if Leveling and Training are not required. (Optional features) */
	MEMMSG("\r\n########## Leveling & Training ##########\r\n");

	/* Step 24. Generate "ctrl_gate_p*", ctrl_read_p* */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[0], (0x1 <<  6));			// ctrl_atgate = 1
	/* Step 25.  Issue Phase 0/1 Read Command during read leveling */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[0], (0x1 << 14));			// p0_cmd_en = 1
	/* Step 26.  Initialize related logic before DQ Calibration */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[2], (0x1 <<  6));			// InitDeskewEn = 1

	/* Step 27. Byte Leveling enable. */
	mmio_set_32(&g_ddrphy_reg->PHY_CON[0], (0x1 << 13));			// byte_rdlvl_en = 1

	/* Step 28. The time to determine the VMP(Vaild Margin Period) Read Training */
	temp = mmio_read_32(&g_ddrphy_reg->PHY_CON[1]) & ~(0xF << 16);		// rdlvl_pass_adj = 4
	temp |= (0x4 << 16);
	mmio_write_32(&g_ddrphy_reg->PHY_CON[1], temp);

	/* Step 29-1. Set "cmd_active=14'h105E" as default value (=LP_DDR_CON3[13:0]) */
	mmio_write_32(&g_ddrphy_reg->LP_DDR_CON[3], 0x105E);			// cmd_active  = DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
	/* Step 29-2. Set "cmd_default=14'h107F" as default value (=LP_DDR_CON4[13:0]) */
	mmio_write_32(&g_ddrphy_reg->LP_DDR_CON[4], 0x107F); 			// cmd_default = DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F

	/* Step 30. Recommand the "rdlvel_incr_adj=7'h01" for the best margin */
	temp = mmio_read_32(&g_ddrphy_reg->PHY_CON[2]) & ~(0x7F << 16);	// rdlvl_incr_adj=1
	temp |= (0x1 << 16);
	mmio_write_32(&g_ddrphy_reg->PHY_CON[2], temp);

#if 0	/* Nexell Step XX. Why? Seperate? */
	mmio_clear_32(&g_ddrphy_reg->MDLL_CON[0], (0x1 <  5));			// ctrl_dll_on[5]=0
	do {
		temp = mmio_read_32(&g_ddrphy_reg->MDLL_CON[1]);		// read lock value
	} while((temp & 0x7) < 0x5);
#else

	/*
	  * Step 31. Disable "ctrl_dll_on" int MDLL_CON0[5] before Leveling
	  * Turn on if the signal is High DLL turn on/Low is turen off.
	  */
	do {
		mmio_set_32(&g_ddrphy_reg->MDLL_CON[0],(0x1 << 5));		// ctrl_dll_on[5]=1
		/* Step 31-1. Read "ctrl_lock_value[8:0]" in MDLL_CON1[16:8] */
		do {
			temp = mmio_read_32(&g_ddrphy_reg->MDLL_CON[1]);	// read lock value
		} while ((temp & 0x7) != 0x7);

		mmio_clear_32(&g_ddrphy_reg->MDLL_CON[0], (0x1 << 5));		// ctrl_dll_on[5]=0
		temp = mmio_read_32(&g_ddrphy_reg->MDLL_CON[1]);		// read lock value
	} while ((temp & 0x7) != 0x7);
#endif
	g_Lock_Val = (temp >> 8) & 0x1FF;

#if (DDR_MEMINFO_SHOWLOCK == 1)
	show_lock_value();
#endif
	/* Step 31-2. Update "ctrl_force[8:0]" in MDLL_CON0[15:7] by the value of "ctrl_lock_value[8:0] */
	temp = mmio_read_32(&g_ddrphy_reg->MDLL_CON[0]) & ~(0x1FF << 7);
	temp |= (g_Lock_Val << 7);
	mmio_write_32(&g_ddrphy_reg->MDLL_CON[0], temp);				// ctrl_force[16:8]

//	mmio_set_32  (&g_ddrphy_reg->RODT_CON, (0x1  << 28));		// ctrl_readduradj [31:28]

#if (SKIP_LEVELING_TRAINING == 0)
	/* Step 32. DDR Controller Calibration*/

	/* temporary code according to suspend/resume policy. */
//	if (is_resume == 0) {
	{
	#if (DDR_WRITE_LEVELING_EN == 1)
		/* Step 32-1. Write Leveling (for Fly-by) */
		if (DDR3_LvlTr & LVLTR_WR_LVL) {
			if (ddr_hw_write_leveling() < 0)
				return -1;
		}
	#endif

	#if (DDR_GATE_LEVELING_EN == 1)
		/*
		  * Step 32-2. Gate Leveling
		  * (It should be used only for DDR3 (800Mhz))
		  */
		if (DDR3_LvlTr & LVLTR_GT_LVL) {
			if (ddr_gate_leveling() < 0)
				return -1;
		}
	#endif

	#if (DDR_READ_DQ_CALIB_EN == 1)
		/* Step 32-3. Read DQ Calibration */
		if (DDR3_LvlTr & LVLTR_RD_CAL) {
			if (ddr_read_dq_calibration() < 0)
				return -1;
		}
	#endif

	#if (DDR_WRITE_LEVELING_EN == 1)
		/* Step 32-4. Write Latenty Calibration */
		if (DDR3_LvlTr & LVLTR_WR_LVL) {
			if (ddr_write_latency_calibration() < 0)
				return -1;
		}
	#endif

	#if (DDR_WRITE_DQ_CALIB_EN == 1)
		/* Step 32-5. Write DQ Calibration */
		if (DDR3_LvlTr & LVLTR_WR_CAL) {
			if (ddr_write_dq_calibration() < 0)
				return -1;
		}
	#endif
#if 0
		/* Nexell Step XX. Save leveling & training values. */
	        mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG,     1);		// open alive power gate

	        mmio_write_32(&pReg_Alive->ALIVESCRATCHRST5,    0xFFFFFFFF);	// clear - ctrl_shiftc
	        mmio_write_32(&pReg_Alive->ALIVESCRATCHRST6,    0xFFFFFFFF);	// clear - ctrl_offsetC
	        mmio_write_32(&pReg_Alive->ALIVESCRATCHRST7,    0xFFFFFFFF);	// clear - ctrl_offsetr
	        mmio_write_32(&pReg_Alive->ALIVESCRATCHRST8,    0xFFFFFFFF);	// clear - ctrl_offsetw

	        mmio_write_32(&pReg_Alive->ALIVESCRATCHSET5,    g_GT_cycle);	// store - ctrl_shiftc
	        mmio_write_32(&pReg_Alive->ALIVESCRATCHSET6,    g_GT_code);	// store - ctrl_offsetc
	        mmio_write_32(&pReg_Alive->ALIVESCRATCHSET7,    g_RD_vwmc);	// store - ctrl_offsetr
	        mmio_write_32(&pReg_Alive->ALIVESCRATCHSET8,    g_WR_vwmc);	// store - ctrl_offsetw

	        mmio_write_32(&pReg_Alive->ALIVEPWRGATEREG,     0);		// close alive power gate
#endif
	}

	/* Nexell Step XX. (Must need step?) */
	mmio_set_32  (&g_ddrphy_reg->OFFSETD_CON, (0x1 << 24));			// ctrl_resync[24]=0x1 (HIGH)
	mmio_clear_32(&g_ddrphy_reg->OFFSETD_CON, (0x1 << 24));			// ctrl_resync[24]=0x0 (LOW)
#endif // #if (SKIP_LEVELING_TRAINING == 0)
#if 0
	/* Nexell Step XX.  */
	mmio_clear_32(&g_ddrphy_reg->PHY_CON[0],  (0x3 << 13));			// p0_cmd_en[14]=0, byte_rdlvl_en[13]=0
#endif
	/* Step 33. DLL turn on */
	mmio_set_32  (&g_ddrphy_reg->MDLL_CON[0], (0x1 << 5));			// ctrl_dll_on[5]=1

	/* Step 34. Deskew Code is updated. */
	mmio_set_32  (&g_ddrphy_reg->PHY_CON[2],  (0x1 << 12));		// DLLDeskewEn[12]=1

	/* Step 35.  Phy Update Mode (0: MC-Initiated, 1: PHY-Initiated) */
	mmio_set_32  (&g_ddrphy_reg->OFFSETD_CON, (0x1 << 28));			// upd_mode=1

	/* Step 36. Enable and Disable "ctrl_resync"(=OFFSETD_CON0[24]" to make sure All SDLL is updated. */
	mmio_set_32  (&g_drex_reg->PHYCONTROL, (0x1 << 3));			// Force DLL Resyncronization
	mmio_clear_32(&g_drex_reg->PHYCONTROL, (0x1 << 3));			// Force DLL Resyncronization
#endif // Skip the following steps if Leveling and Training are not required. (Optional features)

	/* Step 37. Disable PHY gateing control through PHY APB I/F */
	/* if necessary(ctrl_Atgate, refer to PHY manual)		    */

	/* Step 38. Send PALL command */
	send_directcmd(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (DDR3_CS_NUM > 1)
	send_directcmd(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

#if 0	// Set the MemControl0 & PhyControl0 (Optional)
#if 0	/* [Drex] Step 39-1. Set the Memory Control(MemControl)  (Optional) */
	mmio_write_32(&g_drex_reg->MEMCONTROL,
		    	(0x0 << 29) |						// [31:29] pause_ref_en : Refresh command issue Before PAUSE ACKNOLEDGE
			(0x0 << 28) |						// [   28] sp_en        : Read with Short Preamble in Wide IO Memory
			(0x0 << 27) |						// [   27] pb_ref_en    : Per bank refresh for LPDDR4/LPDDR3
//			(0x0 << 25) |           					// [26:25] reserved : SBZ
			(0x0 << 24) |						// [   24] pzq_en       : DDR3 periodic ZQ(ZQCS) enable
//			(0x0 << 23) |						// [   23] reserved     :SBZ
			(0x3 << 20) |						// [22:20] bl : Memory Burst Length :: 3'h3  - 8
			((DDR3_CS_NUM - 1) << 16) |				// [19:16] num_chip : Number of Memory Chips :: 4'h0  - 1chips
			(0x2 << 12) |						// [15:12] mem_width    : Width of Memory Data Bus :: 4'h2  - 32bits
			(0x6 <<  8) |						// [11: 8] mem_type     : Type of Memory :: 4'h6  - ddr3
			(0x0 <<  6) |						// [ 7: 6] add_lat_pall : Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
			(0x0 <<  5) |						// [    5] dsref_en     : Dynamic Self Refresh :: 1'b0  - Disable
//			(0x0 <<  4) |						// [    4] Reserved : SBZ
			(0x0 <<  2) |						// [ 3: 2] dpwrdn_type  : Type of Dynamic Power Down :: 2'b00 - Active/precharge power down
			(0x0 <<  1) |						// [    1] dpwrdn_en    : Dynamic Power Down :: 1'b0  - Disable
			(0x0 <<  0));						// [    0] clk_stop_en  : Dynamic Clock Control :: 1'b0  - Always running
#endif

	/* [Drex] Step 39-2. Set the (Phy Control and Memory Control) */
	mmio_write_32(&g_drex_reg->PHYCONTROL,
		    	(0x1 << 31) |						// [   31] mem_term_en. Termination Enable for memory. Disable : 0, Enable : 1
			(0x1 << 30) |						// [   30] phy_term_en. Termination Enable for PHY. Disable : 0, Enable : 1
			(0x1 << 29) |						// [   29] ctrl_shgate. Duration of DQS Gating Signal. gate signal length <= 200MHz : 0, > 200MHz : 1
			(0x0 << 24) |						// [28:24] ctrl_pd. Input Gate for Power Down.
			(0x0 <<  8) |						// [    8] Termination Type for Memory Write ODT (0:single, 1:both chip ODT)
			(0x0 <<  7) |						// [    7] Resync Enable During PAUSE Handshaking
			(0x0 <<  4) |						// [ 6: 4] dqs_delay. Delay cycles for DQS cleaning. refer to DREX datasheet
			(0x0 <<  3) |						// [    3] fp_resync. Force DLL Resyncronization : 1. Test : 0x0
			(0x0 <<  2) |						// [    2] Drive Memory DQ Bus Signals
			(0x0 <<  1) |						// [    1] sl_dll_dyn_con. Turn On PHY slave DLL dynamically. Disable : 0, Enable : 1
			(0x1 << 0));						// [    0] mem_term_chips. Memory Termination between chips(2CS). Disable : 0, Enable : 1
#endif
	/* [Drex] Step 40. Set the Controller Control */
	temp = (unsigned int)((0x0   << 28) |					// [   28] dfi_init_start
			      (0xFFF << 16) |					// [27:16] timeout_level0
			      (0x1   << 12) |					// [14:12] rd_fetch
			      (0x1   <<  8) |					// [    8] empty
			      (0x0   <<  6) |					// [ 7: 6] io_pd_con
			      (0x1   <<  5) |					// [    5] aref_en - Auto Refresh Counter. Disable:0, Enable:1
			      (0x0   <<  3) |					// [    3] update_mode - Update Interface in DFI.
			      (0x0   <<  1) |					// [ 2: 1] clk_ratio
			      (0x0   <<  0));					// [    0] ca_swap

	if (is_resume)
		temp &= ~(0x1 << 5);

	mmio_write_32(&g_drex_reg->CONCONTROL, temp);

	/* [Drex] Step 41. Set the Clock Gating Control */
	mmio_write_32(&g_drex_reg->CGCONTROL,
			(0x0 <<  4) |						// [    4] phy_cg_en
			(0x0 <<  3) |						// [    3] memif_cg_en
			(0x0 <<  2) |						// [    2] scg_sg_en
			(0x0 <<  1) |						// [    1] busif_wr_cg_en
			(0x0 <<  0));						// [    0] busif_rd_cg_en

	MEMMSG("DLL Lock Value = %d \r\n", g_Lock_Val);

	MEMMSG("g_GT_cycle  = 0x%08X\r\n", g_GT_cycle);
	MEMMSG("g_GT_code   = 0x%08X\r\n", g_GT_code);
	MEMMSG("g_RD_vwmc   = 0x%08X\r\n", g_RD_vwmc);
	MEMMSG("g_WR_vwmc   = 0x%08X\r\n", g_WR_vwmc);

	MEMMSG("GATE CYC    = 0x%08X\r\n", mmio_read_32(&g_ddrphy_reg->SHIFTC_CON));
	MEMMSG("GATE CODE   = 0x%08X\r\n", mmio_read_32(&g_ddrphy_reg->OFFSETC_CON[0]));

	MEMMSG("Read  DQ    = 0x%08X\r\n", mmio_read_32(&g_ddrphy_reg->OFFSETR_CON[0]));
	MEMMSG("Write DQ    = 0x%08X\r\n", mmio_read_32(&g_ddrphy_reg->OFFSETW_CON[0]));

	return 0;
}
