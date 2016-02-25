/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : PMIC MP8835
 *      File            : pmic_mp8845.h
 *      Description     : 
 *      Author          : Hans
 *      History         : 2015.08.19 Hans create
 */

#ifndef __PMIC_MP8845_H__
#define __PMIC_MP8845_H__

#define MP8845C_REG_VSEL                0x00
#define MP8845C_REG_SYSCNTL1            0x01
#define MP8845C_REG_SYSCNTL2            0x02
#define MP8845C_REG_ID1                 0x03
#define MP8845C_REG_ID2                 0x04
#define MP8845C_REG_STATUS              0x05

#define I2C_ADDR_MP8845                 (0x38 >> 1)  // SVT & ASB

#endif	// ifdef __PMIC_MP8845_H__
