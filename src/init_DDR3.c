/*                                                                             
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                             
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                             
 *      Module          : DDR3
 *      File            : init_DDR3.c
 *      Description     : 
 *      Author          : Kook
 *      History         : 
 */
#include "sysHeader.h"

#include <nx_drex.h>
#include <nx_ddrphy.h>
#include "ddr3_sdram.h"

#define CFG_ODT_OFF                     (0)
#define CFG_FLY_BY                      (0)
#define CFG_8BIT_DESKEW                 (0)
#define CFG_UPDATE_DREX_SIDE            (1)     // 0 : PHY side,  1: Drex side
#define SKIP_LEVELING_TRAINING          (0)

//#define USE_HEADER
#define DDR_RW_CAL      0

#define DDR_CA_SWAP_MODE                (0)     // for LPDDR3

#define DDR_WRITE_LEVELING_EN           (0)     // for fly-by
#define DDR_CA_CALIB_EN                 (0)     // for LPDDR3
#define DDR_GATE_LEVELING_EN            (0)     // for DDR3, great then 800MHz
#define DDR_READ_DQ_CALIB_EN            (0)
#define DDR_WRITE_LATENCY_CALIB_EN      (1)     // for fly-by
#define DDR_WRITE_DQ_CALIB_EN           (0)

#define DDR_RESET_GATE_LVL              (1)
#define DDR_RESET_READ_DQ               (1)
#define DDR_RESET_WRITE_DQ              (1)


#if (CFG_NSIH_EN == 0)
#include "DDR3_K4B8G1646B_MCK0.h"
#endif

#ifdef aarch32
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");
#endif

#ifdef aarch64
#define nop() __asm__ __volatile__("mov\tx0,x0\t\r\n nop\n\t");
#endif


extern inline void ResetCon(U32 devicenum, CBOOL en);
extern inline void DMC_Delay(int milisecond);

struct phy_lock_info
{
    U32 val;
    U32 count;
    U32 lock_count[5];
};

U32 g_Lock_Val;
U32 g_WR_lvl;
U32 g_GT_cycle;
U32 g_GT_code;
U32 g_RD_vwmc;
U32 g_WR_vwmc;


#if 0
void showLockValue(void)
{
    struct phy_lock_info lock_info[20];
    U32 fFound = 0;
    U32 lock_status, lock_val;
    U32 temp, i, j;

    for (i = 0; i < 20; i++)
    {
        lock_info[i].val        = 0;
        lock_info[i].count      = 0;

        for (j = 0; j < 5; j++)
        {
            lock_info[i].lock_count[j]  = 0;
        }
    }

    for (i = 0; i < 1000000; i++)
    {
        temp        = ReadIO32( &pReg_DDRPHY->MDLL_CON[1] );
        lock_status = temp & 0x7;
        lock_val    = (temp >> 8) & 0x1FF;         // read lock value

        fFound = 0;

        for (j = 0; lock_info[j].val != 0; j++)
        {
            if (lock_info[j].val == lock_val)
            {
                fFound = 1;
                lock_info[j].count++;
                if (lock_status)
                    lock_info[j].lock_count[(lock_status>>1)]++;
                else
                    lock_info[j].lock_count[4]++;
            }
        }

        if (j == 20)
            break;

        if (fFound == 0)
        {
            lock_info[j].val   = lock_val;
            lock_info[j].count = 1;
            if (lock_status)
                lock_info[j].lock_count[(lock_status>>1)] = 1;
            else
                lock_info[j].lock_count[4]  = 1;
        }

        DMC_Delay(10);
    }

    printf("\r\n");
    printf("--------------------------------------\r\n");
    printf(" Show lock values : %d\r\n", g_Lock_Val );
    printf("--------------------------------------\r\n");

    printf("lock_val,   hit       bad, not bad,   good, better,   best\r\n");

    for (i = 0; lock_info[i].val; i++)
    {
        printf("[%6d, %6d] - [%6d", lock_info[i].val, lock_info[i].count, lock_info[i].lock_count[4]);

        for (j = 0; j < 4; j++)
        {
            printf(", %6d", lock_info[i].lock_count[j]);
        }
        printf("]\r\n");
    }
}
#endif

#if 0
void DUMP_PHY_REG(void)
{
    U32     *pAddr = (U32 *)&pReg_DDRPHY->PHY_CON[0];
    U32     temp;
    U32     i;

    for (i = 0; i < (0x3AC>>2); i++)
    {
        temp = ReadIO32( pAddr + i );

        if ( (i & 3) == 0 ) {
            printf("\r\n0x%08X :", (i<<2));
        }

        printf(" %08x", temp);
    }
    printf("\r\n");
}
#endif

#if defined(MEM_TYPE_DDR3)
//inline
void SendDirectCommand(SDRAM_CMD cmd, U8 chipnum, SDRAM_MODE_REG mrx, U16 value)
{
    WriteIO32( &pReg_Drex->DIRECTCMD, (U32)((cmd<<24) | ((chipnum & 1)<<20) | (mrx<<16) | value) );
}
#endif
#if defined(MEM_TYPE_LPDDR23)
//inline
void SendDirectCommand(SDRAM_CMD cmd, U8 chipnum, SDRAM_MODE_REG mrx, U16 value)
{
    WriteIO32( &pReg_Drex->DIRECTCMD, (U32)((cmd<<24) | ((chipnum & 1)<<20) | (((mrx>>3) & 0x7)<<16) | ((mrx & 0x7)<<10) | ((value & 0xFF)<<2) | ((mrx>>6) & 0x3)) );
}
#endif

void enterSelfRefresh(void)
{
    union SDRAM_MR MR;
    U32     nTemp;
    U32     nChips = 0;

#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    nChips = 0x3;
#else
    nChips = 0x1;
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        nChips = 0x3;
    else
        nChips = 0x1;
#endif


    while( ReadIO32(&pReg_Drex->CHIPSTATUS) & 0xF )
    {
        nop();
    }

    /* Send PALL command */
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
    DMC_Delay(100);

    // odt off
    MR.Reg          = 0;
    MR.MR2.RTT_WR   = 0;        // 0: disable, 1: RZQ/4 (60ohm), 2: RZQ/2 (120ohm)
    MR.MR2.SRT      = 0;        // self refresh normal range, if (ASR == 1) SRT = 0;
    MR.MR2.ASR      = 1;        // auto self refresh enable
#if (CFG_NSIH_EN == 0)
    MR.MR2.CWL      = (nCWL - 5);
#else
    MR.MR2.CWL      = (pSBI->DII.CWL - 5);
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif

    MR.Reg          = 0;
    MR.MR1.DLL      = 1;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR.MR1.AL       = MR1_nAL;
#else
    MR.MR1.AL       = pSBI->DII.MR1_AL;
#endif
    MR.MR1.ODS1     = pSBI->DDR3_DSInfo.MR1_ODS & (1 << 1);
    MR.MR1.ODS0     = pSBI->DDR3_DSInfo.MR1_ODS & (1 << 0);
    MR.MR1.RTT_Nom2 = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 2);
    MR.MR1.RTT_Nom1 = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 1);
    MR.MR1.RTT_Nom0 = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 0);
    MR.MR1.QOff     = 0;
    MR.MR1.WL       = 0;
#if 0
#if (CFG_NSIH_EN == 0)
    MR.MR1.TDQS     = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR.MR1.TDQS     = (pSBI->DII.BusWidth>>3) & 1;
#endif
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif

    /* Enter self-refresh command */
    SendDirectCommand(SDRAM_CMD_REFS, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_REFS, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_REFS, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

    do
    {
        nTemp = ( ReadIO32(&pReg_Drex->CHIPSTATUS) & nChips );
    } while( nTemp );

    do
    {
        nTemp = ( (ReadIO32(&pReg_Drex->CHIPSTATUS) >> 8) & nChips );
    } while( nTemp != nChips );

    // Step 52 Auto refresh counter disable
    ClearIO32( &pReg_Drex->CONCONTROL,  (0x1    <<   5) );          // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1

    // Step 10  ACK, ACKB off
    SetIO32( &pReg_Drex->MEMCONTROL,    (0x1    <<   0) );          // clk_stop_en[0] : Dynamic Clock Control   :: 1'b0  - Always running

//    DMC_Delay(1000 * 3);
}

void exitSelfRefresh(void)
{
    union SDRAM_MR MR;

    // Step 10    ACK, ACKB on
    ClearIO32( &pReg_Drex->MEMCONTROL,  (0x1    <<   0) );          // clk_stop_en[0] : Dynamic Clock Control   :: 1'b0  - Always running
    DMC_Delay(10);

    // Step 52 Auto refresh counter enable
    SetIO32( &pReg_Drex->CONCONTROL,    (0x1    <<   5) );          // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1
    DMC_Delay(10);

    /* Send PALL command */
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

    MR.Reg          = 0;
    MR.MR1.DLL      = 0;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR.MR1.AL       = MR1_nAL;
#else
    MR.MR1.AL       = pSBI->DII.MR1_AL;
#endif
    MR.MR1.ODS1     = pSBI->DDR3_DSInfo.MR1_ODS & (1 << 1);
    MR.MR1.ODS0     = pSBI->DDR3_DSInfo.MR1_ODS & (1 << 0);
    MR.MR1.RTT_Nom2 = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 2);
    MR.MR1.RTT_Nom1 = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 1);
    MR.MR1.RTT_Nom0 = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 0);
    MR.MR1.QOff     = 0;
    MR.MR1.WL       = 0;
#if 0
#if (CFG_NSIH_EN == 0)
    MR.MR1.TDQS     = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR.MR1.TDQS     = (pSBI->DII.BusWidth>>3) & 1;
#endif
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif

    // odt on
    MR.Reg          = 0;
    MR.MR2.RTT_WR   = pSBI->DDR3_DSInfo.MR2_RTT_WR;
    MR.MR2.SRT      = 0;        // self refresh normal range
    MR.MR2.ASR      = 0;        // auto self refresh disable
#if (CFG_NSIH_EN == 0)
    MR.MR2.CWL      = (nCWL - 5);
#else
    MR.MR2.CWL      = (pSBI->DII.CWL - 5);
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif

    /* Exit self-refresh command */
    SendDirectCommand(SDRAM_CMD_REFSX, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_REFSX, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_REFSX, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

#if 0
    while( ReadIO32(&pReg_Drex->CHIPSTATUS) & (0xF << 8) )
    {
        nop();
    }
#endif

//    DMC_Delay(1000 * 2);
}


#if DDR_RW_CAL
void DDR3_RW_Delay_Calibration(void)
{
extern    void BurstZero(U32 *WriteAddr, U32 WData);
extern    void BurstWrite(U32 *WriteAddr, U32 WData);
extern    void BurstRead(U32 *ReadAddr, U32 *SaveAddr);
//    struct NX_DREXSDRAM_RegisterSet *pReg_Drex = (struct NX_DREXSDRAM_RegisterSet *)PHY_BASEADDR_DREX_MODULE_CH0_APB;
    struct NX_DDRPHY_RegisterSet *pReg_DDRPHY = (struct NX_DDRPHY_RegisterSet *)PHY_BASEADDR_DREX_MODULE_CH1_APB;
    unsigned int rnw, lane, adjusted_dqs_delay=0, bit, pmin, nmin;
    unsigned int *tptr = (unsigned int *)0x40100000;
    int toggle;

    for(rnw = 0; rnw<2; rnw++)
    {
        printf("\r\nserching %s delay value......\r\n", rnw?"read":"write");
        bit = 0;
        for(bit = 0; bit<32; bit++)
        {
            unsigned int readdata[8];
            unsigned int dqs_wdelay, repeatcount;
            unsigned char pwdelay, nwdelay;
            lane = bit>>3;

            if((bit & 7) == 0)
            {
                pmin = 0x7f;
                nmin = 0x7f;
            }
            printf("bit:%02d\t", bit);
            pwdelay = 0x80;
            if(rnw ==0)
                WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], 0x80<<(8*lane));
            else
                WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], 0x80<<(8*lane));
            SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            DMC_Delay(10000);
            for(dqs_wdelay = 0; dqs_wdelay<=0x7f && pwdelay==0x80; dqs_wdelay++)
            {
                repeatcount=0;
                if(rnw ==0)
                    WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], dqs_wdelay<<(8*lane));
                else
                    WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], dqs_wdelay<<(8*lane));
                SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                DMC_Delay(10000);
                while(repeatcount<100)
                {
                    for(toggle=1; toggle>=0; toggle--)
                    {
                        if(toggle)
                            BurstWrite(tptr, 1<<bit);
                        else
                            BurstWrite(tptr, ~(1<<bit));
                        BurstRead(tptr, readdata);
                        if( ((readdata[0]>>bit)&0x01) == !toggle &&
                            ((readdata[1]>>bit)&0x01) == !toggle &&
                            ((readdata[2]>>bit)&0x01) == !toggle &&
                            ((readdata[3]>>bit)&0x01) == !toggle &&
                            ((readdata[4]>>bit)&0x01) ==  toggle &&
                            ((readdata[5]>>bit)&0x01) == !toggle &&
                            ((readdata[6]>>bit)&0x01) == !toggle &&
                            ((readdata[7]>>bit)&0x01) == !toggle)
                        {
                            repeatcount++;
                        }else
                        {
                            pwdelay = dqs_wdelay;
                            if(pmin > pwdelay)
                                pmin = pwdelay;
                            printf("p%d:%02d ", toggle, pwdelay);
                            repeatcount = 100;
                            toggle = -1;
                            break;
                        }
                    }
                }
            }    // dqs_wdelay
            if(rnw==0)
                WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], 0<<(8*lane));
            else
                WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], 0<<(8*lane));
            SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            DMC_Delay(10000);
            nwdelay = 0;
            for(dqs_wdelay = 0x80; dqs_wdelay<=0xFF && nwdelay==0; dqs_wdelay++)
            {
                repeatcount=0;
                if(rnw == 0)
                    WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], dqs_wdelay<<(8*lane));
                else
                    WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], dqs_wdelay<<(8*lane));
                SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                DMC_Delay(10000);
                while(repeatcount<100)
                {
                    for(toggle=1; toggle>=0; toggle--)
                    {
                        if(toggle)
                            BurstWrite(tptr, 1<<bit);
                        else
                            BurstWrite(tptr, ~(1<<bit));
                        BurstRead(tptr, readdata);
                        if( ((readdata[0]>>bit)&0x01) == !toggle &&
                            ((readdata[1]>>bit)&0x01) == !toggle &&
                            ((readdata[2]>>bit)&0x01) == !toggle &&
                            ((readdata[3]>>bit)&0x01) == !toggle &&
                            ((readdata[4]>>bit)&0x01) ==  toggle &&
                            ((readdata[5]>>bit)&0x01) == !toggle &&
                            ((readdata[6]>>bit)&0x01) == !toggle &&
                            ((readdata[7]>>bit)&0x01) == !toggle)
                        {
                            repeatcount++;
                        }else
                        {
                            nwdelay = dqs_wdelay & 0x7F;
                            if(nmin > nwdelay)
                                nmin = nwdelay;
                            printf("n%d:%02d  ", toggle, nwdelay);
                            repeatcount = 100;
                            toggle = -1;
                            break;
                        }
                    }
                }
            }    // dqs_wdelay

            if(pwdelay > nwdelay)    // biased to positive side
            {
                printf("margin: %2d  adj: %2d\r\n", (pwdelay - nwdelay), (pwdelay - nwdelay)>>1);
            }
            else    // biased to negative side
            {
                printf("margin: %2d  adj:-%2d\r\n", (nwdelay - pwdelay), (nwdelay - pwdelay)>>1);
            }
            if((bit & 7)==7)
            {
                printf("lane average positive min:%d negative min:%d ", pmin, nmin);
                if(pmin > nmin) // biased to positive side
                {
                    adjusted_dqs_delay |= ((pmin - nmin)>>1) << (8*lane);
                    printf("margin: %2d  adj: %2d\r\n", (pmin - nmin), (pmin - nmin)>>1);
                }
                else    // biased to negative side
                {
                    adjusted_dqs_delay |= (((nmin - pmin)>>1) | 0x80) << (8*lane);
                    printf("margin: %2d  adj:-%2d\r\n", (nmin - pmin), (nmin - pmin)>>1);
                }
            }
            if(((bit+1) & 0x7) == 0)
                printf("\n");
        }   // lane

        if(rnw == 0)
            WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], adjusted_dqs_delay);
        else
            WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], adjusted_dqs_delay);
        SetIO32  ( &pReg_Drex->PHYCONTROL,   (0x1   <<   3));           // Force DLL Resyncronization
        ClearIO32( &pReg_Drex->PHYCONTROL,   (0x1   <<   3));           // Force DLL Resyncronization
        printf("\r\n");
        printf("read  delay value is 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->OFFSETR_CON[0]));
        printf("write delay value is 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->OFFSETW_CON[0]));
    }
}
#endif

#if (SKIP_LEVELING_TRAINING == 0)
CBOOL DDR_HW_Write_Leveling(void)
{
#if defined(MEM_TYPE_DDR3)
    union SDRAM_MR MR1;
#endif
    volatile U32 cal_count = 0;
    U32     temp;
    CBOOL   ret = CTRUE;

    MEMMSG("\r\n########## Write Leveling - Start ##########\r\n");

#if 1
    // Send PALL command.
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
//    DMC_Delay(0x100);
#endif


    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[2]) & ~(0x7F << 16);
    temp |= (0x2 <<  16);                                                   // rdlvl_incr_adj=2
//    WriteIO32( &pReg_DDRPHY->PHY_CON[2],        temp);


#if defined(MEM_TYPE_DDR3)
    /* Set MPR mode enable */
    MR1.Reg         = 0;
    MR1.MR1.DLL     = 0;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR1.MR1.AL      = MR1_nAL;
#else
    MR1.MR1.AL      = pSBI->DII.MR1_AL;
#endif
#if 1
    MR1.MR1.ODS1    = 0;    // 00: RZQ/6, 01 : RZQ/7
    MR1.MR1.ODS0    = 1;
#else
    MR1.MR1.ODS1    = 0;    // 00: RZQ/6, 01 : RZQ/7
    MR1.MR1.ODS0    = 0;
#endif
    MR1.MR1.QOff    = 0;
    MR1.MR1.RTT_Nom2    = 0;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
    MR1.MR1.RTT_Nom1    = 1;
    MR1.MR1.RTT_Nom0    = 1;
    MR1.MR1.WL      = 1;    // Write leveling enable
#if 0
#if (CFG_NSIH_EN == 0)
    MR1.MR1.TDQS    = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR1.MR1.TDQS    = (pSBI->DII.BusWidth>>3) & 1;
#endif
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
//    DMC_Delay(0x100);
#endif  // #if defined(MEM_TYPE_DDR3)


#if (CFG_ODT_OFF == 1)
//    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x7    <<  29) );          // ODT Disable

    ClearIO32( &pReg_Drex->WRLVL_CONFIG[0],     (0x1    <<   0) );          // odt_on[0]="0" (turn off)

    WriteIO32( &pReg_Drex->WRLVL_CONFIG[1],     0x0 );                      // dfi_wdata_en_p0[0]="0" (turn off)
    WriteIO32( &pReg_Drex->WRLVL_CONFIG[1],     0x1 );                      // dfi_wdata_en_p0[0]="0" (turn on)

    ClearIO32( &pReg_DDRPHY->LP_DDR_CON[4],     (0x3    <<   7) );          // cmd_default, ODT[8:7]=0x0
#else

//    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x7    <<  29) );          // ODT Enable

    SetIO32  ( &pReg_Drex->WRLVL_CONFIG[0],     (0x1    <<   0) );          // odt_on[0]="1" (turn on)

    WriteIO32( &pReg_Drex->WRLVL_CONFIG[1],     0x1 );                      // dfi_wdata_en_p0[0]="1"
    WriteIO32( &pReg_Drex->WRLVL_CONFIG[1],     0x0 );                      // dfi_wdata_en_p0[0]="0"

    SetIO32  ( &pReg_DDRPHY->LP_DDR_CON[4],     (0x3    <<   7) );          // cmd_default, ODT[8:7]=0x3
#endif  // #if (CFG_ODT_OFF == 1)

    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  16) );          // wrlvl_mode[16]="1" (Enable)


//    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  16) );          // wrlvl_start[16]="1" (Enable)
    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  16) );          // wrlvl_start[16]="1" (Enable)

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        temp = ReadIO32( &pReg_DDRPHY->PHY_CON[3] );
        if ( temp & (0x1 << 24) )                                           // wrlvl_resp[24] : Wating until WRITE leveling is complete
        {
            break;
        }

        MEMMSG("WRITE LVL: Waiting wrlvl_resp...!!!\r\n");
        DMC_Delay(100);
    }

    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  24) );          // wrlvl_resp[24]="0"

//    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        0x0 );                      // wrlvl_start[16]="0" (Disable)
    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  16) );          // wrlvl_start[16]="0" (Disable)
    ClearIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  16) );          // wrlvl_mode[16]="0" (Disable)

    //------------------------------------------------------------------------------------------------------------------------

    if (cal_count == 100)
    {
        MEMMSG("WRITE LVL: Leveling Responese Checking : fail...!!!\r\n");

        ret = CFALSE;                                               // Failure Case
    }

    //------------------------------------------------------------------------------------------------------------------------

    g_WR_lvl = ReadIO32( &pReg_DDRPHY->WR_LVL_CON[0] );

#if 0
    SetIO32  ( &pReg_DDRPHY->WR_LVL_CON[3],     (0x1    <<   0) );          // wrlvl_resync[0]=0x1
    ClearIO32( &pReg_DDRPHY->WR_LVL_CON[3],     (0x1    <<   0) );          // wrlvl_resync[0]=0x0
#endif

    ClearIO32( &pReg_Drex->WRLVL_CONFIG[0],     (0x1    <<   0) );          // odt_on[0]="0" (turn off)
    ClearIO32( &pReg_DDRPHY->LP_DDR_CON[4],     (0x3    <<   7) );          // cmd_default, ODT[8:7]=0x0


#if defined(MEM_TYPE_DDR3)
    MR1.MR1.WL      = 0;

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
    DMC_Delay(0x100);
#endif  // #if defined(MEM_TYPE_DDR3)


#if 0
    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x0 (LOW)
#if 0
    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
#endif
#endif


#if 1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<   0) );          // reg_mode[7:0]=0x1

    MEMMSG("ctrl_wrlat      = 0x%08X\r\n", (ReadIO32(&pReg_DDRPHY->PHY_CON[4]) >> 16) & 0x1F );
    MEMMSG("ctrl_wrlat_plus = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->PHY_CON[5]) );
    MEMMSG("WR_LVL_CON0     = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->WR_LVL_CON[0]) );
    MEMMSG("WR_LVL_CON1     = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->WR_LVL_CON[1]) );
    MEMMSG("WR_LVL_CON2     = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->WR_LVL_CON[2]) );
    MEMMSG("WR_LVL_CON3     = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->WR_LVL_CON[3]) );
    MEMMSG("CAL_WL_STAT     = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->CAL_WL_STAT) );

    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0xFF   <<   0) );          // reg_mode[7:0]=0x0
#endif

    MEMMSG("\r\n########## Write Leveling - End ##########\r\n");

    return ret;
}

#if (DDR_CA_CALIB_EN == 1)
//for LPDDR3
CBOOL DDR_CA_Calibration(void)
{
    CBOOL ret = CTRUE;
    U32 temp, mr41, mr48, vwml, vwmr, vwmc;
    U32 resp_mr41, resp_mr48;
    U32 keep_cacal_mode;
    U32 ch = 0;
    int find_vmw;
    U8  code;

    printf("\r\n########## CA Calibration - Start ##########\r\n");

    //*** keep_cacal_mode
    //-  "0" --> Disabing CA calibration mode after CA calibration is normally complete,
    //                : Set to "rdlvl_ca_en"=0
    //-  "1" --> Keep CA calibration mode after CA calibration is normally complete
    //                : Remain setting of "rdlvl_ca_en"=1

    //m_printf("\n\n Select Cal Mode after CA calibration is normally complete\n");
    //m_printf("     0. Stopping Cal Mode : change CMD SDLL Code to offsetd (Code = VWMC - T/4)\n");
    //m_printf("     1. Continuing Cal Mode(Code = VWMC)\n");
    //m_printf("\n▶ Enter : ");

    //keep_cacal_mode = cgetn();
    keep_cacal_mode = 1;                                            //- 0:Stopping Cal Mode,          1:Continuing Cal Mode(Code=VWMC)

//    if(PHY_address == 0x10c00000)           ch=0;
//    else if(PHY_address == 0x10c10000)      ch=1;


    //phyBase = 0x10c00000+(0x10000 * ch);
    //ioRdOffset = 0x150 + (0x4 * ch);
//    ioRdOffset = 0x150;
//#############
//      < Step1 >
//#############

    //*** Enabling "CA Calibration" for Controller
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],    (0x1    <<  16) );      // ctrl_wrlvl_en(wrlvl_mode)[16]="1" (Enable)

    //- ★ Enabling "CA Calibration Mode" for Controller
    //- Although CA calibration is normally complete, ENABLE or DISABLE of "rdlvl_ca_en" for normal operation representes whether CA calibration mode is being kept or not
    //- "Enable"  -> ctrl_offsetd=vwmc,    "Disable" -> ctrl_offsetd=vwmc - T/4 (T : DLL lock value for one period)
    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],    (0x1    <<  23) );      // rdlvl_ca_en(ca_cal_mode)[23]="1" (Enable)

    code = 0x8;                                                     // CMD SDLL Code default value "ctrl_offsetd"=0x8
    find_vmw = 0;
    vwml = vwmr = vwmc = 0;


//#if defined(POP_PKG)
#if 1
//*** Response for Issuing MR41 - CA Calibration Enter1
    //- CA Data Pattern transfered at Rising Edge   : CA[9:0]=0x3FF     => CA[8:5],CA[3:0] of Data Pattern transfered through MR41 is returned to DQ (CA[3:0]={DQ[6],DQ[4],DQ[2],DQ[0]}=0xF, CA[8:5]={DQ[14],DQ[12],DQ[10],DQ[8]}=0xF)
    //- CA Data Pattern transfered at Falling Edge  : CA[9:0]=0x000     => CA[8:5],CA[3:0] of Data Pattern transfered through MR41 is returned to DQ (CA[3:0]={DQ[7],DQ[5],DQ[3],DQ[1]}=0x0, CA[8:5]={DQ[15],DQ[13],DQ[11],DQ[9]}=0x0)
    //- So response(ctrl_io_rdata) from MR41 is "0x5555".
//*** Response for Issuing MR48 - CA Calibration Enter2
    //- CA Data Pattern transfered at Rising Edge   : CA[9], CA[4]=0x3  => CA[9],CA[4] of Data Pattern transfered through MR48 is returned to DQ (CA[9]=DQ[8]=0x1, CA[4]=DQ[0]}=0x1)
    //- CA Data Pattern transfered at Falling Edge  : CA[9], CA[4]=0x0  => CA[9],CA[4] of Data Pattern transfered through MR48 is returned to DQ (CA[9]=DQ[9]=0x0, CA[4]=DQ[1]}=0x0)
    //- So response(ctrl_io_rdata) from MR48 is "0x0101".
    resp_mr41 = 0x5555;
    resp_mr48 = 0x0101;
#else
    if ( ch == 0 ) {
        resp_mr41 = 0x69C5;
        resp_mr48 = 0x4040;
    } else {
        resp_mr41 = 0xD14E;
        resp_mr48 = 0x8008;
    }
#endif

    while (1)
    {

//#############
//  < Step2 >
//#############

        //*** Change CMD SDLL Code to start code value(0x8)
        printf("CA skew = %d\r\n", code );
        WriteIO32( &pReg_DDRPHY->PHY_CON[10],   code );

//#############
//  < Step3 >
//#############

        //*** Update CMD SDLL Code (ctrl_offsetd) : Make "ctrl_resync" HIGH and LOW
//        ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );
        SetIO32  ( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );  // ctrl_resync=0x1 (HIGH)
        ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );  // ctrl_resync=0x0 (LOW)
        DMC_Delay(0x1000);



#if 0
    SendDirectCommand(SDRAM_CMD_MRR, 0, 5, 0);
    temp = ReadIO32(&pReg_Drex->MRSTATUS);
    if (temp & 0x1)
    {
        printf("MRR5 = 0x%04X\r\n", temp );
        while(1);
    }
#endif



#if 0
    SendDirectCommand(SDRAM_CMD_MRR, 0, 8, 0);
    temp = ReadIO32( &pReg_Drex->MRSTATUS );
    if ( temp )
    {
        printf("MR8 = 0x%08X\r\n", temp);
        while(1);
    }
#endif




//#############
//  < Step4 >
//#############

#if 0
        SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#else
        if (pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#endif
#endif

        SendDirectCommand(SDRAM_CMD_MRS, 0, 41, 0xA4 );             //- CH0 : Send MR41 to start CA calibration for LPDDR3 : MA=0x29 OP=0xA4, 0x50690
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, 41, 0xA4);              //- CH1 : Send MR41 to start CA calibration for LPDDR3 : MA=0x29 OP=0xA4, 0x50690
#else
        if (pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, 41, 0xA4);          //- CH1 : Send MR41 to start CA calibration for LPDDR3 : MA=0x29 OP=0xA4, 0x50690
#endif
#endif

        temp = 0x003FF001 | (tADR << 4);
        WriteIO32( &pReg_Drex->CACAL_CONFIG[0], temp );             //- deassert_cke[0]=1 : CKE pin is "LOW"
//        SetIO32  ( &pReg_Drex->CACAL_CONFIG[0], (0x1    <<   0) );  // deassert_cke[0]=1 : CKE pin is "LOW"
        WriteIO32( &pReg_Drex->CACAL_CONFIG[1], 0x00000001 );       // cacal_csn[0]=1    : generate one pulse CSn(Low and High), cacal_csn field need not to return to "0" and whenever this field is written in "1", one pulse is genrerated.
        DMC_Delay(0x80);

//#############
//  < Step5 >
//#############

#if 0
        mr41 = Inp32( DREX_address + ioRdOffset );
        mr41 &= 0xFFFF;
#else
        mr41 = ReadIO32( &pReg_Drex->CTRL_IO_RDATA ) & 0xFFFF;
//        mr41 = ReadIO32( &pReg_Drex->CTRL_IO_RDATA );
//        if (mr41)
//            printf("mr41 = 0x%08X\r\n", mr41);
#endif

        ClearIO32( &pReg_Drex->CACAL_CONFIG[0], (0x1    <<   0) );  // assert_cke[0]=0 : Normal operation
        DMC_Delay(0x80);

//#############
//  < Step6 >
//#############

#if 0
        SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#else
        if (pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#endif
#endif

        SendDirectCommand(SDRAM_CMD_MRS, 0, 48, 0xC0);              // CH0 : Send MR48 to start CA calibration for LPDDR3 : MA=0x30 OP=0xC0, 0x60300
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, 48, 0xC0);              // CH1 : Send MR48 to start CA calibration for LPDDR3 : MA=0x30 OP=0xC0, 0x60300
#else
        if (pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, 48, 0xC0);          // CH1 : Send MR48 to start CA calibration for LPDDR3 : MA=0x30 OP=0xC0, 0x60300
#endif
#endif

        temp = 0x003FF001 | (tADR << 4);
        WriteIO32( &pReg_Drex->CACAL_CONFIG[0], temp );             //- deassert_cke[0]=1 : CKE pin is "LOW"
//        SetIO32  ( &pReg_Drex->CACAL_CONFIG[0], (0x1    <<   0) );  // deassert_cke[0]=1 : CKE pin is "LOW"
        WriteIO32( &pReg_Drex->CACAL_CONFIG[1], 0x00000001 );       // cacal_csn[0]=1    : generate one pulse CSn(Low and High), cacal_csn field need not to return to "0" and whenever this field is written in "1", one pulse is genrerated.
        DMC_Delay(0x80);

//#############
//  < Step7 >
//#############

#if 0
        mr48 = Inp32( DREX_address + ioRdOffset );

//#if defined(POP_PKG)
#if 1
        mr48 &= 0x0303;
#else
        if ( ch == 0 ) {
            mr48 &= 0xC060;
        } else {
            mr48 &= 0x8418;
        }
#endif
#else
        mr48 = ReadIO32( &pReg_Drex->CTRL_IO_RDATA ) & 0x0303;
//        mr48 = ReadIO32( &pReg_Drex->CTRL_IO_RDATA );
//        if (mr48)
//            printf("mr48 = 0x%08X\r\n", mr48);
#endif

        ClearIO32( &pReg_Drex->CACAL_CONFIG[0], (0x1    <<   0) );  // deassert_cke[0]=0 : Normal operation
        DMC_Delay(0x80);

//#############
//  < Step8 >
//#############

        //* CA Cal 첫 번재 PASS부터  연속 3회 PASS이면 PASS인 것으로 간주하고,
        //  연속 3회 PASS인 경우의 첫 번째 PASS일 때의 code가 vwml값이 됨.
        //* To find "VWML", if consecutive PASS is more than three times, consider as first PASS and keep searching right margin until failure occurs.
        //- First one of consecutive three times PASS is "VWML"

#if 1
        if (mr41 || mr48)
            printf("mr41 = 0x%08X, mr48 = 0x%08X\r\n", mr41, mr48);
#endif

        //*** Check if first consecutive PASS is three times
        if( (find_vmw < 0x3) && (mr41 == resp_mr41) && (mr48 == resp_mr48) )
        {
            find_vmw++;
            if(find_vmw == 0x1)
            {
                vwml = code;
            }
        }
        else if( ((find_vmw > 0x0) && (find_vmw < 0x3)) && ((mr41 != resp_mr41) || (mr48 != resp_mr48)) )
        {
            find_vmw = 0x0;                                 //- 첫 번째 PASS로부터 연속 3회 PASS 하지 못하면 연속 3회 PASS가 발생할 때까지 Searching 다시 시작하도록 "find_vmw" = "0"으로 초기화.
        }

        //*** Finding rightmost code value. "VWMR" is the same as code value right before failure on searching
        if( (find_vmw == 0x3) && ((mr41 != resp_mr41) || (mr48 != resp_mr48)) )
        {
            find_vmw = 0x4;
            vwmr = code - 1;
            break;
        }

//#############
//  < Step9 >
//#############

        //*** Increase CMD SDLL Code  by "1"
        code++;

DMC_Delay(0x10000 * 10);

        //*** If code value is under 256, and then go to Step2 to update CMD SDLL Code
        //*** Otherwise, execute the adequate flow for error status.

        //*** CMD SDLL Code : ctrl_offsetd[7:0] is made of total 8-bit, so maximum value is 255.
        //*** Therefore, offset value more than 255 should be considered  as ★error
        //*** The code below denotes right sequence for managing error case.

        if (code == 255)
        {
            WriteIO32( &pReg_DDRPHY->PHY_CON[10],   0x8 );

            //*** CA calibration is failure!
            ret = CFALSE;
            goto ca_error_ret;
        }//- End of "if(code == 255)"
    }//- End of "while(1)"

//##################
//  < Step10 >,< Step11>
//##################

    vwmc = (vwml + vwmr) / 2;
    printf(" \n★ CH%d : CA Calibration ▶ VWMC = (VWML + VWMR)/2 = (%d + %d)/2 = %d \n", ch, vwml, vwmr, vwmc);


    //*** Convert "VWMC" to "ctrl_offsetd" value to apply "Disable Mode" of CA calibration after normal completion of CA calibration.
//    gDDR_Lock = (ReadIO32( &pReg_DDRPHY->PHY_CON[13]) >> 8) & 0x1FF;

//    lock=PHY_GetDllLockValue( ch );
    code = vwmc - (gDDR_Lock >> 2);                                 //- (lock >> 2) means "T/4", lock value means the number of delay cell for one period
    printf(" \n★ [CH%d] Lock Value : %d ,  VWMC : %d", ch, gDDR_Lock, vwmc);
    printf(" \n★ CH%d : CMD SDLL Code(with disabling CA Cal) = offsetd = VWMC - T/4(%d) = %d\n", ch, (gDDR_Lock>>2), code);

    if (keep_cacal_mode)
    {
        WriteIO32( &pReg_DDRPHY->PHY_CON[10],   vwmc );
    }
    else
    {
        WriteIO32( &pReg_DDRPHY->PHY_CON[10],   code );
    }

ca_error_ret:

WriteIO32( &pReg_DDRPHY->PHY_CON[10],   144 );

    //*** Update CMD SDLL Code (ctrl_offsetd) : Make "ctrl_resync" HIGH and LOW
//    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );      // ctrl_resync[24]=0x0 (LOW)
    SetIO32  ( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );      // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );      // ctrl_resync[24]=0x0 (LOW)

    //※ When CA calibration is only progressing, ctrl_wrlvl_en[16] should be "1"
    //※ If CA calibration is complete, set ctrl_wrlvl_en[16] to "0" at last step.
    ClearIO32( &pReg_DDRPHY->PHY_CON[0],    (0x1    <<  16) );      // ctrl_wrlvl_en(wrlvl_mode)[16]="0"(Disable)

    //*** Disabling CA Calibration Mode for Controller : rdlvl_ca_en[23] = "0"(Disable), rdlvl_ca_en = "1"(Enable)
    ClearIO32( &pReg_DDRPHY->PHY_CON[2],    (0x1    <<  23) );      // rdlvl_ca_en(ca_cal_mode)[23]="0"(Disable)


//#############
//  < Step12 >
//#############

    //*** Exiting Calibration Mode of LPDDR3 using MR42
    SendDirectCommand(SDRAM_CMD_MRS, 0, 42, 0xA8);                  // CH0 : Send MR42 to exit from CA calibration mode for LPDDR3, MA=0x2A OP=0xA8, 0x50AA0
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, 42, 0xA8);                  //- CH1 : Send MR42 to exit from CA calibration mode for LPDDR3, MA=0x2A OP=0xA8, 0x50AA0
#else
    if (pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, 42, 0xA8);              //- CH1 : Send MR42 to exit from CA calibration mode for LPDDR3, MA=0x2A OP=0xA8, 0x50AA0
#endif
#endif

    printf("\r\n########## CA Calibration - End ##########\r\n");

    return ret;
}
#endif  // #if (DDR_CA_CALIB_EN == 1)

CBOOL DDR_Gate_Leveling(void)
{
#if defined(MEM_TYPE_DDR3)
    union SDRAM_MR MR;
#endif
    volatile U32 cal_count = 0;
    U32     temp;
    CBOOL   ret = CTRUE;

    MEMMSG("\r\n########## Gate Leveling - Start ##########\r\n");

#if 1
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#endif


#if defined(MEM_TYPE_DDR3)
    /* Set MPR mode enable */
    MR.Reg          = 0;
    MR.MR3.MPR      = 1;

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif  // #if defined(MEM_TYPE_DDR3)


    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<  24) );          // gate_cal_mode[24] = 1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x5    <<   6) );          // ctrl_shgate[8]=1, ctrl_atgate[6]=1
    ClearIO32( &pReg_DDRPHY->PHY_CON[1],        (0xF    <<  20) );          // ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9

    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  18) );          // gate_lvl_start[18] = 1
//    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  18) );          // gate_lvl_start[18] = 1

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        temp = ReadIO32( &pReg_DDRPHY->PHY_CON[3] );
        if ( temp & (0x1 << 26) )                                           // rd_wr_cal_resp[26] : Wating until GATE calibration is complete
        {
            break;
        }

        DMC_Delay(100);
    }

//    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        0x0 );                      // gate_lvl_start[18]=0 : Stopping it after completion of GATE leveling.
    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  18) );          // gate_lvl_start[18]=0 : Stopping it after completion of GATE leveling.

    //------------------------------------------------------------------------------------------------------------------------

    if (cal_count == 100)
    {
        MEMMSG("GATE: Calibration Responese Checking : fail...!!!\r\n");

        ret = CFALSE;                                               // Failure Case
        goto gate_err_ret;
    }

    //------------------------------------------------------------------------------------------------------------------------

    g_GT_code   = ReadIO32( &pReg_DDRPHY->CAL_GT_VWMC[0] );
    g_GT_cycle  = ReadIO32( &pReg_DDRPHY->CAL_GT_CYC );
//    g_GT_cycle  = ReadIO32( &pReg_DDRPHY->CAL_GT_CYC ) + 0x492;

//    MEMMSG("CAL_FAIL_STAT0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) );

//    MEMMSG("GT VWMC0 = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[0]) );
    MEMMSG("GT VWMC0 = 0x%08X\r\n", g_GT_code );
    MEMMSG("GT VWMC1 = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[1]) );
    MEMMSG("GT VWMC2 = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->CAL_GT_VWMC[2]) );

//    MEMMSG("GT CYC   = 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->CAL_GT_CYC) );
    MEMMSG("GT CYC   = 0x%08X\r\n", g_GT_cycle );

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

    WriteIO32( &pReg_DDRPHY->LP_CON,            0x0 );                      // ctrl_pulld_dqs[8:0] = 0
    ClearIO32( &pReg_DDRPHY->RODT_CON,          (0x1    <<  16) );          // ctrl_read_dis[16] = 0


#if defined(MEM_TYPE_DDR3)
    /* Set MPR mode disable */
    MR.Reg          = 0;

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif  // #if defined(MEM_TYPE_DDR3)

    MEMMSG("\r\n########## Gate Leveling - End ##########\r\n");

//    if ( pSBI->FlyBy_Mode && (g_GT_code == 0x08080808) )
    if (g_GT_code == 0x08080808)
        ret = CFALSE;

    return ret;
}

CBOOL DDR_Read_DQ_Calibration(void)
{
#if defined(MEM_TYPE_DDR3)
    union SDRAM_MR MR;
#endif
    volatile U32 cal_count = 0;
    U32     temp;
    CBOOL   ret = CTRUE;


    MEMMSG("\r\n########## Read DQ Calibration - Start ##########\r\n");

#if (CFG_8BIT_DESKEW == 1)
    ClearIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  13) );          // byte_rdlvl_en[13]=0, for Deskewing
#endif

#if 1
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#endif


#if defined(MEM_TYPE_DDR3)
    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) & 0xFFFF0000;
    temp |= 0xFF00;                                                         // rdlvl_rddata_adj[15:0]
//    temp |= 0x0100;                                                         // rdlvl_rddata_adj[15:0]
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],    temp );

    /* Set MPR mode enable */
    MR.Reg          = 0;
    MR.MR3.MPR      = 1;

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif

    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[2],
        (0x0    << 16) |    // [31:16] ddr3_default
        (0x0    <<  1) |    // [15: 1] ddr3_address
        (0x0    <<  0) );   // [    0] ca_swap_mode
#endif  // #if defined(MEM_TYPE_DDR3)

    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<  25) );          // rd_cal_mode[25]=1

    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  19) );          // rd_cal_start[19]=1 : Starting READ calibration
//    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  19) );              // rd_cal_start[19]=1 : Starting READ calibration

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        temp = ReadIO32( &pReg_DDRPHY->PHY_CON[3] );
        if ( temp & (0x1 << 26) )                                           // rd_wr_cal_resp[26] : Wating until READ calibration is complete
        {
            break;
        }

        DMC_Delay(100);
    }
//    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  26) );

//    WriteIO32( &pReg_DDRPHY->PHY_CON[3],        0x0 );                      // rd_cal_start[19]=0 : Stopping it after completion of Read calibration.
    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  19) );          // rd_cal_start[19]=0 : Stopping it after completion of Read calibration.

    //------------------------------------------------------------------------------------------------------------------------

    if (cal_count == 100)                                                   // Failure Case
    {
        MEMMSG("RD DQ CAL Status Checking error\r\n");

        ret = CFALSE;
        goto rd_err_ret;
    }

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        if ( ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) == 0 )
        {
            break;
        }

        DMC_Delay(100);
    }

    if (cal_count == 100)
    {
        MEMMSG("RD DQ: CAL_FAIL_STATUS Checking : fail...!!!\r\n");

        ret = CFALSE;                                                       // Failure Case
        goto rd_err_ret;
    }

    //------------------------------------------------------------------------------------------------------------------------

    g_RD_vwmc = ReadIO32( &pReg_DDRPHY->CAL_RD_VWMC[0] );

    //------------------------------------------------------------------------------------------------------------------------

#if 1
{
    U32 i;

    MEMMSG("\r\n\n");
    for(i = 0; i < 8; i++)
        MEMMSG("RD%d RD_DESKEW_CON  = 0x%08x\r\n", i, ReadIO32(&pReg_DDRPHY->RD_DESKEW_CON[(i*3)]) );

    MEMMSG("\r\n");
    MEMMSG("RD DQ CAL_RD_VWMC0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_RD_VWMC[0]) );
    MEMMSG("RD DQ CAL_RD_VWML0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_RD_VWML[0]) );
    MEMMSG("RD DQ CAL_RD_VWMR0  = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_RD_VWMR[0]) );

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
    MR.Reg          = 0;

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif

    MEMMSG("\r\n########## Read DQ Calibration - End ##########\r\n");

    return ret;
}

CBOOL DDR_Write_Latency_Calibration(void)
{
    volatile U32 cal_count = 0;
    U32     temp;
    CBOOL   ret = CTRUE;

    MEMMSG("\r\n########## Write Latency Calibration - Start ##########\r\n");

#if (CFG_8BIT_DESKEW == 1)
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  13) );          // byte_rdlvl_en[13]=1
#endif


    // Set issue active command.
    WriteIO32( &pReg_Drex->WRTRA_CONFIG,
        (0x0    << 16) |    // [31:16] row_addr
        (0x0    <<  1) |    // [ 3: 1] bank_addr
        (0x1    <<  0) );   // [    0] write_training_en

    ClearIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  14) );          // p0_cmd_en[14] = 0
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  14) );          // p0_cmd_en[14] = 1


    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  20) );          // wl_cal_mode[20] = 1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  21) );          // wl_cal_start[21] = 1

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        temp = ReadIO32( &pReg_DDRPHY->PHY_CON[3] );
        if ( temp & (0x1 << 27) )                                           // wl_cal_resp[27] : Wating until WRITE LATENCY calibration is complete
        {
            break;
        }

        DMC_Delay(0x100);
    }

    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  21) );          // wl_cal_start[21] = 0
    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x1    <<  20) );          // wl_cal_mode[20] = 0
//    ClearIO32( &pReg_DDRPHY->PHY_CON[3],        (0x3    <<  20) );          // wl_cal_start[21] = 0, wl_cal_mode[20] = 0

    ClearIO32( &pReg_Drex->WRTRA_CONFIG,        (0x1    <<   0) );          // write_training_en[0] = 0

    //------------------------------------------------------------------------------------------------------------------------

    if (cal_count == 100)                                                   // Failure Case
    {
        MEMMSG("WR Latency CAL Status Checking error\r\n");

        ret = CFALSE;
    }

    //------------------------------------------------------------------------------------------------------------------------

    MEMMSG("\r\n########## Write Latency Calibration - End ##########\r\n");

    return ret;
}

CBOOL DDR_Write_DQ_Calibration(void)
{
    volatile U32 cal_count = 0;
    U32     temp;
    CBOOL   ret = CTRUE;

    MEMMSG("\r\n########## Write DQ Calibration - Start ##########\r\n");

#if (CFG_8BIT_DESKEW == 1)
    ClearIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  13) );          // byte_rdlvl_en[13]=0, for Deskewing
#endif

    // Set issue active command.
    WriteIO32( &pReg_Drex->WRTRA_CONFIG,
        (0x0    << 16) |    // [31:16] row_addr
        (0x0    <<  1) |    // [ 3: 1] bank_addr
        (0x1    <<  0) );   // [    0] write_training_en

    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<  26) );          // wr_cal_mode[26] = 1, Write Training mode
    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<  27) );          // wr_cal_start[27] = 1

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        temp = ReadIO32( &pReg_DDRPHY->PHY_CON[3] );
        if ( temp & (0x1 << 26) )                                           // rd_wr_cal_resp[26] : Wating until WRITE calibration is complete
        {
            break;
        }

        DMC_Delay(0x100);
    }

    ClearIO32( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<  27) );          // wr_cal_start[27] = 0
//    ClearIO32( &pReg_DDRPHY->PHY_CON[2],        (0x3    <<  26) );          // wr_cal_start[27] = 0, wr_cal_mode[26] = 0

    ClearIO32( &pReg_Drex->WRTRA_CONFIG,        (0x1    <<   0) );          // write_training_en[0] = 0

    //------------------------------------------------------------------------------------------------------------------------

    if (cal_count == 100)                                                   // Failure Case
    {
        MEMMSG("WR DQ CAL Status Checking error\r\n");

        ret = CFALSE;
        goto wr_err_ret;
    }

    for (cal_count = 0; cal_count < 100; cal_count++)
    {
        if ( (ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[0]) | ReadIO32(&pReg_DDRPHY->CAL_FAIL_STAT[3])) == 0 )
        {
            break;
        }

        DMC_Delay(100);
    }

    if (cal_count == 100)
    {
        MEMMSG("WR DQ: CAL_FAIL_STATUS Checking : fail...!!!\r\n");

        ret = CFALSE;                                                       // Failure Case
        goto wr_err_ret;
    }

    //------------------------------------------------------------------------------------------------------------------------

    g_WR_vwmc = ReadIO32( &pReg_DDRPHY->CAL_WR_VWMC[0] );

    //------------------------------------------------------------------------------------------------------------------------
#if 1
{
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
    MEMMSG("WR CAL_DM_VWMC0 = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_DM_VWMC[0]) );
    MEMMSG("WR CAL_DM_VWML0 = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_DM_VWML[0]) );
    MEMMSG("WR CAL_DM_VWMR0 = 0x%08x\r\n", ReadIO32(&pReg_DDRPHY->CAL_DM_VWMR[0]) );

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
#endif  // #if (SKIP_LEVELING_TRAINING == 0)


U32 getVWMC_Offset(U32 code, U32 lock_div4)
{
    U32 i, ret_val;
    U8  vwmc[4];
    int offset[4];

    for (i = 0; i < 4; i++)
    {
        vwmc[i]   = ((code >> (8*i)) & 0xFF);
    }

    for (i = 0; i < 4; i++)
    {
        offset[i] = (int)(vwmc[i] - lock_div4);
        if (offset[i] < 0)
        {
            offset[i] *= -1;
            offset[i] |= 0x80;
        }
    }

    ret_val = ( ((U8)offset[3] << 24) | ((U8)offset[2] << 16) | ((U8)offset[1] << 8) | (U8)offset[0] );

    return ret_val;
}


CBOOL init_DDR3(U32 isResume)
{
    union SDRAM_MR MR0, MR1, MR2, MR3;
    U32 DDR_AL, DDR_WL, DDR_RL;
    U32 temp;


//    printf("\r\nDDR3 POR Init Start\r\n");

    // Step 1. reset (Min : 10ns, Typ : 200us)
#if 0
    ResetCon(RESETINDEX_OF_DREX_MODULE_CRESETn, CTRUE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_ARESETn, CTRUE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_nPRST,   CTRUE);
    DMC_Delay(0x100);                           // wait 300ms
    ResetCon(RESETINDEX_OF_DREX_MODULE_CRESETn, CFALSE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_ARESETn, CFALSE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_nPRST,   CFALSE);
    DMC_Delay(0x1000);                          // wait 300ms

    ResetCon(RESETINDEX_OF_DREX_MODULE_CRESETn, CTRUE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_ARESETn, CTRUE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_nPRST,   CTRUE);
    DMC_Delay(0x100);                           // wait 300ms
    ResetCon(RESETINDEX_OF_DREX_MODULE_CRESETn, CFALSE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_ARESETn, CFALSE);
    ResetCon(RESETINDEX_OF_DREX_MODULE_nPRST,   CFALSE);
    DMC_Delay(0x1000);                          // wait 300ms
    DMC_Delay(0xF000);
#else

    ClearIO32( &pReg_RstCon->REGRST[0],     (0x7    <<  26) );
    DMC_Delay(0x1000);                              // wait 300ms
    SetIO32  ( &pReg_RstCon->REGRST[0],     (0x7    <<  26) );
    DMC_Delay(0x1000);                              // wait 300ms
    ClearIO32( &pReg_RstCon->REGRST[0],     (0x7    <<  26) );
    DMC_Delay(0x1000);                              // wait 300ms
    SetIO32  ( &pReg_RstCon->REGRST[0],     (0x7    <<  26) );
//    DMC_Delay(0x10000);                             // wait 300ms

#if 0
    ClearIO32( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
    DMC_Delay(0x1000);                              // wait 300ms
    SetIO32  ( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
    DMC_Delay(0x1000);                              // wait 300ms
    ClearIO32( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
    DMC_Delay(0x1000);                              // wait 300ms
    SetIO32  ( &pReg_Tieoff->TIEOFFREG[3],  (0x1    <<  31) );
#endif
    DMC_Delay(0x10000);                             // wait 300ms
#endif

    while( ReadIO32(&pReg_DDRPHY->SHIFTC_CON) != 0x0492 )
    {
        DMC_Delay(1000);
    }
    MEMMSG("PHY Version: 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->VERSION_INFO));

#if (CFG_NSIH_EN == 0)
    //pSBI->LvlTr_Mode    = ( LVLTR_WR_LVL | LVLTR_CA_CAL | LVLTR_GT_LVL | LVLTR_RD_CAL | LVLTR_WR_CAL );
    //pSBI->LvlTr_Mode    = ( LVLTR_GT_LVL | LVLTR_RD_CAL | LVLTR_WR_CAL );
    pSBI->LvlTr_Mode    = LVLTR_GT_LVL;
    //pSBI->LvlTr_Mode    = 0;
#endif

#if (CFG_NSIH_EN == 0)
#if 1   // Common
    pSBI->DDR3_DSInfo.MR2_RTT_WR    = 2;    // RTT_WR - 0: ODT disable, 1: RZQ/4, 2: RZQ/2
    pSBI->DDR3_DSInfo.MR1_ODS       = 1;    // ODS - 00: RZQ/6, 01 : RZQ/7
    pSBI->DDR3_DSInfo.MR1_RTT_Nom   = 2;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8

    pSBI->PHY_DSInfo.DRVDS_Byte3    = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte2    = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte1    = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_Byte0    = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_CK       = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_CKE      = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_CS       = PHY_DRV_STRENGTH_240OHM;
    pSBI->PHY_DSInfo.DRVDS_CA       = PHY_DRV_STRENGTH_240OHM;

    pSBI->PHY_DSInfo.ZQ_DDS         = PHY_DRV_STRENGTH_48OHM;
    pSBI->PHY_DSInfo.ZQ_ODT         = PHY_DRV_STRENGTH_120OHM;
#endif

#if 0   // DroneL 720Mhz
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

#if 0   // DroneL 800Mhz
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

    DDR_AL = 0;
#if (CFG_NSIH_EN == 0)
    if (MR1_nAL > 0)
        DDR_AL = nCL - MR1_nAL;

    DDR_WL = (DDR_AL + nCWL);
    DDR_RL = (DDR_AL + nCL);
#else
    if (pSBI->DII.MR1_AL > 0)
        DDR_AL = pSBI->DII.CL - pSBI->DII.MR1_AL;

    DDR_WL = (DDR_AL + pSBI->DII.CWL);
    DDR_RL = (DDR_AL + pSBI->DII.CL);
#endif

    if (isResume == 0)
    {
        MR2.Reg         = 0;
        MR2.MR2.RTT_WR  = pSBI->DDR3_DSInfo.MR2_RTT_WR;
        MR2.MR2.SRT     = 0; // self refresh normal range
        MR2.MR2.ASR     = 0; // auto self refresh disable
#if (CFG_NSIH_EN == 0)
        MR2.MR2.CWL     = (nCWL - 5);
#else
        MR2.MR2.CWL     = (pSBI->DII.CWL - 5);
#endif

        MR3.Reg         = 0;
        MR3.MR3.MPR     = 0;
        MR3.MR3.MPR_RF  = 0;

        MR1.Reg         = 0;
        MR1.MR1.DLL     = 0;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
        MR1.MR1.AL      = MR1_nAL;
#else
        MR1.MR1.AL      = pSBI->DII.MR1_AL;
#endif
        MR1.MR1.ODS1        = pSBI->DDR3_DSInfo.MR1_ODS & (1 << 1);
        MR1.MR1.ODS0        = pSBI->DDR3_DSInfo.MR1_ODS & (1 << 0);
        MR1.MR1.RTT_Nom2    = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 2);
        MR1.MR1.RTT_Nom1    = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 1);
        MR1.MR1.RTT_Nom0    = pSBI->DDR3_DSInfo.MR1_RTT_Nom & (1 << 0);
        MR1.MR1.QOff    = 0;
        MR1.MR1.WL      = 0;
#if 0
#if (CFG_NSIH_EN == 0)
        MR1.MR1.TDQS    = (_DDR_BUS_WIDTH>>3) & 1;
#else
        MR1.MR1.TDQS    = (pSBI->DII.BusWidth>>3) & 1;
#endif
#endif

#if (CFG_NSIH_EN == 0)
        if (nCL > 11)
            temp = ((nCL-12) << 1) + 1;
        else
            temp = ((nCL-4) << 1);
#else
        if (pSBI->DII.CL > 11)
            temp = ((pSBI->DII.CL-12) << 1) + 1;
        else
            temp = ((pSBI->DII.CL-4) << 1);
#endif

        MR0.Reg         = 0;
        MR0.MR0.BL      = 0;
        MR0.MR0.BT      = 1;
        MR0.MR0.CL0     = (temp & 0x1);
        MR0.MR0.CL1     = ((temp>>1) & 0x7);
        MR0.MR0.DLL     = 0;//1;
#if (CFG_NSIH_EN == 0)
        MR0.MR0.WR      = MR0_nWR;
#else
        MR0.MR0.WR      = pSBI->DII.MR0_WR;
#endif
        MR0.MR0.PD      = 0;//1;
    }   // if (isResume == 0)

#if 1    // ddr simulation
// Step 2. Select Memory type : DDR3
// Check DDR3 MPR data and match it to PHY_CON[1]??
    temp = (
        (0x17   <<  24) |           // [28:24] T_WrWrCmd
        (0x1    <<  22) |           // [23:22] ctrl_upd_mode. DLL Update control 0:always, 1: depending on ctrl_flock, 2: depending on ctrl_clock, 3: don't update
        (0x0    <<  20) |           // [21:20] ctrl_upd_range
#if (CFG_NSIH_EN == 0)
#if (tWTR == 3)     // 6 cycles
        (0x7    <<  17) |           // [19:17] T_WrRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#elif (tWTR == 2)   // 4 cycles
        (0x6    <<  17) |           // [19:17] T_WrRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#endif
#endif
        (0x0    <<  16) |           // [   16] wrlvl_mode. Write Leveling Enable. 0:Disable, 1:Enable
        (0x0    <<  14) |           // [   14] p0_cmd_en. 0:Issue Phase1 Read command during Read Leveling. 1:Issue Phase0
        (0x0    <<  13) |           // [   13] byte_rdlvl_en. Read Leveling 0:Disable, 1:Enable
        (0x1    <<  11) |           // [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3, 2:LPDDR2, 3:LPDDR3
        (0x1    <<  10) |           // [   10] ctrl_wr_dis. Write ODT Disable Signal during Write Calibration. 0: not change, 1: disable
        (0x1    <<   9) |           // [    9] ctrl_dfdqs. 0:Single-ended DQS, 1:Differential DQS
//        (0x1    <<   8) |           // [    8] ctrl_shgate. 0:Gate signal length=burst length/2+N, 1:Gate signal length=burst length/2-1
        (0x1    <<   6) |           // [    6] ctrl_atgate
        (0x0    <<   4) |           // [    4] ctrl_cmosrcv
        (0x0    <<   3) |           // [    3] ctrl_twpre
        (0x0    <<   0));           // [ 2: 0] ctrl_fnc_fb. 000:Normal operation.

#if (CFG_NSIH_EN == 1)
    if ((pSBI->DII.TIMINGDATA >> 28) == 3)      // 6 cycles
        temp |= (0x7    <<  17);
    else if ((pSBI->DII.TIMINGDATA >> 28) == 2) // 4 cycles
        temp |= (0x6    <<  17);
#endif

    WriteIO32( &pReg_DDRPHY->PHY_CON[0],    temp );

#if 0
    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,   (0x1    <<  28) );  // upd_mode[28]=1, DREX-initiated Update Mode
//    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,   (0x1    <<  28) );  // upd_mode[28]=0, PHY-initiated Update Mode
#endif

#if 1
#if defined(MEM_TYPE_DDR3)
    temp  = ReadIO32( &pReg_DDRPHY->LP_DDR_CON[3]) & ~0x3FFF;
    temp |= 0x105E;                                             // cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[3], temp);

    temp  = ReadIO32( &pReg_DDRPHY->LP_DDR_CON[4]) & ~0x3FFF;
    temp |= 0x107F;                                             // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[4], temp);
#endif  // #if defined(MEM_TYPE_DDR3)

#if defined(MEM_TYPE_LPDDR23)
    temp  = ReadIO32( &pReg_DDRPHY->LP_DDR_CON[3]) & ~0x3FFF;
    temp |= 0x000E;                                             // cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[3], temp);

    temp  = ReadIO32( &pReg_DDRPHY->LP_DDR_CON[4]) & ~0x3FFF;
    temp |= 0x000F;                                             // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[4], temp);
#endif  // #if defined(MEM_TYPE_LPDDR23)
#endif

    printf("phy init\r\n");

    /* Set WL, RL, BL */
    WriteIO32( &pReg_DDRPHY->PHY_CON[4],
        (DDR_WL << 16) |            // [20:16] T_wrdata_en (WL+1)
        (0x8    <<  8) |            // [12: 8] Burst Length(BL)
        (DDR_RL <<  0));            // [ 4: 0] Read Latency(RL), 800MHz:0xB, 533MHz:0x5

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

    WriteIO32( &pReg_DDRPHY->DRVDS_CON[0],      0x00 );
    WriteIO32( &pReg_DDRPHY->DRVDS_CON[1],      0x00 );
#endif

    // Driver Strength(zq_mode_dds), zq_clk_div_en[18]=Enable
    WriteIO32( &pReg_DDRPHY->ZQ_CON,
        (0x1    <<  27) |                       // [   27] zq_clk_en. ZQ I/O clock enable.
        (pSBI->PHY_DSInfo.ZQ_DDS  <<  24) |     // [26:24] zq_mode_dds, Driver strength selection. 100 : 48ohm, 101 : 40ohm, 110 : 34ohm, 111 : 30ohm
        (pSBI->PHY_DSInfo.ZQ_ODT  <<  21) |     // [23:21] ODT resistor value. 001 : 120ohm, 010 : 60ohm, 011 : 40ohm, 100 : 30ohm
        (0x0    <<  20) |                       // [   20] zq_rgddr3. GDDR3 mode. 0:Enable, 1:Disable
        (0x0    <<  19) |                       // [   19] zq_mode_noterm. Termination. 0:Enable, 1:Disable
        (0x1    <<  18) |                       // [   18] zq_clk_div_en. Clock Dividing Enable : 0, Disable : 1
        (0x0    <<  15) |                       // [17:15] zq_force-impn
//        (0x7    <<  12) |                       // [14:12] zq_force-impp
        (0x0    <<  12) |                       // [14:12] zq_force-impp
        (0x30   <<   4) |                       // [11: 4] zq_udt_dly
        (0x1    <<   2) |                       // [ 3: 2] zq_manual_mode. 0:Force Calibration, 1:Long cali, 2:Short cali
        (0x0    <<   1) |                       // [    1] zq_manual_str. Manual Calibration Stop : 0, Start : 1
        (0x0    <<   0));                       // [    0] zq_auto_en. Auto Calibration enable

    SetIO32( &pReg_DDRPHY->ZQ_CON,          (0x1    <<   1) );          // zq_manual_str[1]. Manual Calibration Start=1
    while( ( ReadIO32( &pReg_DDRPHY->ZQ_STATUS ) & 0x1 ) == 0 );        //- PHY0: wait for zq_done
    ClearIO32( &pReg_DDRPHY->ZQ_CON,        (0x1    <<   1) );          // zq_manual_str[1]. Manual Calibration Stop : 0, Start : 1

    ClearIO32( &pReg_DDRPHY->ZQ_CON,        (0x1    <<  18) );          // zq_clk_div_en[18]. Clock Dividing Enable : 1, Disable : 0



    // Step 5. dfi_init_start : High
    WriteIO32( &pReg_Drex->CONCONTROL,
        (0xFFF  <<  16) |   // [27:16] time out level0
        (0x3    <<  12) |   // [14:12] read data fetch cycles - n cclk cycles (cclk: DREX core clock)
//        (0x1    <<   5) |   // [  : 5] afre_en. Auto Refresh Counter. Disable:0, Enable:1
//        (0x1    <<   4) |   // nexell: 0:ca swap bit, 4 & samsung drex/phy initiated bit
        (0x0    <<   1)     // [ 2: 1] aclk:cclk = 1:1
        );

    SetIO32  ( &pReg_Drex->CONCONTROL,      (0x1    <<  28) );          // dfi_init_start[28]. DFI PHY initialization start
    while( (ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1<<3) ) == 0);       // dfi_init_complete[3]. wait for DFI PHY initialization complete
    ClearIO32( &pReg_Drex->CONCONTROL,      (0x1    <<  28) );          // dfi_init_start[28]. DFI PHY initialization clear

    // Step 3. Set the PHY for dqs pull down mode
    WriteIO32( &pReg_DDRPHY->LP_CON,
        (0x0    <<  16) |       // [24:16] ctrl_pulld_dq
        (0xF    <<   0));       // [ 8: 0] ctrl_pulld_dqs.  No Gate leveling : 0xF, Use Gate leveling : 0x0(X)

    WriteIO32( &pReg_DDRPHY->RODT_CON,
        (0x0    <<  28) |       // [31:28] ctrl_readduradj
        (0x1    <<  24) |       // [27:24] ctrl_readadj
        (0x1    <<  16) |       // [  :16] ctrl_read_dis
        (0x0    <<   0) );      // [  : 0] ctrl_read_width

    // Step 8 : Update DLL information
    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization


    // Step 11. MemBaseConfig
    WriteIO32( &pReg_DrexTZ->MEMBASECONFIG[0],
        (0x040      <<  16) |                   // chip_base[26:16]. AXI Base Address. if 0x20 ==> AXI base addr of memory : 0x2000_0000
#if (CFG_NSIH_EN == 0)
        (chip_mask  <<   0));                   // 256MB:0x7F0, 512MB: 0x7E0, 1GB:0x7C0, 2GB: 0x780, 4GB:0x700
#else
        (pSBI->DII.ChipMask <<   0));
#endif

#if (CFG_NSIH_EN == 0)
    WriteIO32( &pReg_DrexTZ->MEMBASECONFIG[1],
        (chip_base1 <<  16) |               // chip_base[26:16]. AXI Base Address. if 0x40 ==> AXI base addr of memory : 0x4000_0000, 16MB unit
        (chip_mask  <<   0));               // chip_mask[10:0]. 2048 - chip size
#else
    temp = (0x40 + pSBI->DII.ChipSize);
    WriteIO32( &pReg_DrexTZ->MEMBASECONFIG[1],
        (temp               <<  16) |       // chip_base[26:16]. AXI Base Address. if 0x40 ==> AXI base addr of memory : 0x4000_0000, 16MB unit
        (pSBI->DII.ChipMask <<   0));       // chip_mask[10:0]. 2048 - chip size
#endif

// Step 12. MemConfig
    WriteIO32( &pReg_DrexTZ->MEMCONFIG[0],
        (0x0    <<  20) |           // bank lsb, LSB of Bank Bit Position in Complex Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12, 5:13
        (0x0    <<  19) |           // rank inter en, Rank Interleaved Address Mapping
        (0x0    <<  18) |           // bit sel en, Enable Bit Selection for Randomized interleaved Address Mapping
        (0x0    <<  16) |           // bit sel, Bit Selection for Randomized Interleaved Address Mapping
        (0x2    <<  12) |           // [15:12] chip_map. Address Mapping Method (AXI to Memory). 0:Linear(Bank, Row, Column, Width), 1:Interleaved(Row, bank, column, width), other : reserved
#if (CFG_NSIH_EN == 0)
        (chip_col   <<   8) |       // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
        (chip_row   <<   4) |       // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
#else
        (pSBI->DII.ChipCol  <<   8) |   // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
        (pSBI->DII.ChipRow  <<   4) |   // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
#endif
        (0x3    <<   0));           // [ 3: 0] chip_bank. Number of  Bank Address Bit. others:Reserved, 2:4bank, 3:8banks


#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        WriteIO32( &pReg_DrexTZ->MEMCONFIG[1],
            (0x0    <<  20) |       // bank lsb, LSB of Bank Bit Position in Complex Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12, 5:13
            (0x0    <<  19) |       // rank inter en, Rank Interleaved Address Mapping
            (0x0    <<  18) |       // bit sel en, Enable Bit Selection for Randomized interleaved Address Mapping
            (0x0    <<  16) |       // bit sel, Bit Selection for Randomized Interleaved Address Mapping
            (0x2    <<  12) |       // [15:12] chip_map. Address Mapping Method (AXI to Memory). 0 : Linear(Bank, Row, Column, Width), 1 : Interleaved(Row, bank, column, width), other : reserved
            (chip_col   <<   8) |   // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
            (chip_row   <<   4) |   // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
            (0x3    <<   0));       // [ 3: 0] chip_bank. Number of  Row Address Bit. others:Reserved, 2:4bank, 3:8banks
#endif
#else
    if(pSBI->DII.ChipNum > 1) {
        WriteIO32( &pReg_DrexTZ->MEMCONFIG[1],
            (0x0    <<  20) |       // bank lsb, LSB of Bank Bit Position in Complex Interleaved Mapping 0:8, 1: 9, 2:10, 3:11, 4:12, 5:13
            (0x0    <<  19) |       // rank inter en, Rank Interleaved Address Mapping
            (0x0    <<  18) |       // bit sel en, Enable Bit Selection for Randomized interleaved Address Mapping
            (0x0    <<  16) |       // bit sel, Bit Selection for Randomized Interleaved Address Mapping
            (0x2    <<  12) |       // [15:12] chip_map. Address Mapping Method (AXI to Memory). 0 : Linear(Bank, Row, Column, Width), 1 : Interleaved(Row, bank, column, width), other : reserved
            (pSBI->DII.ChipCol  <<   8) |   // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
            (pSBI->DII.ChipRow  <<   4) |   // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
            (0x3    <<   0));       // [ 3: 0] chip_bank. Number of  Row Address Bit. others:Reserved, 2:4bank, 3:8banks
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
    WriteIO32( &pReg_Drex->PRECHCONFIG1,    0x00 );         //- precharge cycle
    WriteIO32( &pReg_Drex->PWRDNCONFIG,     0xFF );         //- low power counter


// Step 14.  AC Timing
#if (CFG_NSIH_EN == 0)
    WriteIO32( &pReg_Drex->TIMINGAREF,
        (tREFIPB    <<  16) |       //- rclk (MPCLK)
        (tREFI      <<   0));       //- refresh counter, 800MHz : 0x618

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGROW,
        (tRFC       <<  24) |
        (tRRD       <<  20) |
        (tRP        <<  16) |
        (tRCD       <<  12) |
        (tRC        <<   6) |
        (tRAS       <<   0)) ;

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGDATA,
        (tWTR       <<  28) |
        (tWR        <<  24) |
        (tRTP       <<  20) |
        (tPPD       <<  17) |
        (W2W_C2C    <<  14) |
        (R2R_C2C    <<  12) |
        (nWL        <<   8) |
        (tDQSCK     <<   4) |
        (nRL        <<   0));

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGPOWER,
        (tFAW       <<  26) |
        (tXSR       <<  16) |
        (tXP        <<   8) |
        (tCKE       <<   4) |
        (tMRD       <<   0));

#if (_DDR_CS_NUM > 1)
    WriteIO32( &pReg_Drex->ACTIMING1.TIMINGROW,
        (tRFC       <<  24) |
        (tRRD       <<  20) |
        (tRP        <<  16) |
        (tRCD       <<  12) |
        (tRC        <<   6) |
        (tRAS       <<   0)) ;

    WriteIO32( &pReg_Drex->ACTIMING1.TIMINGDATA,
        (tWTR       <<  28) |
        (tWR        <<  24) |
        (tRTP       <<  20) |
        (W2W_C2C    <<  14) |   // W2W_C2C
        (R2R_C2C    <<  12) |   // R2R_C2C
        (nWL        <<   8) |
        (tDQSCK     <<   4) |   // tDQSCK
        (nRL        <<   0));

    WriteIO32( &pReg_Drex->ACTIMING1.TIMINGPOWER,
        (tFAW       <<  26) |
        (tXSR       <<  16) |
        (tXP        <<   8) |
        (tCKE       <<   4) |
        (tMRD       <<   0));
#endif

//    WriteIO32( &pReg_Drex->TIMINGPZQ,   0x00004084 );     //- average periodic ZQ interval. Max:0x4084
    WriteIO32( &pReg_Drex->TIMINGPZQ,   tPZQ );           //- average periodic ZQ interval. Max:0x4084

    WriteIO32( &pReg_Drex->WRLVL_CONFIG[0],     (2      <<   4) );          // tWLO[7:4]
//    WriteIO32( &pReg_Drex->WRLVL_CONFIG[0],     (tWLO   <<   4) );          // tWLO[7:4]
#else

// Step 14.  AC Timing
    WriteIO32( &pReg_Drex->TIMINGAREF,              pSBI->DII.TIMINGAREF );                 //- refresh counter, 800MHz : 0x618

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGROW,     pSBI->DII.TIMINGROW ) ;
    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGDATA,    pSBI->DII.TIMINGDATA );
    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGPOWER,   pSBI->DII.TIMINGPOWER );

    if(pSBI->DII.ChipNum > 1)
    {
        WriteIO32( &pReg_Drex->ACTIMING1.TIMINGROW,     pSBI->DII.TIMINGROW ) ;
        WriteIO32( &pReg_Drex->ACTIMING1.TIMINGDATA,    pSBI->DII.TIMINGDATA );
        WriteIO32( &pReg_Drex->ACTIMING1.TIMINGPOWER,   pSBI->DII.TIMINGPOWER );
    }

//    WriteIO32( &pReg_Drex->TIMINGPZQ,   0x00004084 );               //- average periodic ZQ interval. Max:0x4084
    WriteIO32( &pReg_Drex->TIMINGPZQ,   pSBI->DII.TIMINGPZQ );      //- average periodic ZQ interval. Max:0x4084

    WriteIO32( &pReg_Drex->WRLVL_CONFIG[0],     (2      <<   4) );          // tWLO[7:4]
//    WriteIO32( &pReg_Drex->WRLVL_CONFIG[0],     (tWLO   <<   4) );          // tWLO[7:4]
#endif

#if 1   // fix - active
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
        (0x6    <<   8) |           // [11: 8] mem_type     : Type of Memory                            :: 4'h6  - ddr3
        (0x0    <<   6) |           // [ 7: 6] add_lat_pall : Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
        (0x0    <<   5) |           // [    5] dsref_en     : Dynamic Self Refresh                      :: 1'b0  - Disable
//        (0x0    <<   4) |           // [    4] Reserved     : SBZ
        (0x0    <<   2) |           // [ 3: 2] dpwrdn_type  : Type of Dynamic Power Down                :: 2'b00 - Active/precharge power down
        (0x0    <<   1) |           // [    1] dpwrdn_en    : Dynamic Power Down                        :: 1'b0  - Disable
        (0x0    <<   0));           // [    0] clk_stop_en  : Dynamic Clock Control                     :: 1'b0  - Always running
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
    WriteIO32( &pReg_DDRPHY->OFFSETR_CON[0],    0x08080808 );

    // set WRITE skew
    WriteIO32( &pReg_DDRPHY->OFFSETW_CON[0],    0x08080808 );
#endif

    // set ctrl_shiftc value.
//    WriteIO32( &pReg_DDRPHY->SHIFTC_CON,        0x00 );

    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization

    if (isResume == 0)
    {
        // Step 18, 19 :  Send NOP command.
        SendDirectCommand(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif


        // Step 20 :  Send MR2 command.
        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR2.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR2.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR2.Reg);
#endif


        // Step 21 :  Send MR3 command.
        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR3.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR3.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR3.Reg);
#endif


        // Step 22 :  Send MR1 command.
        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif


        // Step 23 :  Send MR0 command.
        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR0, MR0.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR0, MR0.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR0, MR0.Reg);
#endif


        // Step 25 : Send ZQ Init command
        SendDirectCommand(SDRAM_CMD_ZQINIT, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_ZQINIT, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_ZQINIT, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
        DMC_Delay(100);
    }   // if (isResume)

#if 0   // fix - inactive
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
        (0x6    <<   8) |           // [11: 8] mem_type     : Type of Memory                            :: 4'h6  - ddr3
        (0x0    <<   6) |           // [ 7: 6] add_lat_pall : Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
        (0x0    <<   5) |           // [    5] dsref_en     : Dynamic Self Refresh                      :: 1'b0  - Disable
//        (0x0    <<   4) |           // [    4] Reserved     : SBZ
        (0x0    <<   2) |           // [ 3: 2] dpwrdn_type  : Type of Dynamic Power Down                :: 2'b00 - Active/precharge power down
        (0x0    <<   1) |           // [    1] dpwrdn_en    : Dynamic Power Down                        :: 1'b0  - Disable
        (0x0    <<   0));           // [    0] clk_stop_en  : Dynamic Clock Control                     :: 1'b0  - Always running
#endif


#if 1

//    printf("\r\n########## READ/GATE Level ##########\r\n");


    //======================================================================
    //======================== Training Preparation ========================
    //======================================================================

    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  28) );          // upd_mode=0, PHY side update mode.

    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<   6) );          // ctrl_atgate=1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  14) );          // p0_cmd_en=1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<   6) );          // InitDeskewEn=1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  13) );          // byte_rdlvl_en=1

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1]) & ~(0xF <<  16);            // rdlvl_pass_adj=4
    temp |= (0x4 <<  16);
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],        temp);

#if defined(MEM_TYPE_DDR3)
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[3],     0x105E );                   // cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[4],     0x107F );                   // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
#endif
#if defined(MEM_TYPE_LPDDR23)
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[3],     0x000E );                   // cmd_active= DDR3:0x105F, LPDDDR2 or LPDDDR3:0x000E
    WriteIO32( &pReg_DDRPHY->LP_DDR_CON[4],     0x000F );                   // cmd_default= DDR3:0x107F, LPDDDR2 or LPDDDR3:0x000F
#endif

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[2]) & ~(0x7F << 16);            // rdlvl_incr_adj=1
    temp |= (0x1 <<  16);
    WriteIO32( &pReg_DDRPHY->PHY_CON[2],        temp);

#if 1
#if 0
    ClearIO32( &pReg_DDRPHY->MDLL_CON[0],       (0x1    <<   5) );          // ctrl_dll_on[5]=0

    do {
        temp = ReadIO32( &pReg_DDRPHY->MDLL_CON[1] );                       // read lock value
    } while( (temp & 0x7) < 0x5 );
#else

    do {
        SetIO32  ( &pReg_DDRPHY->MDLL_CON[0],       (0x1    <<   5) );      // ctrl_dll_on[5]=1

        do {
            temp = ReadIO32( &pReg_DDRPHY->MDLL_CON[1] );                   // read lock value
        } while( (temp & 0x7) != 0x7 );

        ClearIO32( &pReg_DDRPHY->MDLL_CON[0],       (0x1    <<   5) );      // ctrl_dll_on[5]=0

        temp = ReadIO32( &pReg_DDRPHY->MDLL_CON[1] );                       // read lock value
    } while( (temp & 0x7) != 0x7 );
#endif

    g_Lock_Val = (temp >> 8) & 0x1FF;
#else

    showLockValue();
#endif

    temp  = ReadIO32( &pReg_DDRPHY->MDLL_CON[0] ) & ~(0x1FF << 7);
    temp |= (g_Lock_Val << 7);
    WriteIO32( &pReg_DDRPHY->MDLL_CON[0],       temp);                      // ctrl_force[16:8]

#if (SKIP_LEVELING_TRAINING == 0)
    if (isResume == 0)
    {
        if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
            DDR_HW_Write_Leveling();

#if 0
        if (pSBI->LvlTr_Mode & LVLTR_CA_CAL)
            DDR_CA_Calibration();
#endif

        if (pSBI->LvlTr_Mode & LVLTR_GT_LVL)
        {
            if (DDR_Gate_Leveling() == CFALSE)
                return CFALSE;
        }

        if (pSBI->LvlTr_Mode & LVLTR_RD_CAL)
            DDR_Read_DQ_Calibration();

        if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
            DDR_Write_Latency_Calibration();

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
    }
    else
    {
        U32 lock_div4 = (g_Lock_Val >> 2);

        //----------------------------------
        // Restore leveling & training values.
        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     1);                 // open alive power gate
        DMC_Delay(100);
        g_GT_cycle  = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE5);        // read - ctrl_shiftc
        g_GT_code   = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE6);        // read - ctrl_offsetc
        g_RD_vwmc   = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE7);        // read - ctrl_offsetr
        g_WR_vwmc   = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE8);        // read - ctrl_offsetw
//        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     0);                 // close alive power gate

        if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
            WriteIO32( &pReg_DDRPHY->WR_LVL_CON[0],     g_WR_lvl );

#if 0
        if (pSBI->LvlTr_Mode & LVLTR_CA_CAL)
            DDR_CA_Calibration();
#endif

        if (pSBI->LvlTr_Mode & LVLTR_GT_LVL)
        {
            U32 i, min;
            U32 GT_cycle = 0;
            U32 GT_code  = 0;

            SetIO32  ( &pReg_DDRPHY->PHY_CON[2],    (0x1    <<  24) );      // gate_cal_mode[24] = 1
            SetIO32  ( &pReg_DDRPHY->PHY_CON[0],    (0x5    <<   6) );      // ctrl_shgate[8]=1, ctrl_atgate[6]=1
            ClearIO32( &pReg_DDRPHY->PHY_CON[1],    (0xF    <<  20) );      // ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9

            min = g_GT_cycle & 0x7;
            for (i = 1; i < 4; i++)
            {
                temp = (g_GT_cycle >> (3 * i)) & 0x7;
                if ( temp < min )
                    min = temp;
            }

            if ( min )
            {
                GT_cycle = (g_GT_cycle & 0x7) - min;
                for (i = 1; i < 4; i++)
                {
                    temp = ((g_GT_cycle >> (3 * i)) & 0x7) - min;
                    GT_cycle |= (temp << (3 * i));
                }

                min = ((ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) >> 28) & 0xF) + min;    // ctrl_gateadj[31:28]

                temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) & 0x0FFFFFFF;
                temp |= (min << 28);
                WriteIO32( &pReg_DDRPHY->PHY_CON[1],  temp );
            }

            MEMMSG("min = %d\r\n", min);
            MEMMSG("GT_cycle  = 0x%08X\r\n", GT_cycle );

            GT_code = getVWMC_Offset(g_GT_code, lock_div4);
            WriteIO32( &pReg_DDRPHY->OFFSETC_CON[0],    GT_code );
            WriteIO32( &pReg_DDRPHY->SHIFTC_CON,        0x00 );

            ClearIO32( &pReg_DDRPHY->PHY_CON[2],    (0x1    <<  24) );      // gate_cal_mode[24] = 0
            WriteIO32( &pReg_DDRPHY->LP_CON,        0x0 );                  // ctrl_pulld_dqs[8:0] = 0
            ClearIO32( &pReg_DDRPHY->RODT_CON,      (0x1    <<  16) );      // ctrl_read_dis[16] = 0
        }

        if (pSBI->LvlTr_Mode & LVLTR_RD_CAL) {
            g_RD_vwmc = getVWMC_Offset(g_RD_vwmc, lock_div4);
            WriteIO32( &pReg_DDRPHY->OFFSETR_CON[0],    g_RD_vwmc );
        }

        if (pSBI->LvlTr_Mode & LVLTR_WR_LVL)
            DDR_Write_Latency_Calibration();

        if (pSBI->LvlTr_Mode & LVLTR_WR_CAL) {
            g_WR_vwmc = getVWMC_Offset(g_WR_vwmc, lock_div4);
            WriteIO32( &pReg_DDRPHY->OFFSETW_CON[0],    g_WR_vwmc );
        }
    }

    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  24) );          // ctrl_resync[24]=0x0 (LOW)
#endif  // #if (SKIP_LEVELING_TRAINING == 0)

    ClearIO32( &pReg_DDRPHY->PHY_CON[0],        (0x3    <<  13) );          // p0_cmd_en[14]=0, byte_rdlvl_en[13]=0

    SetIO32  ( &pReg_DDRPHY->MDLL_CON[0],       (0x1    <<   5) );          // ctrl_dll_on[5]=1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    <<  12) );          // DLLDeskewEn[12]=1

    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,       (0x1    <<  28) );          // upd_mode=1

    SetIO32  ( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL,          (0x1    <<   3) );          // Force DLL Resyncronization
#endif  // gate leveling


    /* Send PALL command */
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

    WriteIO32( &pReg_Drex->PHYCONTROL,
        (0x1    <<  31) |           // [   31] mem_term_en. Termination Enable for memory. Disable : 0, Enable : 1
        (0x1    <<  30) |           // [   30] phy_term_en. Termination Enable for PHY. Disable : 0, Enable : 1
        (0x1    <<  29) |           // [   29] ctrl_shgate. Duration of DQS Gating Signal. gate signal length <= 200MHz : 0, >200MHz : 1
        (0x0    <<  24) |           // [28:24] ctrl_pd. Input Gate for Power Down.
        (0x0    <<   8) |           // [    8] Termination Type for Memory Write ODT (0:single, 1:both chip ODT)
        (0x0    <<   7) |           // [    7] Resync Enable During PAUSE Handshaking
        (0x0    <<   4) |           // [ 6: 4] dqs_delay. Delay cycles for DQS cleaning. refer to DREX datasheet
        (0x0    <<   3) |           // [    3] fp_resync. Force DLL Resyncronization : 1. Test : 0x0
        (0x0    <<   2) |           // [    2] Drive Memory DQ Bus Signals
        (0x0    <<   1) |           // [    1] sl_dll_dyn_con. Turn On PHY slave DLL dynamically. Disable : 0, Enable : 1
        (0x1    <<   0));           // [    0] mem_term_chips. Memory Termination between chips(2CS). Disable : 0, Enable : 1

    temp = (U32)(
        (0x0    <<  28) |           // [   28] dfi_init_start
        (0xFFF  <<  16) |           // [27:16] timeout_level0
        (0x1    <<  12) |           // [14:12] rd_fetch
        (0x1    <<   8) |           // [    8] empty
        (0x0    <<   6) |           // [ 7: 6] io_pd_con
        (0x1    <<   5) |           // [    5] aref_en - Auto Refresh Counter. Disable:0, Enable:1
        (0x0    <<   3) |           // [    3] update_mode - Update Interface in DFI.
        (0x0    <<   1) |           // [ 2: 1] clk_ratio
        (0x0    <<   0));           // [    0] ca_swap

    if (isResume)
        temp &= ~(0x1    <<   5);

    WriteIO32( &pReg_Drex->CONCONTROL,  temp );

    WriteIO32( &pReg_Drex->CGCONTROL,
        (0x0    <<   4) |           // [    4] phy_cg_en
        (0x0    <<   3) |           // [    3] memif_cg_en
        (0x0    <<   2) |           // [    2] scg_sg_en
        (0x0    <<   1) |           // [    1] busif_wr_cg_en
        (0x0    <<   0));           // [    0] busif_rd_cg_en

#if DDR_RW_CAL
    DDR3_RW_Delay_Calibration();
#endif

#else    // ddr simulation
{
    //1.3.4 DDR3 with PHY V6
    //1. Apply power. RESET# pin of memory needs to be maintained for minimum 200us with stable power.
    //   CKE is pulled 'Low' anytime before RESET# being de-asserted (min. time 10ns)

    //2. Set the PHY for DDR3 operation mode, RL/WL/BL register and proceed ZQ calibration. Refer to "INITIALIZATION" in PHY manual.
    // - After power-up and system PLL locking time, system reset(rst_n) is released.
    // - Select Memory Type (=PHY_CON0[12:11]).
    //   + ctrl_ddr_mode=2'b11 (LPDDR3)
    //   + ctrl_ddr_mode=2'b10 (LDDR2)
    //   + ctrl_ddr_mode=2'b00 (DDR2)
    //   + ctrl_ddr_mode=2'b01 (DDR3)
    WriteIO32( pReg_DDRPHY->PHY_CON[0],
        (0x17   <<  24) |       // [28:24] T_WrWrCmd
        (0x1    <<  22) |       // [23:22] ctrl_upd_mode
        (0x0    <<  20) |       // [21:20] ctrl_upd_range
        (0x6    <<  17) |       // [19:17] T_WrRdCmd
        (0x0    <<  16) |       // [   16] wrlvl_mode
        (0x0    <<  14) |       // [   14] p0_cmd_en
        (0x0    <<  13) |       // [   13] byte_rdlvl_en
        (0x1    <<  11) |       // [12:11] ctrl_ddr_mode    0:DDR2&LPDDR1, 1:DDR3, 2:LPDDR2, 3:LPDDR3
        (0x1    <<  10) |       // [   10] ctrl_wr_dis
        (0x1    <<   9) |       // [    9] ctrl_dfdqs
        (0x1    <<   8) |       // [    8] ctrl_shgate
        (0x0    <<   7) |       // [    7] Reserved (ctrl_ckdis removed?)
        (0x1    <<   6) |       // [    6] ctrl_atgate
        (0x0    <<   5) |       // [    5] Reserved (ctrl_read_dis removed?)
        (0x0    <<   4) |       // [    4] ctrl_cmosrcv
        (0x0    <<   3) |       // [    3] Reserved (ctrl_read_width removed?)
        (0x0    <<   0));       // [ 2: 0] ctrl_fnc_fb

    // NOTE: If ctrl_ddr_mode[1]=1'b1, cmd_active=14'h000E(=LP_DDR_CON3[13:0]), cmd_default=14'h000F(=LP_DDR_CON4[13:0]) upd_mode=1'b1(=OFFSETD_CON0[28])

    // - Set Read Latency(RL), Burst Length(BL) and Write Latency(WL)
    //   + Set RL in PHY_CON4[4:0].
    //   + Set BL in PHY_CON4[12:8].
    //   + Set WL in PHY_CON4[20:16].
    WriteIO32( pReg_DDRPHY->PHY_CON[4],
        (0x8    <<  16) |       // [20:16] WL = CWL + AL
        (0x8    <<   8) |       // [12: 8] BL
        (0xB    <<   0));       // [ 4: 0] RL

    // - ZQ Calibration(Please refer to "8.5 ZQ I/O CONTROL PROCEDURE" for more details)
    //   + Enable and Disable "zq_clk_div_en" in ZQ_CON0[18]
    //   + Enable "zq_manual_str" in ZQ_CON0[1]
    //   + Wait until "zq_cal_done"(ZQ_CON1[0]) is enabled.
    //   + Disable "zq_manual_str" in ZQ_CON0[1]
    WriteIO32( &pReg_DDRPHY->ZQ_CON,
        (0x1    <<  27) |       // [   27] zq_clk_en
        (0x7    <<  24) |       // [26:24] zq_mode_dds
        (0x0    <<  21) |       // [23:21] zq_mode_term
        (0x0    <<  20) |       // [   20] zq_rgddr
        (0x0    <<  19) |       // [   19] zq_mode_noterm
        (0x1    <<  18) |       // [   18] zq_clk_div_en    - Clock dividing enable
        (0x0    <<  15) |       // [17:15] zq_force_impn
        (0x7    <<  12) |       // [14:12] zq_force_impp
        (0x30   <<   4) |       // [11: 4] zq_udt_dly
        (0x1    <<   2) |       // [ 3: 2] zq_manual_mode
        (0x1    <<   1) |       // [    1] zq_manual_str    - Manual calibration start
        (0x0    <<   0));       // [    0] zq_auto_en

    while( (ReadIO32(&pReg_DDRPHY->ZQ_STATUS) & 0x1) != 0x1 );       // [    0] zq_done    - ZQ Callbration is finished

    WriteIO32( &pReg_DDRPHY->ZQ_CON,
        (0x1    <<  27) |       // [   27] zq_clk_en
        (0x7    <<  24) |       // [26:24] zq_mode_dds
        (0x0    <<  21) |       // [23:21] zq_mode_term
        (0x0    <<  20) |       // [   20] zq_rgddr3
        (0x0    <<  19) |       // [   19] zq_mode_noterm
        (0x0    <<  18) |       // [   18] zq_clk_div_en    - Clock dividing enable
        (0x0    <<  15) |       // [17:15] zq_force_impn
        (0x7    <<  12) |       // [14:12] zq_force_impp
        (0x30   <<   4) |       // [11: 4] zq_udt_dly
        (0x1    <<   2) |       // [ 3: 2] zq_manual_mode
        (0x0    <<   1) |       // [    1] zq_manual_str    - Manual calibration start
        (0x0    <<   0));       // [    0] zq_auto_en

    //3. Assert the ConControl.dfi_init_start field to high but leave as default value for other fields.(aref_en and io_pd_con should be off.)
    //   Clock gating in CGControl should be disabled in initialization and training se-quence.
    SetIO32( &pReg_Drex->CONCONTROL,    (0x1    << 28));    // [   28] dfi_init_start    - DFI PHY initialization strat

    //4. Wait for the PhyStatus0.dfi_init_complete field to change to '1'.
    while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & 0x8 ) != 0x8 );    // [    3] dfi_init_complete    - DFI PHY initialization complete

    //5. Deassert the ConControl.dfi_init_start field to low.
    ClearIO32( &pReg_Drex->CONCONTROL,    (0x1    << 28));    // [   28] dfi_init_start    - DFI PHY initialization strat

    //6. Set the PHY for dqs pulldown mode. (Refer to PHY manual)
    // + Enable DQS pull down mode
    //   - Set "ctrl_pulld_dqs=9'h1FF" (=LP_CON0[8:0]) in case of using 72bit PHY.
    //   - Please be careful that DQS pull down can be disabled only after Gate Leveling is done.
    WriteIO32( &pReg_DDRPHY->LP_CON,
        (0x0    <<  16) |       // [24:16] ctrl_pulld_dq
        (0xF    <<   0));       // [ 8: 0] ctrl_pulld_dqs    - 0: pull-up for PDQS/NDQS signal 1: pull-down for PDQS/NDQS signal

    // + Memory Controller should assert "dfi_ctrlupd_req" after "dfi_init_complete" is set.
    //   - Please keep "Ctrl-Initiated Update" mode until finishing Leveling and Training.

    //7. Set the PhyControl0.fp_resync bit-field to '1' to update DLL information.
    SetIO32  ( &pReg_Drex->PHYCONTROL,    (0x1    <<  3));    // [    3] fp_resync    - Froce DLL Resyncronization

    //8. Set the PhyControl0.fp_resync bit-field to '0'.
    ClearIO32( &pReg_Drex->PHYCONTROL,    (0x1    <<  3));    // [    3] fp_resync    - Froce DLL Resyncronization

    //9. Set the MemBaseConfig0 register and MemBaseConfig1 register if needed.
    WriteIO32( &pReg_DrexTZ->MEMBASECONFIG[0],
        (0x40   <<  16) |       // [26:16] chip_base    - AXI Base Address        0x80000000
        (0x7C0  <<   0));       // [10: 0] chip_mask    - AXI Base Address Mask    1GB
//    WriteIO32( &pReg_DrexTZ->MEMBASECONFIG[0],
//        (0x40   <<  16) |       // [26:16] chip_base    - AXI Base Address        0x40000000
//        (0x780  <<   0));       // [10: 0] chip_mask    - AXI Base Address Mask    2GB

    WriteIO32( &pReg_DrexTZ->MEMCONFIG[0],
        (0x0    <<  20) |       // [22:20] bank_lsb
        (0x0    <<  19) |       // [   19] rank_inter_en
        (0x0    <<  18) |       // [   18] bit_sel_en
        (0x2    <<  16) |       // [17:16] bit_sel
        (0x2    <<  12) |       // [15:12] chip_map     - 2 : Split Column Interleaved
        (0x3    <<   8) |       // [11: 8] chip_col     - 3 : 10 bits
        (0x3    <<   4) |       // [ 7: 4] chip_row     - 3 : 15 bits
        (0x3    <<   0));       // [ 3: 0] chip_bank    - 3 : 8  Banks


    //10. Set the PrechConfig and PwrdnConfig registers.
//    WriteIO32( &pReg_Drex->PRECHCONFIG0,    0x00000000 );
//    WriteIO32( &pReg_Drex->PRECHCONFIG1,    0xFFFFFFFF );
//    WriteIO32( &pReg_Drex->PWRDNCONFIG ,    0xFFFF00FF );

    //11. Set the TimingAref, TimingRow, TimingData and TimingPower registers according to memory AC parameters.
    WriteIO32( &pReg_Drex->TIMINGAREF,
        (0x98   <<  16) |       // [31:16] t_refpb  - 0.4875 us (rclk)
        (0x618  <<   0));       // [15: 0] t_refi   - 7.8    us (rclk)

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGROW,
        (0x68   <<  24) |       // [31:24] tRFC     - 260   ns  (cclk)
        (0x4    <<  20) |       // [23:20] tRRD     - 7.5   ns  (cclk)
        (0x6    <<  16) |       // [19:16] tRP      - 13.75 ns  (cclk)
        (0x6    <<  12) |       // [15:12] tRCD     - 13.75 ns  (cclk)
        (0x14   <<   6) |       // [11: 6] tRC      - 48.75 ns  (cclk)
        (0xF    <<   0));       // [ 5: 0] tRAS     - 35    ns  (cclk)

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGDATA,
        (0x4    <<  28) |       // [31:28] tWTR     - 7.5 ns
        (0x6    <<  24) |       // [27:24] tWR      - 15  ns
        (0x4    <<  20) |       // [23:20] tRTP     - 7.5 ns
        (0x0    <<  17) |       // [   17] tPPD     - for LPDDR3
        (0x0    <<  14) |       // [   14] t_w2w_c2c -
        (0x0    <<  12) |       // [   12] t_r2r_c2c -
        (0x8    <<   8) |       // [11: 8] WL       - AL + CWL
        (0x0    <<   4) |       // [ 7: 4] dqsck    - DDR3 : 0
        (0xB    <<   0));       // [ 3: 0] RL       - AL + CL

    WriteIO32( &pReg_Drex->ACTIMING0.TIMINGPOWER,
        (0x10   <<  26) |       // [31:26] tFAW     - 40 ns
        (0x20   <<  16) |       // [25:16] tXSR     @sei ?? 512 tCK
        (0x0A   <<   8) |       // [15: 8] tXP      @sei ?? 3 tCK
        (0x2    <<   4) |       // [ 7: 4] tCKE     -  5 ns
        (0x4    <<   0));       // [ 3: 0] tMRD     -  4 tCK

    //12. If QoS scheme is required, set the QosControl0~15 and QosConfig0~15 registers.
    //--------------------------------------
    // @modified by choiyk 2014/02/12
    // for display stresstest. QOS15 is timingout 0
    // and set display's qos to 15 (0xF)
//    WriteIO32( DREX_QOSCONTROL15, 0 );
    //--------------------------------------


    //13. Confirm that after RESET# is de-asserted, 500 us have passed before CKE becomes active.
    //14. Confirm that clocks(CK, CK#) need to be started and stabilized for at least 10 ns or 5 tCK (which is larger) before CKE goes active.

    //15. Issue a NOP command using the DirectCmd register to assert and to hold CKE to a logic high level.
    SendDirectCommand(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL, CNULL);

    //16. Wait for tXPR(max(5nCK,tRFC(min)+10ns)) or set tXP to tXPR value before step 16.
    //    If the system set tXP to tXPR, then the system must set tXP to proper value before normal memory operation.
    DMC_Delay(100);

    //17. Issue an EMRS2 command using the DirectCmd register to program the operating parameters. Dynamic ODT should be disabled. A10 and A9 should be low.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2,
        (0x2    <<   9) |       // Dynamic ODT(Rtt)[10:9]. 0:Disable, 1:RZQ/4(60ohm), 2:RZQ/2(120ohm), 3:Reserved
        (0x0    <<   8) |       // Reserved[8] SBZ
        (0x0    <<   7) |       // Self Refresh Temperature[7]. 0:Normal(0~85), 1:Extended(0~95)
        (0x0    <<   6) |       // Auto Self Refresh[6]. 0:Disabled, 1:Enabled
        (0x3    <<   3) |       // CAS Write Latency(CWL).
                                // 0:5ck(tCK>2.5ns), 1:6ck(2.5ns>tCK>1.875ns), 2:7ck(1.875ns>tCK>2.5ns), 3:8ck(1.5ns>tCK>1.25ns,
                                // 4:9ck(1.25ns>tCK>1.07ns), 5:10ck(1.07ns>tCK>0.935ns), 6:11ck(0.935ns>tCK>0.833ns), 7:12ck(0.833ns>tCK>0.75ns)
        (0x0    <<   0));       // Partial Array Self-Refresh(Option)

    //18. Issue an EMRS3 command using the DirectCmd register to program the operating parameters.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3,
        (0x0    <<   3) |       // Reserved SBZ[14:3].
        (0x0    <<   2) |       // MPR enable. 0:Normal DRAM operation, 1:Dataflow from MPR
        (0x0    <<   0));       // MPR Read function. 0:Predefined pattern. 1~3: Reserved

    //19. Issue an EMRS command using the DirectCmd register to enable the memory DLL.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1,
        (0x0    <<  12) |       // Qoff[12]. 0:Enable, 1:Disable
        (0x0    <<  11) |       // TDQS[11]. 0:Disable(x4, x16), 1:Enable(x8 only)
        (0x0    <<  10) |       // Reserved SBZ[10]
        (0x0    <<   9) |       // Rtt(ODT)[9,6,2]. 000:Disable, 001:RZQ/4, 010:RZQ/2
        (0x1    <<   6) |       // Rtt(ODT)[9,6,2]. 011:RZQ/6, 100:RZQ/12, 101:RZQ/8
        (0x0    <<   2) |       // Rtt(ODT)[9,6,2]. 110, 111:Reserved
        (0x0    <<   8) |       // Reserved SBZ[8]
        (0x0    <<   7) |       // Write Leveling[7]. 0:Disable, 1:Enable
        (0x0    <<   5) |       // Output Drive Strength[5,1]. 00:RZQ/6, 01:RZQ/7
        (0x1    <<   1) |       // Output Drive Strength[5,1]. 10, 11 : Reserved
        (0x0    <<   3) |       // Additive Latency(AL)[4:3]. 0:AL=0, 1:AL=CL-1, 2:AL=CL-2, 3:Reserved
        (0x0    <<   0));       // DLL[0]. 0:Enable, 1:Disable

    //20. Issue a MRS command using the DirectCmd register to reset the memory DLL.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR0,
        (0x0    <<  12) |       // Precharge PD. 0:DLL off(slow exit), 1:DLL on(fast exit)
        (0x6    <<   9) |       // Write Recovery(WR)[11:9]. 0:Reserved, 1:5, 2:6, 3:7, 4:8, 5:10, 6:12, 7:14 (### 12clk test)
        (0x1    <<   8) |       // DLL Reset[8]. 0:No, 1:Reset
        (0x0    <<   7) |       // Reserved[7] SBZ
        (0x1    <<   6) |       // CAS Latency[6:4],[2]. 0010:5, 0100:6, 0110:7
        (0x1    <<   5) |       // CAS Latency[6:4],[2]. 1000:8, 1010:9, 1100:10
        (0x1    <<   4) |       // CAS Latency[6:4],[2]. 1110:11, 0001:12, 0011:13
        (0x0    <<   2) |       // CAS Latency[6:4],[2]. others:Reserved
        (0x1    <<   3) |       // Read Burst Type[3]. 0:Sequential, 1:Interleave
        (0x0    <<   0));       // Burst Length[1:0]. 0:Fixed 8, 1:4or8, 2:Fixed4, 3:Reserved
                                                                                     // Precharge P/D_DLL Off, WR : 5, DLL Reset Off, CL : 11, Read Burst Type : Interleave, Burst Length : 8

    //21. Issues a MRS command using the DirectCmd register to program the operating parameters without resetting the memory DLL.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR0,
        (0x0    <<  12) |       // Precharge PD. 0:DLL off(slow exit), 1:DLL on(fast exit)
        (0x6    <<   9) |       // Write Recovery(WR)[11:9]. 0:Reserved, 1:5, 2:6, 3:7, 4:8, 5:10, 6:12, 7:14 (### 12clk test)
        (0x0    <<   8) |       // DLL Reset[8]. 0:No, 1:Reset
        (0x0    <<   7) |       // Reserved[7] SBZ
        (0x1    <<   6) |       // CAS Latency[6:4],[2]. 0010:5, 0100:6, 0110:7
        (0x1    <<   5) |       // CAS Latency[6:4],[2]. 1000:8, 1010:9, 1100:10
        (0x1    <<   4) |       // CAS Latency[6:4],[2]. 1110:11, 0001:12, 0011:13
        (0x0    <<   2) |       // CAS Latency[6:4],[2]. others:Reserved
        (0x1    <<   3) |       // Read Burst Type[3]. 0:Sequential, 1:Interleave
        (0x0    <<   0));       // Burst Length[1:0]. 0:Fixed 8, 1:4or8, 2:Fixed4, 3:Reserved
                                                                // Precharge P/D_DLL Off, WR : 5, DLL Reset Off, CL : 11, Read Burst Type : Interleave, Burst Length : 8

    //22. Issues a ZQINIT commands using the DirectCmd register.
    SendDirectCommand(SDRAM_CMD_ZQINIT, 0, (SDRAM_MODE_REG)CNULL, CNULL);

    //23. If there are more external memory chips, perform steps 17 ~ 24 procedures for other memory device.
    //24. If any leveling/training is needed, enable ctrl_atgate, p0_cmd_en, InitDeskewEn and byte_rdlvl_en. Disable ctrl_dll_on and set ctrl_force value. (Refer to PHY manual)
    //25. If write leveling is not needed, skip this procedure. If write leveling is needed, set DDR3 into write leveling mode using MRS direct command, set ODT pin high and tWLO using WRLVL_CONFIG0 regis-ter(offset=0x120) and set the related PHY SFR fields through PHY APB I/F(Refer to PHY manual). To gener-ate 1 cycle pulse of dfi_wrdata_en_p0, write 0x1 to WRLVL_CONFIG1 register(Offset addr=0x124). To read the value of memory data, use CTRL_IO_RDATA(offset = 0x150). If write leveling is finished, disable write leveling mode in PHY register and set ODT pin low and disable write leveling mode of DDR3.
    //26. If gate leveling is not needed, skip 27 ~ 28. If gate leveling is needed, set DDR3 into MPR mode using MRS direct command and set the related PHY SFR fields through PHY APB I/F. Do the gate leveling. (Refer to PHY manual)
    //27. If gate leveling is finished, set DDR3 into normal operation mode using MRS command and disable DQS pull-down mode.(Refer to PHY manual)
    //28. If read leveling is not needed skip 29 ~ 30. If read leveling is needed, set DDR3 into MPR mode using MRS direct command and set proper value to PHY control register. Do the read leveling..(Refer to PHY manual)
    //29. If read leveling is finished, set DDR3 into normal operation mode using MRS direct command.
    //30. If write training is not needed, skip 31. If write training is nedded, set the related PHY SFR fields through PHY APB I/F..(Refer to PHY manual). To issue ACT command, enable and disable WrtraCon-fig.write_training_en . Refer to this register definition for row and bank address. Do write training. (Refer to PHY manual)
    //31. After all leveling/training are completed, enable ctrl_dll_on. (Refer to PHY manual)

    //32. Set the PhyControl0.fp_resync bit-field to '1' to update DLL information.
    SetIO32  ( &pReg_Drex->PHYCONTROL,    (0x1    <<  3));        // [    3] fp_resync - Froce DLL Resyncronization

    //33. Set the PhyControl0.fp_resync bit-field to '0'.
    ClearIO32( &pReg_Drex->PHYCONTROL,    (0x1    <<  3));        // [    3] fp_resync - Froce DLL Resyncronization

    //34. Disable PHY gating control through PHY APB I/F if necessary(ctrl_atgate, refer to PHY manual).
    WriteIO32( &pReg_DDRPHY->PHY_CON[0],
        (0x17   <<  24) |       // [28:24] T_WrWrCmd
        (0x1    <<  22) |       // [23:22] ctrl_upd_mode
        (0x0    <<  20) |       // [21:20] ctrl_upd_range
        (0x6    <<  17) |       // [19:17] T_WrRdCmd
        (0x0    <<  16) |       // [   16] wrlvl_mode
        (0x0    <<  14) |       // [   14] p0_cmd_en
        (0x0    <<  13) |       // [   13] byte_rdlvl_en
        (0x1    <<  11) |       // [12:11] ctrl_ddr_mode
        (0x1    <<  10) |       // [   10] ctrl_wr_dis
        (0x1    <<   9) |       // [    9] ctrl_dfdqs
//        (0x1    <<   8) |       // [    8] ctrl_shgate
        (0x0    <<   8) |       // [    8] ctrl_shgate
        (0x0    <<   7) |       // [    7] Reserved (ctrl_ckdis removed?)
        (0x1    <<   6) |       // [    6] ctrl_atgate 0 : Controller generate ctrl_gate_p*, ctrl_read_p*, 1: PHY generate ctrl_gate_p* ctrl_read_p*
        (0x0    <<   5) |       // [    5] Reserved (ctrl_read_dis removed?)
        (0x0    <<   4) |       // [    4] ctrl_cmosrcv
        (0x0    <<   3) |       // [    3] Reserved (ctrl_read_width removed?)
        (0x0    <<   0));       // [ 2: 0] ctrl_fnc_fb


    //35. Issue PALL to all chips using direct command. This is an important step if write training has been done.
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);

    //36. Set the MemControl and PhyControl0 register.
    WriteIO32( &pReg_Drex->MEMCONTROL,
        (0x0    <<  29) |       // [   29] pause_ref_en
        (0x0    <<  28) |       // [   28] sp_en
        (0x0    <<  27) |       // [   27] pb_ref_en
        (0x0    <<  24) |       // [   24] pzq_en
        (0x3    <<  20) |       // [22:20] bl    - Memory burst length  1: 2, 2: 4, 3: 8;
        (0x0    <<  16) |       // [19:16] num_chip
        (0x2    <<  12) |       // [15:12] mem_width    - Width of Memory data bus 2: 32bit, 4:128bit
        (0x6    <<   8) |       // [11: 8] mem_type    - Tytpe of Memory 5:LPDDR2, 6:DDR3, 7:LPDDR3
        (0x0    <<   6) |       // [ 7: 6] add_lat_pall
        (0x0    <<   5) |       // [    5] dsref_en
        (0x0    <<   2) |       // [ 3: 2] dpwrdn_type
        (0x0    <<   1) |       // [    1] dpwrdn_en
        (0x0    <<   0));       // [    0] clk_stop_en

    WriteIO32( &pReg_Drex->PHYCONTROL,
        (0x1    <<  31) |       // [   31] mem_term_en - termination Enable Memory Wirte ODT 0: Disable, 1: Enable // ??
        (0x1    <<  30) |       // [   30] phy_term_en - Termination Eanble for PHY // ??
   //     (0x1    <<  29) |       // [   29] ctrl_shgate - Duration of DQS Gating Signal
        (0x0    <<  29) |       // [   29] ctrl_shgate - Duration of DQS Gating Signal
        (0x0    <<  24) |       // [28:24] ctrl_pd
        (0x0    <<   8) |       // [    8] mem_term_type
        (0x0    <<   7) |       // [    7] pause_resync_en
        (0x0    <<   4) |       // [ 6: 4] dqs_delay
        (0x0    <<   3) |       // [    3] fp_resync
        (0x0    <<   2) |       // [    2] drv_bus_en
        (0x0    <<   1) |       // [    1] sl_dll_dyn_con
        (0x0    <<   0));       // [    0] mem_term_chips

       //37. Set the ConControl register. aref_en should be turn on.
    WriteIO32( &pReg_Drex->CONCONTROL,
        (0x0    <<  28) |       // [   28] dfi_init_start
        (0xFFF  <<  16) |       // [27:16] timeout_level0
        (0x1    <<  12) |       // [14:12] rd_fetch
        (0x1    <<   8) |       // [    8] empty
        (0x0    <<   6) |       // [ 7: 6] io_pd_con
        (0x1    <<   5) |       // [    5] aref_en - Auto Refresh Counter. Disable:0, Enable:1
        (0x0    <<   3) |       // [    3] update_mode - Update Interface in DFI.
        (0x0    <<   1) |       // [ 2: 1] clk_ratio
        (0x0    <<   0));       // [    0] ca_swap

       //38. Set the CGControl register for clock gating enable.
    WriteIO32( &pReg_Drex->CGCONTROL,
        (0x0    <<   4) |       // [    4] phy_cg_en
        (0x0    <<   3) |       // [    3] memif_cg_en
        (0x0    <<   2) |       // [    2] scg_sg_en
        (0x0    <<   1) |       // [    1] busif_wr_cg_en
        (0x0    <<   0));       // [    0] busif_rd_cg_en
}
#endif    // ddr simulation

    printf("Lock value  = %d\r\n", g_Lock_Val );

    MEMMSG("g_GT_cycle  = 0x%08X\r\n", g_GT_cycle );
    MEMMSG("g_GT_code   = 0x%08X\r\n", g_GT_code );
    MEMMSG("g_RD_vwmc   = 0x%08X\r\n", g_RD_vwmc );
    MEMMSG("g_WR_vwmc   = 0x%08X\r\n", g_WR_vwmc );

    printf("GATE CYC    = 0x%08X\r\n", ReadIO32( &pReg_DDRPHY->SHIFTC_CON ) );
    printf("GATE CODE   = 0x%08X\r\n", ReadIO32( &pReg_DDRPHY->OFFSETC_CON[0] ) );

    printf("Read  DQ    = 0x%08X\r\n", ReadIO32( &pReg_DDRPHY->OFFSETR_CON[0] ) );
    printf("Write DQ    = 0x%08X\r\n", ReadIO32( &pReg_DDRPHY->OFFSETW_CON[0] ) );

    printf("\r\n\r\n");

    return CTRUE;
}
