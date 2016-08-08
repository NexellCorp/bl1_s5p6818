/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sangjong, Han <hans@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
