/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : Reset Controller
 *      File            : resetcon.c
 *      Description     : 
 *      Author          : Hans
 *      History         : 2013.01.10 First Implementation
 */

#include "sysHeader.h"

void ResetCon(U32 devicenum, CBOOL en)
{
    if (en)
        ClearIO32( &pReg_RstCon->REGRST[(devicenum>>5)&0x3],    (0x1<<(devicenum&0x1F)) );  // reset
    else
        SetIO32  ( &pReg_RstCon->REGRST[(devicenum>>5)&0x3],    (0x1<<(devicenum&0x1F)) );  // reset negate
}
