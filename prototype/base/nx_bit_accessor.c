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
#include "nx_bit_accessor.h"
#include "nx_debug.h"

U32 NX_BIT_SetBitRange32( U32 OldValue, U32 BitValue, U32 MSB, U32 LSB )
{
	U32 mask   = (1UL<<(MSB-LSB)) - 1;
	U32 value0, value1;

	NX_ASSERT( 32 > MSB );
	NX_ASSERT( 32 > LSB );
	NX_ASSERT( MSB >= LSB );

	mask = ((mask<<1)|1) << LSB;
	NX_ASSERT( 0 == ((BitValue<<LSB)&(~mask)) );

	value0 = OldValue & (~mask);
	value1 = BitValue << LSB;

	return value0 | value1;
}

U32 NX_BIT_SetBit32( U32 OldValue, U32 BitValue, U32 BitNumber )
{
	U32 mask, value0, value1;

	NX_ASSERT( 32 > BitNumber );
	NX_ASSERT( 0 == (BitValue&(~1UL)) );

	mask   = (1UL<<BitNumber);
	value0 = OldValue & (~mask);
	value1 = BitValue << BitNumber;

	return value0 | value1;
}

U16 NX_BIT_SetBitRange16( U16 OldValue, U32 BitValue, U32 MSB, U32 LSB )
{
	NX_ASSERT( 16 > MSB );
	NX_ASSERT( 16 > LSB );
	return (U16)NX_BIT_SetBitRange32( (U32)OldValue, BitValue, MSB, LSB );
}

U16 NX_BIT_SetBit16( U16 OldValue, U32 BitValue, U32 BitNumber )
{
	NX_ASSERT( 16 > BitNumber );
	return (U16)NX_BIT_SetBit32( (U32)OldValue, BitValue, BitNumber );
}

U32 NX_BIT_GetBitRange32( U32 Value, U32 MSB, U32 LSB )
{
	U32 mask   = (1UL<<(MSB-LSB)) - 1;

	NX_ASSERT( 32 > MSB );
	NX_ASSERT( 32 > LSB );
	NX_ASSERT( MSB >= LSB );

	mask = ((mask<<1)|1) << LSB;
	return ( Value & mask ) >> LSB;
}

CBOOL NX_BIT_GetBit32( U32 Value, U32 BitNumber )
{
	NX_ASSERT( 32 > BitNumber );
	if( (Value >> BitNumber) & 1UL ){ return CTRUE; }
	return CFALSE;
}

U16 NX_BIT_GetBitRange16( U16 Value, U32 MSB, U32 LSB )
{
	NX_ASSERT( 16 > MSB );
	NX_ASSERT( 16 > LSB );
	return (U16)NX_BIT_GetBitRange32( (U32)Value, MSB, LSB );
}

CBOOL NX_BIT_GetBit16( U16 Value, U32 BitNumber )
{
	NX_ASSERT( 16 > BitNumber );
	return NX_BIT_GetBit32( (U32)Value, BitNumber );
}
