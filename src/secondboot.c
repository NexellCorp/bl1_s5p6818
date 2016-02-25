/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : 
 *      File            : secondboot.c
 *      Description     : 
 *      Author          : Hans
 *      History         : 2014.08.20 Hans modift for Peridot
 *			  2013.01.10 Hans create
 */

#define __SET_GLOBAL_VARIABLES
#include "sysHeader.h"


#define SMEM_TEST           (0)

#define EMA_VALUE           (1)     // Manual setting - 1: 1.1V, 3: 1.0V, 4: 0.95V

extern void     DMC_Delay(int milisecond);

//extern void     flushICache(void);
//extern void     enableICache(CBOOL enable);

extern void     enterSelfRefresh(void);
extern void     exitSelfRefresh(void);
extern void     set_bus_config(void);
extern void     set_drex_qos(void);

extern CBOOL    iUSBBOOT(struct NX_SecondBootInfo * const pTBI);
extern CBOOL    iUARTBOOT(struct NX_SecondBootInfo * const pTBI);
extern CBOOL    iSPIBOOT(struct NX_SecondBootInfo * const pTBI);
extern CBOOL    iSDXCBOOT(struct NX_SecondBootInfo * const pTBI);
extern CBOOL    iNANDBOOTEC(struct NX_SecondBootInfo * const pTBI);
extern CBOOL    iSDXCFSBOOT(struct NX_SecondBootInfo * const pTBI);
extern void     initClock(void);
#ifdef MEM_TYPE_DDR3
extern CBOOL    init_DDR3(U32);
#endif
#ifdef MEM_TYPE_LPDDR23
extern CBOOL    init_LPDDR3(U32);
#endif
extern CBOOL    buildinfo(void);

extern void     printClkInfo(void);

extern void     ResetCon(U32 devicenum, CBOOL en);
extern CBOOL    SubCPUBringUp(U32 CPUID);

extern void initPMIC(void);
extern void dowakeup(void);
extern void RomUSBBoot(U32 RomJumpAddr);
extern void SetSecureState(void);
void SimpleMemoryTest(U32 * pStart,U32 * pEnd);

//------------------------------------------------------------------------------
#if (CCI400_COHERENCY_ENABLE == 1)
void initCCI400(void)
{
	//before set barrier instruction.
	SetIO32( &pReg_CCI400->SCR, 1<<0);                      // static bus disable speculative fetches
	SetIO32( &pReg_CCI400->SCR, 1<<1);                      // SFR bus disable speculative fetches

	WriteIO32( &pReg_CCI400->COR,   (1UL<<3) );             // protect to send barrier command to drex

	WriteIO32( &pReg_CCI400->CSI[BUSID_CS].SCR,     0 );    // snoop request disable
	WriteIO32( &pReg_CCI400->CSI[BUSID_CODA].SCR,   0 );    // snoop request disable
	WriteIO32( &pReg_CCI400->CSI[BUSID_TOP].SCR,    0 );    // snoop request disable

#if (MULTICORE_BRING_UP == 1)
	WriteIO32( &pReg_CCI400->CSI[BUSID_CPUG0].SCR,  0x3 );      // cpu 0~3 Snoop & DVM Req
	while(ReadIO32(&pReg_CCI400->STSR) & 0x1);

	WriteIO32( &pReg_CCI400->CSI[BUSID_CPUG1].SCR,  0x3 );      // cpu 4~7 Snoop & DVM Req
	while(ReadIO32(&pReg_CCI400->STSR) & 0x1);
#else
	WriteIO32( &pReg_CCI400->CSI[BUSID_CPUG0].SCR,  0x0 );
	WriteIO32( &pReg_CCI400->CSI[BUSID_CPUG1].SCR,  0x0 );
#endif
}
#endif  // #if (CCI400_COHERENCY_ENABLE == 1)


struct NX_CLKPWR_RegisterSet * const clkpwr;

//------------------------------------------------------------------------------
void BootMain( U32 CPUID )
{
	struct NX_SecondBootInfo    TBI;
	struct NX_SecondBootInfo * pTBI = &TBI;    // third boot info
	CBOOL Result = CFALSE;
	register volatile U32 temp;
	U32 sign, isResume = 0;

	//--------------------------------------------------------------------------
	// Set EMA
	//--------------------------------------------------------------------------

	// Set EMA for CPU Cluster0
	temp  = ReadIO32( &pReg_Tieoff->TIEOFFREG[94] ) & ~((0x7 << 23) | (0x7 << 17));
	temp |= ((EMA_VALUE << 23) | (EMA_VALUE << 17));
	WriteIO32( &pReg_Tieoff->TIEOFFREG[94], temp);

	// Set EMA for CPU Cluster1
	temp  = ReadIO32( &pReg_Tieoff->TIEOFFREG[111] ) & ~((0x7 << 23) | (0x7 << 17));
	temp |= ((EMA_VALUE << 23) | (EMA_VALUE << 17));
	WriteIO32( &pReg_Tieoff->TIEOFFREG[111], temp);

	//--------------------------------------------------------------------------
	// Set Affinity ID
	//--------------------------------------------------------------------------
#if (CONFIG_RESET_AFFINITY_ID == 1)
	// Set Affinity level1 for CPU Cluster1
	temp  = ReadIO32( &pReg_Tieoff->TIEOFFREG[95] ) & 0x00FFFFFF;
	temp |= (1 << 24);
	WriteIO32( &pReg_Tieoff->TIEOFFREG[95], temp);

	// Set Affinity level2 for CPU Cluster1
	temp  = ReadIO32( &pReg_Tieoff->TIEOFFREG[96] ) & 0xF0;
	//    temp |= (1 << 0);
	WriteIO32( &pReg_Tieoff->TIEOFFREG[96], temp);
#endif

	//--------------------------------------------------------------------------
	// Init debug
	//--------------------------------------------------------------------------
	DebugInit();

	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);

	if(USBREBOOT_SIGNATURE == ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE5))
		RomUSBBoot((U32)0x0000009C);

#if (BOOTCOUNT == 1)
	U32 RBCount = ReadIO32(&pReg_RTC->RTCSCRATCH);
	pSBI->BootCount = RBCount & 0xFFFFFF;
	pSBI->ResetCount = RBCount>>24;
	if(pSBI->BootCount > 0x100000)
	{
		pSBI->BootCount = 0;
		pSBI->ResetCount = 0;
	}
	pSBI->BootCount++;
	WriteIO32(&pReg_RTC->RTCSCRATCH, (pSBI->ResetCount<<24) | pSBI->BootCount);

	printf("Reset Count :\t%d\r\nBoot Count :\t%d\r\n", pSBI->ResetCount, pSBI->BootCount);
#endif
	U8 resetcount = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG) & 0xFF;
	WriteIO32( &pReg_Alive->ALIVESCRATCHRSTREG, 0xFF);        // clear reboot marker
	if(resetcount != 0)
	{
		U8 xorresetcount = ((resetcount>>4)&0xF)^0xF;
		resetcount &= 0xF;
		if(resetcount == xorresetcount)     // scratch register valid test, only used LSByte
		{
			if(resetcount>3)
			{
				printf("scratch is not cleared, try rom usb boot\r\n");
				// it's watchdog reboot so try usb boot
				WriteIO32( &pReg_Alive->ALIVESCRATCHSETREG, 0xF0);        // reset reboot marker
				RomUSBBoot((U32)0x0000009C);
			}
			else
			{
				resetcount++;
				printf("boot retry count is %d\r\n", resetcount);
				resetcount |= ((resetcount^0xF)<<4);
				WriteIO32( &pReg_Alive->ALIVESCRATCHSETREG, resetcount);
#if (BOOTCOUNT == 1)
				pSBI->ResetCount++;
				WriteIO32(&pReg_RTC->RTCSCRATCH, (pSBI->ResetCount<<24) | pSBI->BootCount);
#endif
			}
		}else
		{
			// first boot, so alive scratch register is broken. reset count
			WriteIO32( &pReg_Alive->ALIVESCRATCHSETREG, 0xE1);        // first count;
			printf("first boot, scratch is cleared\r\n");
		}
	}
	else
	{
		// alive reset code is broken. reset count
		WriteIO32( &pReg_Alive->ALIVESCRATCHSETREG, 0xE1);        // first count;
		printf("scratch is broken, clear\r\n");
	}
	{
#if !defined( LOAD_FROM_USB )
		// set watchdog timer
		printf("watchdog timer start\r\n");
		SetIO32(&pReg_RstCon->REGRST[RESETINDEX_OF_WDT_MODULE_PRESETn>>5], 1<<(RESETINDEX_OF_WDT_MODULE_PRESETn&0x1F));
		SetIO32(&pReg_RstCon->REGRST[RESETINDEX_OF_WDT_MODULE_nPOR>>5], 1<<(RESETINDEX_OF_WDT_MODULE_nPOR&0x1F));
		WriteIO32( &pReg_WDT->WTCON, 0xFF<<8 |          // prescaler value
				0x03<<3 |          // division factor (3:128)
				0x01<<2);          // watchdog reset enable
		WriteIO32( &pReg_WDT->WTCNT, 0xFFFF);           // 200MHz/256/128 = 6103.515625, 65536/6103.5 = 10.74 sec
		//        SetIO32  ( &pReg_WDT->WTCON, 0x01<<5);          // watchdog timer enable
#endif
	}

	//--------------------------------------------------------------------------
	// Get resume information.
	//--------------------------------------------------------------------------
	sign = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG);
	if ((SUSPEND_SIGNATURE == (sign&0xFFFFFF00)) && ReadIO32(&pReg_Alive->WAKEUPSTATUS))
	{
		isResume = 1;
	}

	//--------------------------------------------------------------------------
	// Initialize PMIC device.
	//--------------------------------------------------------------------------
#if defined( INITPMIC_YES )
	if (isResume == 0)
		initPMIC();
#endif


	SYSMSG("EMA is %s\r\n", (EMA_VALUE==1)?"1.1V":(EMA_VALUE==3)?"1.0V":"0.95V");


	printf("\r\n\nworking to aarch%d\r\nwaiting for pll change..\r\n", sizeof(void*)*8);

	while(!DebugIsUartTxDone());


	//--------------------------------------------------------------------------
	// Change to PLL.
	//--------------------------------------------------------------------------
	initClock();

	//--------------------------------------------------------------------------
	// Debug Console
	//--------------------------------------------------------------------------
	DebugInit();

	//--------------------------------------------------------------------------
	// build information. version, build time and date
	//--------------------------------------------------------------------------
#if 1
	buildinfo();
#else
	if ( buildinfo() == CFALSE )
	{
		printf( "WARNING : NSIH mismatch...!!!\r\n" );
		while(1);
	}
#endif

	//--------------------------------------------------------------------------
	// print clock information
	//--------------------------------------------------------------------------
	printClkInfo();


	printf("\r\nDDR3 POR Init Start %d\r\n", isResume);
#ifdef MEM_TYPE_DDR3
#if 0
	if (init_DDR3(isResume) == CFALSE)
		init_DDR3(isResume);
#else
	if (init_DDR3(0) == CFALSE)
		init_DDR3(0);
#endif
#endif
#ifdef MEM_TYPE_LPDDR23
	if (init_LPDDR3(0) == CFALSE)
		init_LPDDR3(0);
#endif

	if (isResume)
	{
		exitSelfRefresh();
	}

	printf( "DDR3 Init Done!\r\n" );

	set_bus_config();
	set_drex_qos();

#if (CCI400_COHERENCY_ENABLE == 1)
	printf( "CCI Init!\r\n" );
	initCCI400();
#endif

	SetSecureState();


	printf( "Wakeup CPU " );

#if (MULTICORE_BRING_UP == 1)
	SubCPUBringUp(CPUID);
#endif

	if (isResume)
	{
		printf( " DDR3 SelfRefresh exit Done!\r\n0x%08X\r\n", ReadIO32(&pReg_Alive->WAKEUPSTATUS) );
		dowakeup();
	}
	WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0);

	if (pSBI->SIGNATURE != HEADER_ID)
		printf( "2nd Boot Header is invalid, Please check it out!\r\n" );

#if (SMEM_TEST == 1)
	SimpleMemoryTest((U32*)0x40000000UL, (U32*)0x50000000UL);
#endif

#if defined( LOAD_FROM_USB )
	printf( "Loading from usb...\r\n" );
	Result = iUSBBOOT(pTBI);            // for USB boot
#endif
#if defined( LOAD_FROM_SPI )
	printf( "Loading from spi...\r\n" );
	Result = iSPIBOOT(pTBI);            // for SPI boot
#endif
#if defined( LOAD_FROM_SDMMC )
	printf( "Loading from sdmmc...\r\n" );
	Result = iSDXCBOOT(pTBI);           // for SD boot
#endif
#if defined( LOAD_FROM_SDFS )
	printf( "Loading from sd FATFS...\r\n" );
	Result = iSDXCFSBOOT(pTBI);         // for SDFS boot
#endif
#if defined( LOAD_FROM_NAND )
	printf( "Loading from nand...\r\n" );
	Result = iNANDBOOTEC(pTBI);         // for NAND boot
#endif
#if defined( LOAD_FROM_UART )
	printf( "Loading from uart...\r\n" );
	Result = iUARTBOOT(pTBI);           // for UART boot
#endif

#if defined( LOAD_FROM_ALL )
	switch(pSBI->DBI.SPIBI.LoadDevice)
	{
		case BOOT_FROM_USB:
			printf( "Loading from usb...\r\n" );
			Result = iUSBBOOT(pTBI);        // for USB boot
			break;
		case BOOT_FROM_SPI:
			printf( "Loading from spi...\r\n" );
			Result = iSPIBOOT(pTBI);        // for SPI boot
			break;
#if 0
		case BOOT_FROM_NAND:
			printf( "Loading from nand...\r\n" );
			Result = iNANDBOOTEC(pTBI);     // for NAND boot
			break;
#endif
		case BOOT_FROM_SDMMC:
			printf( "Loading from sdmmc...\r\n" );
			Result = iSDXCBOOT(pTBI);       // for SD boot
			break;
		case BOOT_FROM_SDFS:
			printf( "Loading from sd FATFS...\r\n" );
			Result = iSDXCFSBOOT(pTBI);     // for SDFS boot
			break;
#if 0
		case BOOT_FROM_UART:
			printf( "Loading from uart...\r\n" );
			Result = iUARTBOOT(pTBI);       // for UART boot
			break;
#endif
	}
#endif


	if(Result)
	{
		void (*pLaunch)(U32,U32) = (void(*)(U32,U32))((MPTRS)pTBI->LAUNCHADDR);
		printf( " Image Loading Done!\r\n" );
		printf( "Launch to 0x%08X\r\n", (MPTRS)pLaunch );
		temp = 0x10000000;
		while(!DebugIsUartTxDone() && temp--);
		pLaunch(0, 4330);
	}

	printf( " Image Loading Failure Try to USB boot\r\n" );
	temp = 0x10000000;
	while(!DebugIsUartTxDone() && temp--);
	RomUSBBoot((U32)0x0000009C);
	while(1);
}

#if (SMEM_TEST == 1)
void SimpleMemoryTest(U32 *pStart, U32 *pEnd)
{
	volatile U32 *ptr = pStart;

	printf("memory test start!\r\n");

	printf("\r\nmemory write data to own address\r\n");
	while(ptr<pEnd)
	{
		*ptr = (U32)((MPTRS)ptr);
		if(((U32)((MPTRS)ptr) & 0x3FFFFFL) == 0)
			printf("0x%16X:\r\n", ptr);
		ptr++;
	}

	printf("\r\nmemory compare with address and own data\r\n");
	ptr = pStart;
	while(ptr<pEnd)
	{
		if(*ptr != (U32)((MPTRS)ptr))
			printf("0x%08X: %16x\r\n", (U32)((MPTRS)ptr), *ptr);
		ptr++;
		if((((MPTRS)ptr) & 0xFFFFFL) == 0)
			printf("0x%16X:\r\n", ptr);
	}

	printf("bit shift test....\r\n");
	printf("write data....\r\n");
	ptr = pStart;
	while(ptr<pEnd)
	{
		*ptr = (1UL<<((((MPTRS)ptr) & 0x1F<<2)>>2));
		ptr++;
	}
	printf("compare data....\r\n");
	ptr = pStart;
	while(ptr<pEnd)
	{
		if(*ptr != (1UL<<((((MPTRS)ptr) & 0x1F<<2)>>2)))
			printf("0x%16x\r\n", *ptr);
		ptr++;
	}
	printf("reverse bit test\r\n");
	printf("write data....\r\n");
	ptr = pStart;
	while(ptr<pEnd)
	{
		*ptr = ~(1UL<<((((MPTRS)ptr) & 0x1F<<2)>>2));
		ptr++;
	}
	printf("compare data....\r\n");
	ptr = pStart;
	while(ptr<pEnd)
	{
		if(*ptr != ~(1UL<<((((MPTRS)ptr) & 0x1F<<2)>>2)))
			printf("0x%16x\r\n", *ptr);
		ptr++;
	}

	printf("\r\nmemory test done\r\n");
}

#endif
