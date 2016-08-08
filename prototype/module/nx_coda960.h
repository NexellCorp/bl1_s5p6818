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
#ifndef __NX_CODA960_H__
#define __NX_CODA960_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	CODA960
//------------------------------------------------------------------------------
//@{

//--------------------------------------------------------------------------
/// @brief	register map
typedef struct
{
	volatile U32 TEMP;
	volatile U32 INTCTRL;
	volatile U32 INTPEND;
} NX_CODA960_RegisterSet;

//------------------------------------------------------------------------------
///	@name	CODA960 Interface
//------------------------------------------------------------------------------
//@{
U32	NX_CODA960_GetTEMP( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL	NX_CODA960_Initialize( void );
U32	NX_CODA960_GetNumberOfModule( void );

U32	NX_CODA960_GetSizeOfRegisterSet( void );
void	NX_CODA960_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_CODA960_GetBaseAddress( U32 ModuleIndex );
U32	NX_CODA960_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL	NX_CODA960_OpenModule( U32 ModuleIndex );
CBOOL	NX_CODA960_CloseModule( U32 ModuleIndex );
CBOOL	NX_CODA960_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32	NX_CODA960_GetClockNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32	NX_CODA960_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
typedef enum
{
	NX_CODA960_INT_TEST0 = 0, ///< test0 interrupt
	NX_CODA960_INT_TEST1 = 1, ///< test1 interrupt
	NX_CODA960_INT_TEST2 = 2, ///< test2 interrupt
} NX_CODA960_INT;

U32 	NX_CODA960_GetInterruptNumber ( U32 ModuleIndex );
void	NX_CODA960_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_CODA960_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_CODA960_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_CODA960_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_CODA960_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_CODA960_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_CODA960_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_CODA960_ClearInterruptPendingAll( U32 ModuleIndex );
S32	NX_CODA960_GetInterruptPendingNumber( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	DMA Interface
//------------------------------------------------------------------------------
//@{
/// @brief	DMA index for DMAChannelIndex
typedef enum
{
	NX_CODA960_DMA_TXDMA = 0, ///< TX channel
	NX_CODA960_DMA_RXDMA = 1, ///< RX channel
}	NX_CODA960_DMA;

U32 NX_CODA960_GetDMANumber ( U32 ModuleIndex, U32 DMAChannelIndex );
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_CODA960_H__
