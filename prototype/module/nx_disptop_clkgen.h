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
#ifndef __NX_DISPTOP_CLKGEN_H__
#define __NX_DISPTOP_CLKGEN_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup
//------------------------------------------------------------------------------
//@{

struct	NX_DISPTOP_CLKGEN_RegisterSet
{
	volatile U32	CLKENB;			///< 0x40 : Clock Enable Register
	volatile U32	CLKGEN[4];			///< 0x44 : Clock Generate Register
};


//enum BCLKMODE
//{
//	BCLKMODE_DISABLE,
//	BCLKMODE_DYNAMIC,
//	BCLKMODE_ALWAYS,
//	BCLKMODE_FORCE32 = 0x7fffffff
//};

//struct ClockGroupRegisterSet
//{
//	volatile U32 CLKENB;
//    volatile U32 CLKGEN[1];
//};
CBOOL	NX_DISPTOP_CLKGEN_Initialize( void );
U32	NX_DISPTOP_CLKGEN_GetNumberOfModule( void );
U32 	NX_DISPTOP_CLKGEN_GetPhysicalAddress( U32 ModuleIndex );
U32	NX_DISPTOP_CLKGEN_GetSizeOfRegisterSet( void );
void	NX_DISPTOP_CLKGEN_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_DISPTOP_CLKGEN_GetBaseAddress( U32 ModuleIndex );

void	NX_DISPTOP_CLKGEN_SetClockPClkMode( U32 ModuleIndex, NX_PCLKMODE mode );
NX_PCLKMODE	NX_DISPTOP_CLKGEN_GetClockPClkMode( U32 ModuleIndex );
void	NX_DISPTOP_CLKGEN_SetClockSource( U32 ModuleIndex, U32 Index, U32 ClkSrc );
U32	NX_DISPTOP_CLKGEN_GetClockSource( U32 ModuleIndex, U32 Index );
void	NX_DISPTOP_CLKGEN_SetClockDivisor( U32 ModuleIndex, U32 Index, U32 Divisor );
U32	NX_DISPTOP_CLKGEN_GetClockDivisor( U32 ModuleIndex, U32 Index );
void	NX_DISPTOP_CLKGEN_SetClockDivisorEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_DISPTOP_CLKGEN_GetClockDivisorEnable( U32 ModuleIndex );

void	NX_DISPTOP_CLKGEN_SetClockBClkMode( U32 ModuleIndex, NX_BCLKMODE mode );
NX_BCLKMODE	NX_DISPTOP_CLKGEN_GetClockBClkMode( U32 ModuleIndex );

//void	NX_DISPTOP_CLKGEN_SetClockInInv( U32 ModuleIndex, CBOOL OutClkInv );
//CBOOL	NX_DISPTOP_CLKGEN_GetClockInInv( U32 ModuleIndex );

void	NX_DISPTOP_CLKGEN_SetClockOutInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
CBOOL	NX_DISPTOP_CLKGEN_GetClockOutInv( U32 ModuleIndex, U32 Index );

CBOOL	NX_DISPTOP_CLKGEN_SetInputInv( U32 ModuleIndex, U32 Index, CBOOL OutClkInv );
CBOOL	NX_DISPTOP_CLKGEN_GetInputInv( U32 ModuleIndex, U32 Index );

void	NX_DISPTOP_CLKGEN_SetClockOutSelect( U32 ModuleIndex, U32 Index, CBOOL bBypass );

#ifdef	__cplusplus
}
#endif

#endif // __NX_DISPTOP_CLKGEN_H__
