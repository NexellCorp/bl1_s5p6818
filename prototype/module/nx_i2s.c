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
#include "nx_chip.h"
#include "nx_dma.h"
#include "nx_i2s.h"

static	struct
{
    struct nx_i2s_registerset *pregister;

} __g_modulevariables[number_of_i2s_module] = { {cnull,}, };

//------------------------------------------------------------------------------
// module interface
//------------------------------------------------------------------------------
/**
 *	@brief	initialize of prototype enviroment & local variables.
 *	@return ctrue	indicates that initialize is successed.
 *			cfalse	indicates that initialize is failed.
 */
cbool	nx_i2s_initialize( void )
{
	static cbool binit = cfalse;
    u32 i;

	if( cfalse == binit )
	{
        for( i=0; i<number_of_i2s_module; i++ )
        {
            __g_modulevariables[i].pregister = cnull;
        }

		binit = ctrue;
	}

	return ctrue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		get number of modules in the chip.
 *	@return		module's number.
 */
u32		nx_i2s_getnumberofmodule( void )
{
	return number_of_i2s_module;
}

//------------------------------------------------------------------------------
// basic interface
//------------------------------------------------------------------------------
/**
 *	@brief		get module's physical address.
 *	@return		module's physical address
 */
u32		nx_i2s_getphysicaladdress( u32 moduleindex )
{
    static const u32 i2sphysicaladdr[number_of_i2s_module] =
    {
        phy_baseaddr_list( i2s )
    };

    nx_assert( number_of_i2s_module > moduleindex );

	return  (u32)i2sphysicaladdr[moduleindex];
}

//------------------------------------------------------------------------------
/**
 *	@brief		get a size, in byte, of register set.
 *	@return		size of module's register set.
 */
u32		nx_i2s_getsizeofregisterset( void )
{
	return sizeof( struct nx_i2s_registerset );
}

//------------------------------------------------------------------------------
/**
 *	@brief		set a base address of register set.
 *	@param[in]	baseaddress module's base address
 *	@return		none.
 */

void	nx_i2s_setbaseaddress( u32 moduleindex, void* baseaddress )
{
	nx_assert( cnull != baseaddress );
    nx_assert( number_of_i2s_module > moduleindex );

	__g_modulevariables[moduleindex].pregister = (struct nx_i2s_registerset *)baseaddress;
}

//------------------------------------------------------------------------------
/**
 *	@brief		get a base address of register set
 *	@return		module's base address.
 */

void*	nx_i2s_getbaseaddress( u32 moduleindex )
{
    nx_assert( number_of_i2s_module > moduleindex );


	return (void*)__g_modulevariables[moduleindex].pregister;
}

//------------------------------------------------------------------------------
/**
 *	@brief		initialize selected modules with default value.
 *	@return		ctrue	indicates that initialize is successed. 
 *				cfalse	indicates that initialize is failed.
 */
cbool	nx_i2s_openmodule( u32 moduleindex )
{
    nx_assert( number_of_i2s_module > moduleindex );

	return ctrue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		deinitialize selected module to the proper stage.
 *	@return		ctrue	indicates that deinitialize is successed.
 *				cfalse	indicates that deinitialize is failed.
 */
cbool	nx_i2s_closemodule( u32 moduleindex )
{
    nx_assert( number_of_i2s_module > moduleindex );

	return ctrue;
}

//------------------------------------------------------------------------------
/**
 *	@brief		indicates whether the selected modules is busy or not.
 *	@return		ctrue	indicates that module is busy. 
 *				cfalse	indicates that module is not busy.
 */
cbool	nx_i2s_checkbusy( u32 moduleindex )
{
    nx_assert( number_of_i2s_module > moduleindex );

	return cfalse;
}

//------------------------------------------------------------------------------
/**
 *	@brief		indicaes whether the selected modules is ready to enter power-down stage
 *	@return		ctrue	indicates that ready to enter power-down stage. \r\n
 *				cfalse	indicates that this module can't enter to power-down stage.
 */
cbool	nx_i2s_canpowerdown( u32 moduleindex )
{
    nx_assert( number_of_i2s_module > moduleindex );

	return ctrue;
}

//------------------------------------------------------------------------------
// dma interface
//------------------------------------------------------------------------------
/**
 *	@brief	get dma peripheral index of i2s controller's dma number
 *	@return dma peripheral index of i2s controller's dma number
 */
u32		nx_i2s_getdmaindex_pcmin( u32 moduleindex )
{
    const u32 i2sdmaindex[number_of_i2s_module] =
    {
        dmaindex_with_channel_list(i2s, i2srxdma)    
    };

    nx_assert( number_of_i2s_module > moduleindex );

    return  i2sdmaindex[moduleindex];
}

/**
 *	@brief	get dma peripheral index of i2s controller's dma number
 *	@return dma peripheral index of i2s controller's dma number
 */
u32		nx_i2s_getdmaindex_pcmout( u32 moduleindex )
{
    const u32 i2sdmaindex[number_of_i2s_module] =
    {
        dmaindex_with_channel_list(i2s, i2stxdma)
    };

    nx_assert( number_of_i2s_module > moduleindex );

    return  i2sdmaindex[moduleindex];
}

/**
 *	@brief		get bus width of i2s controller
 *	@return		dma bus width of i2s controller.
 */
u32		nx_i2s_getdmabuswidth( u32 moduleindex )
{
    nx_assert( number_of_i2s_module > moduleindex );
	return 32;
}

//------------------------------------------------------------------------------
// clock control interface
//------------------------------------------------------------------------------

/**
 *	@brief		get current clock number
 *	@return		current clock index
 */
u32		nx_i2s_getclocknumber( u32 moduleindex )
{
    static const u32 clkgen_i2slist[] =
    {
        clockindex_list( i2s )
    };

	nx_assert( number_of_i2s_module > moduleindex );

	return (u32)clkgen_i2slist[moduleindex];
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get current reset number
 *  @param[in]	moduleindex		a index of module.
 *  @return		current reset number
 *  @remarks
 */
u32     nx_i2s_getresetnumber( u32 moduleindex )
{
    const u32 resetportnumber[number_of_i2s_module] =
    {
        resetindex_list( i2s, presetn )
    };

    nx_assert( number_of_i2s_module > moduleindex );

    return (u32)resetportnumber[moduleindex];
}


//------------------------------------------------------------------------------
/// @  name   i2s interface
/// @{

// the dma-related functions. 
//------------------------------------------------------------------------------
/**
 *  @brief      	set dma tx pause enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	enable	ctrue	indicate that dma tx pause enable
 *  						cfalse	indicate that dma tx pause disable
 *  @return		none.
 */
void    nx_i2s_txdmapauseenable( u32 moduleindex, cbool enable )
{
    const u32 tdp_pos   = 6;
    const u32 tdp_mask  = 1ul<<tdp_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~tdp_mask;
    regvalue     |= enable<<tdp_pos;

    writeio32(&pregister->con, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set dma rx pause enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	enable	ctrue	indicate that dma rx pause enable
 *  						cfalse	indicate that dma rx pause disable
 */
void    nx_i2s_rxdmapauseenable( u32 moduleindex, cbool enable )
{
    const u32 rdp_pos   = 5;
    const u32 rdp_mask  = 1ul<<rdp_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~rdp_mask;
    regvalue     |= enable<<rdp_pos;

    writeio32(&pregister->con, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set i2s tx pause enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	enable	ctrue	indicate that i2s tx pause enable
 *  						cfalse	indicate that i2s tx pause disable
 *  @return		none.
 */
void    nx_i2s_txchpauseenable( u32 moduleindex, cbool enable )
{
    const u32 tcp_pos   = 4;
    const u32 tcp_mask  = 1ul<<tcp_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~tcp_mask;
    regvalue     |= enable<<tcp_pos;

    writeio32(&pregister->con, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set i2s rx pause enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @parma[in]	enable	ctrue	indicate that i2s rx pause enable
 *  						cfalse	indicate that i2s rx pause disable
 *  @reutn		none.
 */
void    nx_i2s_rxchpauseenable( u32 moduleindex, cbool enable )
{
    const u32 rcp_pos   = 3;
    const u32 rcp_mask  = 1ul<<rcp_pos;

    register struct nx_i2s_registerset* pregister;
    register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

    pregister   = __g_modulevariables[moduleindex].pregister;

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~rcp_mask;
    regvalue     |= enable<<rcp_pos;

    writeio32(&pregister->con, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set dma tx enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	enable	ctrue	indicate that dma tx enable
 *  						cfalse	indicate that dma tx disable
 *  @return		none.
 */
void    nx_i2s_txdmaenable( u32 moduleindex, cbool enable )
{
    const u32 txd_pos   = 2;
    const u32 txd_mask  = 1ul<<txd_pos;

    register struct nx_i2s_registerset* pregister;
    register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

    pregister   = __g_modulevariables[moduleindex].pregister;

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~txd_mask;
    regvalue     |= enable<<txd_pos;

    writeio32(&pregister->con, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set dma rx enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	enable	ctrue	indicate that dma rx enable
 *  						cfalse	indicate that dma rx disable
 *  @return		none.
 */
void    nx_i2s_rxdmaenable( u32 moduleindex, cbool enable )
{
    const u32 rxd_pos   = 1;
    const u32 rxd_mask  = 1ul<<rxd_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~rxd_mask;
    regvalue     |= enable<<rxd_pos;

    writeio32(&pregister->con, regvalue);
}

// the fifo-related functions. 
//------------------------------------------------------------------------------
/**
 *  @brief      	get c
 *  @param[in]	moduleindex		a index of module.
 *  @return		current reset number
 */
cbool   nx_i2s_getchannelclockindication( u32 moduleindex )
{
    const u32 lri_pos   = 11;
    const u32 lri_mask  = 1ul << lri_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (cbool)((readio32(&pregister->con) & lri_mask)>>lri_pos);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get tx fifo is empty or not
 *  @param[in]	moduleindex		a index of module.
 *  @return		ctrue			indicate that fifo empty
 *  				cfalse			indicate that fifo not empty
 */
cbool   nx_i2s_gettxfifoempty( u32 moduleindex )
{
    const u32 tfe_pos   = 10;
    const u32 tfe_mask  = 1ul<<tfe_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (cbool)((readio32(&pregister->con) & tfe_mask)>>tfe_pos);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get rx fifo is empty or not
 *  @param[in]	moduleindex		a index of module.
 *  @return		ctrue			indicate that fifo empty
 *  				cfalse			indicate that fifo not empty
 */

cbool   nx_i2s_getrxfifoempty( u32 moduleindex )
{
    const u32 rfe_pos   = 9;
    const u32 rfe_mask  = 1ul<<rfe_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (cbool)((readio32(&pregister->con) & rfe_mask)>>rfe_pos);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get tx fifo is full or not
 *  @param[in]	moduleindex		a index of module.
 *  @return		ctrue			indicate that fifo full
 *  				cfalse			indicate that fifo not full
 */
cbool   nx_i2s_gettxfifofull( u32 moduleindex )
{
    const u32 tff_pos   = 8;
    const u32 tff_mask  = 1ul<<tff_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );


    return (cbool)((readio32(&pregister->con) & tff_mask)>>tff_pos);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get rx fifo is full or not
 *  @param[in]	moduleindex		a index of module.
 *  @return		ctrue			indicate that fifo full
 *  				cfalse			indicate that fifo not full
 */
cbool   nx_i2s_getrxfifofull( u32 moduleindex )
{
    const u32 rff_pos   = 7;
    const u32 rff_mask  = 1ul<<rff_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (cbool)((readio32(&pregister->con) & rff_mask)>>rff_pos);
}

//------------------------------------------------------------------------------
/**
  *  @brief		set transmit fifo flush.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	enable	ctrue	indicate that tx fifo flush.
  						cfalse indicate that tx fifo no flush.
  *	@return		none.
  */
void    nx_i2s_txfifoflushenable( u32 moduleindex, cbool enable )
{
    const u32 tfl_pos   = 15;
    const u32 tfl_mask  = 1ul<<tfl_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );
	
    regvalue      = readio32(&pregister->fic);
    regvalue     &= ~tfl_mask;
    regvalue     |= enable<<tfl_pos;

    writeio32(&pregister->fic, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief		get transmit fifo data count.
  *  @param[in]  moduleindex		a index of module.
  *  @return	   	tx fifo count	( range : 0 ~ 64 depth )
  */
u32     nx_i2s_gettxfifodatacount( u32 moduleindex )
{
    const u32 tfc_pos   = 8;
    const u32 tfc_mask  = 0x7f<<tfc_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (u32)( (readio32(&pregister->fic) & tfc_mask)>>tfc_pos );
}

//------------------------------------------------------------------------------
/**
  *  @brief		set receive fifo flush.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	enable	ctrue	indicate that rx fifo flush.
  						cfalse indicate that rx fifo no flush.
  *	@return		none.
  */
void    nx_i2s_rxfifoflushenable( u32 moduleindex, cbool enable )
{
    const u32 rfl_pos   = 7;
    const u32 rfl_mask  = 1ul<<rfl_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->fic);
    regvalue     &= ~rfl_mask;
    regvalue     |= enable<<rfl_pos;

    writeio32(&pregister->fic, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief		get receive fifo data count.
  *  @param[in]  moduleindex		a index of module.
  *  @return	   	rx fifo count	( range : 0 ~ 64 depth )
  */
u32     nx_i2s_getrxfifodatacount( u32 moduleindex )
{
    const u32 rfc_pos   = 0;
    const u32 rfc_mask  = 0x7f<<rfc_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (u32)( (readio32(&pregister->fic) & rfc_mask)>>rfc_pos );
}

// the mode-related functions. 
//------------------------------------------------------------------------------
/**
 *  @brief      	set i2s enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	enable	ctrue	indicate that i2s enable
 *  						cfalse	indicate that i2s disable
 *  @return		none.
 */
void    nx_i2s_seti2senable( u32 moduleindex, cbool enable )
{
    const u32 act_pos   = 0;
    const u32 act_mask  = 1ul<<act_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->con);
    regvalue     &= ~act_mask;
    regvalue     |= enable<<act_pos;

    writeio32(&pregister->con, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get i2s enable or disable
 *  @param[in]	moduleindex		a index of module.
 *  @return		ctrue			indicate that i2s enable
 *  				cfalse			indicate that i2s disable
 */
cbool    nx_i2s_geti2senable( u32 moduleindex )
{
    const u32 act_pos   = 0;
    const u32 act_mask  = 1ul<<act_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    pregister   = __g_modulevariables[moduleindex].pregister;

    return (cbool)(readio32(&pregister->con) & act_mask) >> act_pos ;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set codec clock source select.
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	disable			(0: internal codec clock(pclk), 1: cdclk )
 *  @return		none.
 */
void    nx_i2s_setcodecclockdisable( u32 moduleindex, cbool disable )
{
    const u32 cce_pos   = 12;
    const u32 cce_mask  = 1ul<<cce_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~cce_mask;
    regvalue     |= disable<<cce_pos;

    writeio32(&pregister->mod, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get codec clock source select.
 *  @param[in]	moduleindex		a index of module.
 *  @return		bitlength		channel bit length (0:16bit, 1:8bit, 2:24bit)
 */
cbool    nx_i2s_getcodecclockdisable( u32 moduleindex )
{
    const u32 cce_pos   = 12;
    const u32 cce_mask  = 1ul<<cce_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

	return (cbool)(readio32(&pregister->mod) & cce_mask) >> cce_pos;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set iis master or slave mode select.
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	mode			iis ms mode (0: master mode, 1:slave mode)			
 *  @return		none.
 */
void    nx_i2s_setmasterslavemode( u32 moduleindex, nx_i2s_ims mode )
{
    const u32 ims_pos   = 11;
    const u32 ims_mask  = 1ul<<ims_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~ims_mask;
    regvalue     |= mode<<ims_pos;

    writeio32(&pregister->mod, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get iis master or slave mode select.
 *  @param[in]	moduleindex		a index of module.
 *  @return		mode			iis ms mode (0: master mode, 1:slave mode)			
 */
nx_i2s_ims	nx_i2s_getmasterslavemode( u32 moduleindex )
{
    const u32 ims_pos   = 11;
    const u32 ims_mask  = 1ul<<ims_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (nx_i2s_ims)(readio32(&pregister->mod) & ims_mask) >> ims_pos;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set transmit or receive mode select.
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	mode			iis txr mode (0: tx only 1:rx only, 2: tx & rx)			
 *  @return		none.
 */
void    nx_i2s_settxrxmode( u32 moduleindex, nx_i2s_txr mode )
{
    const u32 txr_pos   = 8;
    const u32 txr_mask  = 3ul<<txr_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = pregister->mod;
    regvalue     &= ~txr_mask;
    regvalue     |= mode<<txr_pos;

    writeio32(&pregister->mod, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief 		 get transmit or receive mode select.
  *  @param[in]  moduleindex		a index of module.
  *  @return	 	mode		 	iis txr mode (0: tx only 1:rx only, 2: tx & rx)			 
  */
 nx_i2s_txr   	nx_i2s_gettxrxmode( u32 moduleindex )
{
    const u32 txr_pos   = 8;
    const u32 txr_mask  = 3ul<<txr_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );;

    return (nx_i2s_txr)(readio32(&pregister->mod) & txr_mask) >> txr_pos;
}

//------------------------------------------------------------------------------
/**
  *  @brief		set lr clock polarity select.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	polarity			lr clock polarity (0:left low/right high, 1:left high/right low)
  *	@return		none.
  */
void    nx_i2s_setlrclockpolarity( u32 moduleindex, nx_i2s_lrp polarity )
{
    const u32 lrp_pos   = 7;
    const u32 lrp_mask  = 1ul<<lrp_pos;

    register struct nx_i2s_registerset* pregister;
    register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

    pregister   = __g_modulevariables[moduleindex].pregister;

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~lrp_mask;
    regvalue     |= polarity<<lrp_pos;

    writeio32(&pregister->mod, regvalue);
}

 //------------------------------------------------------------------------------
 /**
   *  @brief	 	set lr clock polarity select.
   *  @param[in]	moduleindex		 a index of module.
   *  @return		polarity			 lr clock polarity (0:left low/right high, 1:left high/right low)
   */
nx_i2s_lrp	nx_i2s_getlrclockpolarity( u32 moduleindex )
{
    const u32 lrp_pos   = 7;
    const u32 lrp_mask  = 1ul<<lrp_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );
	
    return (nx_i2s_lrp)(readio32(&pregister->mod) & lrp_mask) >> lrp_pos;
}

//------------------------------------------------------------------------------
/**
 *  @brief      	set transmission 8/16/24 bits per audio channel. 
 *  @param[in]	moduleindex		a index of module.
 *  @param[in]	bitlength		channel bit length (0:16bit, 1:8bit, 2:24bit)
 *  @return		none.
 */
void    nx_i2s_setbitlengthcontrol( u32 moduleindex, nx_i2s_blc bitlength )
{
    const u32 blc_pos   = 13;
    const u32 blc_mask  = 3ul<<blc_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~blc_mask;
    regvalue     |= bitlength<<blc_pos;

    writeio32(&pregister->mod, regvalue);
}

//------------------------------------------------------------------------------
/**
 *  @brief      	get transmission 8/16/24 bits per audio channel. 
 *  @param[in]	moduleindex		a index of module.
 *  @return		bitlength		channel bit length (0:16bit, 1:8bit, 2:24bit)
 */
nx_i2s_blc	nx_i2s_getbitlengthcontrol( u32 moduleindex )
{
    const u32 blc_pos   = 13;
    const u32 blc_mask  = 3ul<<blc_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    return (nx_i2s_blc)(readio32(&pregister->mod) & blc_mask) >> blc_pos;
}


//------------------------------------------------------------------------------
/**
  *  @brief		set serial data format select.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	dataformat		serial data format (0:i2s, 1:msb, 2:lsb)
  *	@return		none.
  */
void    nx_i2s_setserialdataformat( u32 moduleindex, nx_i2s_sdf dataformat )
{
    const u32 sdf_pos   = 5;
    const u32 sdf_mask  = 3ul<<sdf_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~sdf_mask;
    regvalue     |= dataformat<<sdf_pos;

    writeio32(&pregister->mod, regvalue);
}

//------------------------------------------------------------------------------
/**
  *  @brief	 	get serial data format select.
  *  @param[in]  moduleindex		a index of module.
  *  @return		dataformat 	 	serial data format (0:i2s, 1:msb, 2:lsb)
  */
nx_i2s_sdf	nx_i2s_getserialdataformat( u32 moduleindex )
{
    const u32 sdf_pos   = 5;
    const u32 sdf_mask  = 3ul<<sdf_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );
	
    return (nx_i2s_sdf)(readio32(&pregister->mod) & sdf_mask) >> sdf_pos;
}

// the sampling clock-related functions. 
//------------------------------------------------------------------------------
/**
  *  @brief		set root clock frequency select.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	rootclock		root clock sampling (0:256fs, 1:512fs, 2:394fs, 3:768fs)
  *	@return		none.
  */
void    nx_i2s_setrootclockfrequency( u32 moduleindex, nx_i2s_rootclock rootclock )
{
    const u32 rfs_pos   = 3;
    const u32 rfs_mask  = 3ul<<rfs_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~rfs_mask;
    regvalue     |= rootclock<<rfs_pos;

    writeio32(&pregister->mod, regvalue);
}

 //------------------------------------------------------------------------------
 /**
   *  @brief	 	get root clock frequency select.
   *  @param[in]	moduleindex		 a index of module.
   *  @return	 	rootclock		 root clock sampling (0:256fs, 1:512fs, 2:394fs, 3:768fs)
   */
nx_i2s_rootclock   nx_i2s_getrootclockfrequency( u32 moduleindex )
{
    const u32 rfs_pos   = 3;
    const u32 rfs_mask  = 3ul<<rfs_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

	return (nx_i2s_rootclock)(readio32(&pregister->mod) & rfs_mask) >> rfs_pos;
}

//------------------------------------------------------------------------------
/**
  *  @brief		set bit clock frequency select.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	bitclock			bit clock sampling (0:32fs, 1:48fs, 2:16fs, 3:24fs)
  *	@return		none.
  */
void    nx_i2s_setbitclockfrequency( u32 moduleindex, nx_i2s_bitclock bitclock )
{
    const u32 bfs_pos   = 1;
    const u32 bfs_mask  = 3ul<<bfs_pos;

	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~bfs_mask;
    regvalue     |= bitclock<<bfs_pos;

    writeio32(&pregister->mod, regvalue);
}

 //------------------------------------------------------------------------------
 /**
   *  @brief	 	get bit clock frequency select.
   *  @param[in]	moduleindex		 a index of module.
   *  @return	 	bitclock			bit clock sampling (0:32fs, 1:48fs, 2:16fs, 3:24fs)
   */
 nx_i2s_bitclock   nx_i2s_getbitclockfrequency( u32 moduleindex )
{
    const u32 bfs_pos   = 1;
    const u32 bfs_mask  = 3ul<<bfs_pos;

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

	return	(nx_i2s_bitclock)(readio32(&pregister->mod) & bfs_mask) >> bfs_pos;
}

// the tramit/receive-related functions. 
//------------------------------------------------------------------------------
/**
  *  @brief		set transmit data.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	channelindex	left / right ( 0:left, 1:right )
  *  @param[in]	bitlength		channel bit length ( 0:16bit, 1:8bit, 2:24bit)  
  *  @param[in]	txdata			set transmission data.  
  *	@return		none.
  */
void    nx_i2s_settxdata( u32 moduleindex, nx_i2s_ch channelindex, nx_i2s_bitlength bitlength, u32 txdata )
{
	const u32 blc_pos 	= 13;
	const u32 blc_mask	= 1ul << blc_pos;
	
	register struct nx_i2s_registerset* pregister;
	register u32 regvalue;

    channelindex = channelindex;
    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );
	
    nx_assert( bitlength==nx_i2s_bitlength_24 || bitlength==nx_i2s_bitlength_16 
		    || bitlength==nx_i2s_bitlength_8 );

    regvalue      = readio32(&pregister->mod);
    regvalue     &= ~blc_mask;
    regvalue     |= bitlength << blc_pos;

	writeio32(&pregister->mod, regvalue);
#if 0
   	u32 tdata			= 0;
	if( bitlength == nx_i2s_bitlength_8 )
		tdata	= (txdata & 0xff);
	else if( bitlength == nx_i2s_bitlength_16 )
		tdata	= (txdata & 0xffff);

	if( channelindex == nx_i2s_ch_right )
		tdata <<= 0;
	if( channelindex == nx_i2s_ch_left )
		tdata <<= 16; 

	writeio32(&pregister->txd, tdata);
#else
	writeio32(&pregister->txd, txdata);
#endif
}

//------------------------------------------------------------------------------
/**
  *  @brief		get receive data.
  *  @param[in]  moduleindex		a index of module.
  *  @param[in]	channelindex	left / right ( 0:left, 1:right )
  *  @param[in]	bitlength		channel bit length ( 0:16bit, 1:8bit, 2:24bit)  
  *	@return		get rx fifo data.
  */
u32     nx_i2s_getrxdata( u32 moduleindex, nx_i2s_ch channelindex, nx_i2s_bitlength bitlength )
{
    const u32 rxd_pos[]   = {0, 16};
    const u32 rxd_mask[]  = {0xffff, 0xff};

    u32 rxdatapos   = rxd_pos[channelindex];
    u32 rxdatamask  = rxd_mask[bitlength];

	register struct nx_i2s_registerset* pregister;

    nx_assert( number_of_i2s_module > moduleindex );

	pregister = __g_modulevariables[moduleindex].pregister;
	nx_assert( cnull != pregister );

    channelindex = channelindex;
    bitlength = bitlength;    
    rxdatamask = rxdatamask;
    rxdatapos  = rxdatapos;
    
    return (u32)(readio32(&pregister->rxd));
	//return ((readio32(&pregister->rxd)) & rxdatamask);
}

