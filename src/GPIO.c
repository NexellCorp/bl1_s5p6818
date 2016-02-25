/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : GPIO.c
 *      File            : 
 *      Description     : 
 *      Author          : Hans
 *      History         : 2013.01.10 First Implementation
 */

#include <nx_peridot.h>
#include <nx_type.h>
#include <nx_debug2.h>

#include <nx_chip.h>

#include <nx_gpio.h>

struct NXPYROPE_GPIO_RegSet
{
	struct NX_GPIO_RegisterSet NXGPIO;
	U8 Reserved[0x1000-sizeof(struct NX_GPIO_RegisterSet)];
};

#if !DIRECT_IO
static struct NXPYROPE_GPIO_RegSet (* const pBaseGPIOReg)[1] = (struct NXPYROPE_GPIO_RegSet (*)[])(PHY_BASEADDR_GPIOA_MODULE);
inline void GPIOSetAltFunction(U32 AltFunc)
{
	register struct NX_GPIO_RegisterSet *pGPIOReg = &pBaseGPIOReg[(AltFunc>>8)&0x7]->NXGPIO;
	 pGPIOReg->GPIOxALTFN[(AltFunc>>7)&0x1] =
	(pGPIOReg->GPIOxALTFN[(AltFunc>>7)&0x1] & ~(0x3UL<<(((AltFunc>>3)&0xF)*2)))
									 | ((AltFunc&0x3)<<(((AltFunc>>3)&0xF)*2));
}
#endif
