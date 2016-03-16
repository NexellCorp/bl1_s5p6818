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
 *      Module          : Base
 *      File            : cfgFreqDefine.h
 *      Description     :
 *      Author          : Russll
 *      History         : 2014.09.29 Russell - First draft
 */

#ifndef __CFG_FREQ_DEFINE_H__
#define __CFG_FREQ_DEFINE_H__

//------------------------------------------------------------------------------
//	oscilator
//------------------------------------------------------------------------------
#define OSC_HZ (24000000)
#define OSC_KHZ (OSC_HZ / 1000)
#define OSC_MHZ (OSC_KHZ / 1000)

#define NX_CLKSRC_PLL_0 (0)
#define NX_CLKSRC_PLL_1 (1)
#define NX_CLKSRC_PLL_2 (2)
#define NX_CLKSRC_PLL_3 (3)

//------------------------------------------------------------------------------
#define NX_CLKSRC_UART (NX_CLKSRC_PLL_0)
#define NX_CLKSRC_SDMMC (NX_CLKSRC_PLL_2)
#define NX_CLKSRC_SPI (NX_CLKSRC_PLL_0)

#endif /* __CFG_FREQ_DEFINE_H__ */
