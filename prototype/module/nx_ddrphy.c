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

#include "nx_ddrphy.h"

static	struct NX_DDRPHY_RegisterSet *__g_pRegister = CNULL;

//------------------------------------------------------------------------------
// Module Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Initialize of prototype enviroment & local variables.
 *	@return		CTRUE	indicates that Initialize is succeeded.
 *			CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_DDRPHY_Initialize( void )
{
	static CBOOL bInit = CFALSE;

	if( CFALSE == bInit )
	{
		__g_pRegister = (struct NX_DDRPHY_RegisterSet *)CNULL;

		bInit = CTRUE;
	}

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 */
U32	NX_DDRPHY_GetNumberOfModule( void )
{
	return NUMBER_OF_DREX_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32	NX_DDRPHY_GetPhysicalAddress( void )
{
	return	(U32)( PHY_BASEADDR_DREX_MODULE_CH1_APB );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32	NX_DDRPHY_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DDRPHY_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */

void	NX_DDRPHY_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );

	__g_pRegister = (struct NX_DDRPHY_RegisterSet *)BaseAddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */

void*	NX_DDRPHY_GetBaseAddress( void )
{

	return (void*)__g_pRegister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicates that Initialize is succeeded. 
 *			CFALSE	indicates that Initialize is failed.
 */
CBOOL	NX_DDRPHY_OpenModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicates that Deinitialize is succeeded. 
 *			 CFALSE	indicates that Deinitialize is failed.
 */
CBOOL	NX_DDRPHY_CloseModule( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicates that Module is Busy. 
 *			 CFALSE	indicates that Module is NOT Busy.
 */
CBOOL	NX_DDRPHY_CheckBusy( void )
{
	return CFALSE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Indicaes whether the selected modules is ready to enter power-down stage
 *	@return		CTRUE	indicates that Ready to enter power-down stage. 
 *			CFALSE	indicates that This module can't enter to power-down stage.
 */
CBOOL	NX_DDRPHY_CanPowerDown( void )
{
	return CTRUE;
}

//------------------------------------------------------------------------------
// Interrupt Interface
//------------------------------------------------------------------------------
// there is no interrupt interface in ddr phy

//------------------------------------------------------------------------------
//	Clock Management
//--------------------------------------------------------------------------
// there in no clock management feature in ddr phy

//--------------------------------------------------------------------------
// Power Management
//--------------------------------------------------------------------------
void	NX_DDRPHY_SetLowPowerModeEnable ( CBOOL Enable )
{
	;
}
CBOOL	NX_DDRPHY_GetLOwPowerModeEnable ( void )
{
	return CFALSE;
}
void	NX_DDRPHY_WakeupFromLowPowerMode ( void )
{
	;
}
