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
#include "nx_displaytop.h"

NX_CASSERT( NUMBER_OF_DISPLAYTOP_MODULE == 1 );

//------------------------------------------------------------------------------
//
//	DISPLAYTOP Interface
//
//------------------------------------------------------------------------------
static	struct
{
	struct NX_DISPLAYTOP_RegisterSet *pRegister;
} __g_ModuleVariables = { CNULL, };

//	Module Interface
//------------------------------------------------------------------------------

/**
 *	@brief	Initialize of prototype enviroment & local variables.
 *	@return CTRUE	indicate that Initialize is successed.\n
 *		CFALSE	indicate that Initialize is failed.
 *	@see	NX_DISPLAYTOP_GetNumberOfModule
 */
CBOOL	NX_DISPLAYTOP_Initialize( void )
{
	static CBOOL bInit = CFALSE;
	U32 i;

	if( CFALSE == bInit )
	{
		for(i=0; i<NUMBER_OF_DISPLAYTOP_MODULE; i++)
		{
			__g_ModuleVariables.pRegister = CNULL;
		}
		bInit = CTRUE;
	}
	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get number of modules in the chip.
 *	@return		Module's number.
 *	@see		NX_DISPLAYTOP_Initialize
 */
U32	NX_DISPLAYTOP_GetNumberOfModule( void )
{
	return NUMBER_OF_DISPLAYTOP_MODULE;
}

//------------------------------------------------------------------------------
// Basic Interface
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
 *	@brief		Get module's physical address.
 *	@return		Module's physical address
 */
U32	NX_DISPLAYTOP_GetPhysicalAddress( void )
{
	static const U32 PhysicalAddr[] = { PHY_BASEADDR_LIST( DISPLAYTOP ) }; // PHY_BASEADDR_UART?_MODULE
	NX_CASSERT( NUMBER_OF_DISPLAYTOP_MODULE == (sizeof(PhysicalAddr)/sizeof(PhysicalAddr[0])) );
	return (U32)(PhysicalAddr[0] + PHY_BASEADDR_DISPLAYTOP_MODULE_OFFSET);
}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a size, in byte, of register set.
 *	@return		Size of module's register set.
 */
U32		NX_DISPLAYTOP_GetSizeOfRegisterSet( void )
{
	return sizeof( struct NX_DISPLAYTOP_RegisterSet );
}

//------------------------------------------------------------------------------
/**
 *	@brief		Set a base address of register set.
 *	@param[in]	BaseAddress Module's base address
 *	@return		None.
 */
void	NX_DISPLAYTOP_SetBaseAddress( void* BaseAddress )
{
	NX_ASSERT( CNULL != BaseAddress );
	__g_ModuleVariables.pRegister = (struct NX_DISPLAYTOP_RegisterSet *)BaseAddress;

}

//------------------------------------------------------------------------------
/**
 *	@brief		Get a base address of register set
 *	@return		Module's base address.
 */
void*	NX_DISPLAYTOP_GetBaseAddress( void )
{

	return (void*)__g_ModuleVariables.pRegister;
}


//------------------------------------------------------------------------------
/**
 *	@brief		Initialize selected modules with default value.
 *	@return		CTRUE	indicate that Initialize is successed. \n
 *			CFALSE	indicate that Initialize is failed.
 */
CBOOL	NX_DISPLAYTOP_OpenModule( void )
{

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@brief		Deinitialize selected module to the proper stage.
 *	@return		CTRUE	indicate that Deinitialize is successed. \n
 *			CFALSE	indicate that Deinitialize is failed.
 */
CBOOL	NX_DISPLAYTOP_CloseModule(  )
{

	return CTRUE;
}

//------------------------------------------------------------------------------
/**
 *	@name		NX_DISPLAYTOP_CheckBusy
 *	@brief		Indicates whether the selected modules is busy or not.
 *	@return		CTRUE	indicate that Module is Busy. \n
 *			CFALSE	indicate that Module is NOT Busy.
 *												NX_DISPLAYTOP_CanPowerDown
 *	@see also	Status register, SSPSR
 */
CBOOL	NX_DISPLAYTOP_CheckBusy(  )
{

	return CFALSE;
}

//------------------------------------------------------------------------------
///	@name	Basic MUX SEL Function
//@{
void	NX_DISPLAYTOP_SetRESCONVMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );

	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->RESCONV_MUX_CTRL, (U32)regvalue);
}

void	NX_DISPLAYTOP_SetHDMIMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (bEnb == CTRUE) || (bEnb == CFALSE) );


	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->INTERCONV_MUX_CTRL, (U32)regvalue);
}

void	NX_DISPLAYTOP_SetMIPIMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (bEnb == CTRUE) || (bEnb == CFALSE) );


	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->MIPI_MUX_CTRL, (U32)regvalue);
}

void	NX_DISPLAYTOP_SetLVDSMUX( CBOOL bEnb, U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	NX_ASSERT( (bEnb == CTRUE) || (bEnb == CFALSE) );


	pRegister = __g_ModuleVariables.pRegister;

	regvalue = (bEnb<<31) | (SEL<<0);
	WriteIO32(&pRegister->LVDS_MUX_CTRL, (U32)regvalue);
}

U32	NX_DISPLAYTOP_GetResetNumber ( void )
{
	// todo
    const U32 ResetPinNumber[NUMBER_OF_DISPLAYTOP_MODULE] =
    {
        RESETINDEX_LIST( DISPLAYTOP, i_Top_nRST ),
    };
    return (U32)ResetPinNumber[0];
}

// Primary MUX Control ! -
// 0 : Primary MLC, 1 : Primary MPU,
// 2 : Secondary MLC, 3 : ResConv(LCDIF)

void	NX_DISPLAYTOP_SetPrimaryMUX( U32 SEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->TFTMPU_MUX, (U32)SEL);
}

//HDMI Sync Set.
void	NX_DISPLAYTOP_HDMI_SetVSyncStart( U32 SEL ) // from posedge VSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL0, (U32)SEL);
}

void	NX_DISPLAYTOP_HDMI_SetVSyncHSStartEnd( U32 Start, U32 End ) // from posedge HSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL3, (U32)(End<<16) | (Start<<0) );
}


void	NX_DISPLAYTOP_HDMI_SetHActiveStart( U32 SEL ) // from posedge HSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL1, (U32)SEL);
}

void	NX_DISPLAYTOP_HDMI_SetHActiveEnd( U32 SEL ) // from posedge HSync
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
//	U32 regvalue;
	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	WriteIO32(&pRegister->HDMISYNCCTRL2, (U32)SEL);
}

void	NX_DISPLAYTOP_SetHDMIField
( 
	U32 Enable,			// Enable
	U32 InitVal,			// Init Value
	U32 VSyncToggle,		// VSync Toggle
	U32 HSyncToggle,		// HSync Toggle
	U32 VSyncClr,
	U32 HSyncClr,
	U32 FieldUse,
	U32 MUXSEL )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;
	regvalue =	((Enable & 0x01)<<0)
			|	((InitVal& 0x01)<<1)
			|	((VSyncToggle&0x3fff)<<2)
			|	((HSyncToggle&0x3fff)<<17);
	WriteIO32(&pRegister->HDMIFIELDCTRL, regvalue);

	regvalue = 	( (FieldUse&0x01)<<31 )
			|	( (MUXSEL&0x01)<<30	)
			|	( (HSyncClr)<<15 )
			|	( (VSyncClr)<<0 );
	WriteIO32(&pRegister->GREG0, regvalue);
}

//enum PrimPAD_MUX_Index{ // Primary TFT MUX
//	PADMUX_PrimaryMLC = 0,
//	PADMUX_PrimaryMPU = 1,
//	PADMUX_SecondaryMLC = 2,
//	PADMUX_ResolutionConv = 3,
//};
//enum PADCLK_Config {
//	PADCLK_CLK = 0,
//	PADCLK_InvCLK = 1,
//	PADCLK_ReservedCLK = 2,
//	PADCLK_ReservedInvCLK = 3,
//	PADCLK_CLK_div2_0   = 4,
//	PADCLK_CLK_div2_90  = 5,
//	PADCLK_CLK_div2_180 = 6,
//	PADCLK_CLK_div2_270 = 7,
//};

void	NX_DISPLAYTOP_SetPADClock( U32	MUX_Index,
								   U32	PADCLK_Cfg )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;

	regvalue = ReadIO32(&pRegister->GREG1);

	if( PADMUX_SecondaryMLC == MUX_Index ) { // Second
		regvalue = regvalue & (~(0x7 << 3));
		regvalue = regvalue | (PADCLK_Cfg<<3);
	} else if ( PADMUX_ResolutionConv == MUX_Index ) { // Resolution
		regvalue = regvalue & (~(0x7 << 6));
		regvalue = regvalue | (PADCLK_Cfg<<6);
	} else { // Primary
		regvalue = regvalue & (~(0x7 << 0));
		regvalue = regvalue | (PADCLK_Cfg<<0);
	}
	WriteIO32(&pRegister->GREG1, regvalue);
}

void	NX_DISPLAYTOP_SetLCDIF_i80Enb( CBOOL Enb )
{
	register struct NX_DISPLAYTOP_RegisterSet *pRegister;
	U32 regvalue;

	NX_ASSERT( CNULL != pRegister );
	pRegister = __g_ModuleVariables.pRegister;

	regvalue = ReadIO32(&pRegister->GREG1);
	regvalue = regvalue & (~(0x1 << 9));
	regvalue = regvalue | ((Enb&0x1)<<9);

	WriteIO32(&pRegister->GREG1, regvalue);
}
