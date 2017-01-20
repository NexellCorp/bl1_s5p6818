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
#ifndef __MAIN_H__
#define __MAIN_H__

/* Extern Function Define */
void DMC_Delay(int milisecond);

/* Set the Reset Generatior (IP Block) */
void ResetCon(U32 devicenum, CBOOL en);

/* EMA(Extra Margin Adjustments) Function */
void cache_setup_ema(void);
void ema_information(void);

/* PMIC(power management ic) Function */
void pmic_initalize(void);

/* CCI400, Bus, Drex Fuction */
void cci400_initialize(void);
void set_bus_config(void);
void set_drex_qos(void);

/* (ddr3/lpdde3) sdram memory function define */
#ifdef MEM_TYPE_DDR3
 int ddr3_initialize(unsigned int);
#endif
#ifdef MEM_TYPE_LPDDR23
 int init_LPDDR3(unsigned int);
#endif
void enter_self_refresh(void);
void exit_self_refresh(void);

/* Secondary CPU Function */
 int s5p6818_subcpu_bringup(unsigned int cpu_id);

 int s5p6818_resume_check(void);

/* Secure Function */
void secure_set_state(void);

/* Build Infomation Function */
 int build_information(void);

/* S5P6818 - Resume Function */
void s5p6818_resume(void);

/* S5P6818 - Jump to Romboot */
extern void RomUSBBoot(unsigned int RomJumpAddr);

/* Memory Test (Optional) */
void simple_memtest(unsigned int *pStart, unsigned int *pEnd);
 int memtester_main(unsigned int start, unsigned int end);

/* CRC Algorithm Check Function */
 int CRC_Check(void* buf, unsigned int size, unsigned int ref_crc);

/* Extern Boot Mode Function */
 int iUSBBOOT(struct NX_SecondBootInfo *const pTBI);
 int iUARTBOOT(struct NX_SecondBootInfo *const pTBI);
 int iSPIBOOT(struct NX_SecondBootInfo *const pTBI);
 int iSDXCBOOT(struct NX_SecondBootInfo *const pTBI);
 int iNANDBOOTEC(struct NX_SecondBootInfo *const pTBI);
 int iSDXCFSBOOT(struct NX_SecondBootInfo *const pTBI);
#endif	// __MAIN_H__
