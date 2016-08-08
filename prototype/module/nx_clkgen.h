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
#ifndef __NX_CLKGEN_H__
#define __NX_CLKGEN_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup
//------------------------------------------------------------------------------
//@{

struct	NX_CLKGEN_RegisterSet
{
	volatile U32	CLKENB;		///< 0x40 : Clock Enable Register
	volatile U32	CLKGEN[4];	///< 0x44 : Clock Generate Register
};

CBOOL	NX_CLKGEN_Initialize( void );
U32	NX_CLKGEN_GetNumberOfModule( void );
U32 	NX_CLKGEN_GetPhysicalAddress( U32 ModuleIndex );
U32	NX_CLKGEN_GetSizeOfRegisterSet( void );
void	NX_CLKGEN_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
CBOOL	NX_CLKGEN_Initialize( void );
U32	NX_CLKGEN_GetNumberOfModule( void );
U32 	NX_CLKGEN_GetPhysicalAddress( U32 ModuleIndex );
U32	NX_CLKGEN_GetSizeOfRegisterSet( void );
void	NX_CLKGEN_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_CLKGEN_GetBaseAddress( U32 ModuleIndex );


void	NX_CLKGEN_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode );
NX_BCLKMODE	NX_CLKGEN_GetClockBClkMode( U32 ModuleIndex );
void	NX_CLKGEN_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode );
NX_PCLKMODE	NX_CLKGEN_GetClockPClkMode( U32 ModuleIndex );
void	NX_CLKGEN_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc );
U32	NX_CLKGEN_GetClockSource( U32 ModuleIndex, U32 Index );
void	NX_CLKGEN_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor );
U32	NX_CLKGEN_GetClockDivisor( U32 ModuleIndex, U32 Index );
void	NX_CLKGEN_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_CLKGEN_GetClockDivisorEnable( U32 ModuleIndex );

void	NX_CLKGEN_SetClockOutInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
CBOOL	NX_CLKGEN_GetClockOutInv( U32 ModuleIndex, U32 Index );

void	NX_CLKGEN_SetInputInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
CBOOL	NX_CLKGEN_GetInputInv( U32 ModuleIndex, U32 Index );


#ifdef	__cplusplus
}
#endif

#endif // __NX_CLKGEN_H__
