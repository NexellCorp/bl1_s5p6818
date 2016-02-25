/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : Exception Handler
 *      File            : aarch32_exception_handler.c
 *      Description     : 
 *      Author          : Firware Team
 *      History         : 2015.06.10 Hans
 */
#include "sysHeader.h"

void sleepMain(void);
U32 GetSMCCodeFromAArch32(void);
U32 GetSMCCodeFromAArch64(void);
U32 GetSaveAArchMode(void);
U32 GetCurrentSMode(void);
U32 GetCPUID(void);
U32 GetESR_EL3(void);
void SetNS(CBOOL bEnable, U32 tmp); // set non-secure mode

void sync_c_handler_EL3(void)
{
	CBOOL SavedMMode = GetSaveAArchMode();
	U32 smccode;
	U32 rESR = GetESR_EL3();
	U32 cpuid = GetCPUID();

	//    printf("prev PE mode is aarch%d\r\n", 64-(SavedMMode*32));

	if(SavedMMode)
		smccode = GetSMCCodeFromAArch32();
	else
		smccode = GetSMCCodeFromAArch64();

	SetNS(CFALSE, 0);
	//    if(((SavedMMode == 0) && (smccode == 12345)) ||
	//        ((SavedMMode == 1) && (smccode == 12)))
	{
		if(cpuid==0)
		{
			printf("smc code:%d\r\n", smccode);
			printf("ESR:0x%08X\r\n", rESR);
			if(rESR & 1<<25)
				printf("Illigal exception state\r\n");
			else
				if(rESR>>26 == 0)
					printf("unknown excption to el3\r\n");
				else
					printf("EC:%X, ISS:%X\r\n", rESR>>26, rESR & 0x1FFFFFF);
		}

		if(cpuid>0)
		{
			//            __asm__ __volatile__ ("dmb sy");
			do{
				__asm__ __volatile__ ("wfi");
			}while(1);
		}
		else
		{
			printf("smc with suspend request code %d\r\nenter suspend...\r\n", smccode);
			while(!DebugIsTXEmpty());
			while(DebugIsBusy());

			sleepMain();

			printf("machine is resumed at el%d\r\n", GetCurrentSMode());
			while(!DebugIsTXEmpty());
			while(DebugIsBusy());
		}
	}
	/*
	   else
	   {
	   printf("smc code:%d\r\n", smccode);
	   printf("ESR:0x%08X\r\n", rESR);
	   if(rESR & 1<<25)
	   printf("Illigal exception state\r\n");
	   else
	   if(rESR>>26 == 0)
	   printf("unknown excption to el3\r\n");
	   else
	   printf("EC:%X, ISS:%X\r\n", rESR>>26, rESR & 0x1FFFFFF);
	   while(1);
	   }
	 */
}

