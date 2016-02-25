/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : Base
 *      File            : cfgBootDefine.h
 *      Description     : 
 *      Author          : Russell
 *      History         : 2014.09.29 Russell - First draft
 */

#ifndef __CFG_BOOT_DEFINE_H__
#define __CFG_BOOT_DEFINE_H__

//------------------------------------------------------------------------------
//  Load from NSIH On/Off.
//------------------------------------------------------------------------------
#define CFG_NSIH_EN         (1)


//------------------------------------------------------------------------------
//  Chip product
//------------------------------------------------------------------------------
//#define ARCH_S5P4418
//#define ARCH_NXP4330
#define ARCH_NXP5430


//------------------------------------------------------------------------------
//  Memory Type
//------------------------------------------------------------------------------
#ifdef MEMTYPE_DDR3
#define MEM_TYPE_DDR3
#endif
#ifdef MEMTYPE_LPDDR3
#define MEM_TYPE_LPDDR23
#endif

//------------------------------------------------------------------------------
//  System optional.
//------------------------------------------------------------------------------
#define MULTICORE_SLEEP_CONTROL     (1)
#define MULTICORE_BRING_UP          (1)
#define CCI400_COHERENCY_ENABLE     (1)
#define CONFIG_RESET_AFFINITY_ID    (0) // Dependent kernel 3.18.x


#endif  //	__CFG_BOOT_DEFINE_H__
