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
 *      File            : aarch64_exception_handler.c
 *      Description     : 
 *      Author          : Firmware Team
 *      History         : 2015.07.02 Hans
 */


#include "sysHeader.h"

void sleepMain(void);
U32 GetCPUID(void);
U32 GetSMCCode(void*);
U32 GetCurrentSMode(void);

void aarch32_monitor(unsigned int lr)
{
	U32 smccode = GetSMCCode((void*)lr);

	if(smccode == 12)
	{
		if(GetCPUID()>0)
		{
			__asm__ __volatile__ ("dmb sy");
			__asm__ __volatile__ ("wfi");
		}
		else
		{
			printf("smc with suspend request code %d\r\nenter suspend...\r\n", smccode);
			while(!DebugIsTXEmpty());
			while(DebugIsBusy());

			sleepMain();

			printf("machine is resumed at mode 0x%x\r\n", GetCurrentSMode());
			while(!DebugIsTXEmpty());
			while(DebugIsBusy());
		}
	}else
	{
		printf("unknown excption to Monitor mode\r\n");
	}
}

