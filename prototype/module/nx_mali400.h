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
#ifndef __NX_MALI400_H__
#define __NX_MALI400_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	MALI400
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
typedef struct 
{
	volatile U32 REG_CONTROL [0x1000/4];
	volatile U32 REG_L2      [0x1000/4];
	volatile U32 REG_PMU     [0x1000/4];
	volatile U32 REG_MMU_GP  [0x1000/4];
	volatile U32 REG_MMU_PP0 [0x1000/4];
	volatile U32 REG_MMU_PP1 [0x1000/4];
	volatile U32 REG_MMU_PP2 [0x1000/4];
	volatile U32 REG_MMU_PP3 [0x1000/4];

	volatile U32 REG_PP0     [0x2000/4];
	volatile U32 REG_PP1     [0x2000/4];
	volatile U32 REG_PP2     [0x2000/4];
	volatile U32 REG_PP3     [0x2000/4];
} NX_MALI400_RegisterSet;

//------------------------------------------------------------------------------
///	@name	MALI400 Interface
//------------------------------------------------------------------------------
//@{
U32   NX_MALI400_GetTEMP( U32 ModuleIndex );
//@}
	
//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL NX_MALI400_Initialize( void );
U32   NX_MALI400_GetNumberOfModule( void );

U32   NX_MALI400_GetSizeOfRegisterSet( void );

void  NX_MALI400_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*  NX_MALI400_GetBaseAddress( U32 ModuleIndex );
U32   NX_MALI400_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL NX_MALI400_OpenModule( U32 ModuleIndex );
CBOOL NX_MALI400_CloseModule( U32 ModuleIndex );
CBOOL NX_MALI400_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_MALI400_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_MALI400_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
U32   NX_MALI400_GetInterruptNumber ( U32 ModuleIndex );
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_MALI400_H__
