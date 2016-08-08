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
#include "nx_tieoff.h"

static struct NX_TIEOFF_RegisterSet *__g_pRegister;


CBOOL	NX_TIEOFF_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister =(struct NX_TIEOFF_RegisterSet *) CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

U32 NX_TIEOFF_GetPhysicalAddress( void )
{
	const U32 PhysicalAddr[] =  {   PHY_BASEADDR_LIST( TIEOFF )  }; // PHY_BASEADDR_TIEOFF_MODULE

	NX_CASSERT( NUMBER_OF_TIEOFF_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );

	return (U32)PhysicalAddr[0];
}


void	NX_TIEOFF_SetBaseAddress( void* BaseAddress )

{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_TIEOFF_RegisterSet *)BaseAddress;
}

void		NX_TIEOFF_Set(U32 tieoff_index, U32 tieoff_value)
{
	U32 regindex, mask;
	U32 LSB, MSB;
	U32 regval;

	U32 Position;
	U32 BitWidth;
	Position = tieoff_index & 0xffff;
	BitWidth = (tieoff_index>>16) & 0xffff;

	regindex	= Position>>5;

	LSB = Position & 0x1F;
	MSB = LSB+BitWidth;

	if(MSB > 32)
	{
		MSB &= 0x1F;
		mask		= ~(0xffffffff<<LSB);
		regval		= ReadIO32(&__g_pRegister->TIEOFFREG[regindex]) & mask;
		regval		|= ((tieoff_value & ((1UL<<BitWidth)-1))<<LSB);
		WriteIO32(&__g_pRegister->TIEOFFREG[regindex], regval);

		mask	= (0xffffffff<<MSB);
		regval		= ReadIO32(&__g_pRegister->TIEOFFREG[regindex+1]) & mask;
		regval		|= ((tieoff_value & ((1UL<<BitWidth)-1))>>MSB);
		WriteIO32(&__g_pRegister->TIEOFFREG[regindex+1], regval);
	}else
	{
		mask		= (0xffffffff<<MSB) | (~(0xffffffff<<LSB));
		regval		= ReadIO32(&__g_pRegister->TIEOFFREG[regindex]) & mask;
		regval		|= ((tieoff_value & ((1UL<<BitWidth)-1))<<LSB);
		WriteIO32(&__g_pRegister->TIEOFFREG[regindex], regval);
	}
}

U32			NX_TIEOFF_Get(U32 tieoff_index)
{
	U32 regindex, mask;
	U32 LSB, MSB;
	U32 regval;

	U32 Position;
	U32 BitWidth;
	Position = tieoff_index & 0xffff;
	BitWidth = (tieoff_index>>16) & 0xffff;

	regindex	= Position/32;
	LSB = Position%32;
	MSB = LSB+BitWidth;

	if(MSB > 32)
	{
		MSB &= 0x1F;
		mask		= 0xffffffff<<LSB;
		regval		= ReadIO32(&__g_pRegister->TIEOFFREG[regindex]) & mask;
		regval		>>= LSB;

		mask		= ~(0xffffffff<<MSB);
		regval		|= ((ReadIO32(&__g_pRegister->TIEOFFREG[regindex+1]) & mask) << (32-LSB));
	}else
	{
		mask		= ~(0xffffffff<<MSB) & (0xffffffff<<LSB);
		regval		= ReadIO32(&__g_pRegister->TIEOFFREG[regindex]) & mask;
		regval		>>= LSB;
	}
	return regval;
}

