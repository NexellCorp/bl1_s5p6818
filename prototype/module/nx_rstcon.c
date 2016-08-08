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
#include "nx_rstcon.h"

static struct NX_RSTCON_RegisterSet *__g_pRegister;


CBOOL	NX_RSTCON_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = CNULL;
		bInit = CTRUE;
	}
	return CTRUE;
}

U32  NX_RSTCON_GetPhysicalAddress( void )
{
	const U32 PhysicalAddr[] =  {   PHY_BASEADDR_LIST( RSTCON )  }; // PHY_BASEADDR_RSTCON_MODULE
	NX_CASSERT( NUMBER_OF_RSTCON_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	NX_ASSERT( PHY_BASEADDR_RSTCON_MODULE == PhysicalAddr[0] );

	return (U32)PhysicalAddr[0];
}

U32	 NX_RSTCON_GetSizeOfRegisterSet( void )
{
	return sizeof(struct NX_RSTCON_RegisterSet);
}


void NX_RSTCON_SetBaseAddress( void* BaseAddress )

{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_RSTCON_RegisterSet *)BaseAddress;
}


void* NX_RSTCON_GetBaseAddress( void )

{

	return (void*)__g_pRegister;

}

void	NX_RSTCON_SetRST(U32 RSTIndex, RSTCON STATUS)
{
	U32 regvalue;

	NX_ASSERT(NUMBER_OF_RESET_MODULE_PIN > RSTIndex);
	NX_ASSERT((RSTCON_ASSERT == STATUS) || (RSTCON_NEGATE == STATUS));

	regvalue	= ReadIO32(&__g_pRegister->REGRST[RSTIndex >> 5]);

	regvalue	&= ~(1UL << (RSTIndex & 0x1f));
	regvalue	|= (STATUS & 0x01) << (RSTIndex & 0x1f);

	WriteIO32(&__g_pRegister->REGRST[RSTIndex >> 5], regvalue);
}

RSTCON	NX_RSTCON_GetRST(U32 RSTIndex)
{
	NX_ASSERT(NUMBER_OF_RESET_MODULE_PIN > RSTIndex);

	return	(RSTCON)((ReadIO32(&__g_pRegister->REGRST[RSTIndex >> 5])>> (RSTIndex & 0x1f)) & 0x1);
}

