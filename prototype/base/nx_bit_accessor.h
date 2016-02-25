/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *	Module		: Base
 *      File            : nx_bit_accessor.h                                                      
 *      Description     :                                                      
 *      Author          : Firware Team                                          
 *      History         :                                                       
 */
#ifndef _NX_BIT_ACCESSOR_H_
#define _NX_BIT_ACCESSOR_H_

#include <nx_type.h>

#ifdef __cplusplus
extern "C" {
#endif

U32 NX_BIT_SetBitRange32( U32 OldValue, U32 BitValue, U32 MSB, U32 LSB );
U32 NX_BIT_SetBit32( U32 OldValue, U32 BitValue, U32 BitNumber );
U16 NX_BIT_SetBitRange16( U16 OldValue, U32 BitValue, U32 MSB, U32 LSB );
U16 NX_BIT_SetBit16( U16 OldValue, U32 BitValue, U32 BitNumber );
U32 NX_BIT_GetBitRange32( U32 Value, U32 MSB, U32 LSB );
CBOOL NX_BIT_GetBit32( U32 Value, U32 BitNumber );
U16 NX_BIT_GetBitRange16( U16 Value, U32 MSB, U32 LSB );
CBOOL NX_BIT_GetBit16( U16 Value, U32 BitNumber );

#ifdef	__cplusplus
}
#endif  

#endif  // _NX_BIT_ACCESSOR_H_
