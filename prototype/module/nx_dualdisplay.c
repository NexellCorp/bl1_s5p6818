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
#include "nx_dualdisplay.h"


// DualDisplay Prototype Must be 1 MODULE
NX_CASSERT( NUMBER_OF_DUALDISPLAY_MODULE == 1 );


//------------------------------------------------------------------------------
//
//	DUALDISPLAY Interface
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Basic Interface
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 *	@brief		Get module's reset index.
 *	@return		Module's reset index.\n
 *				It is equal to RESETINDEX_OF_DUALDISPLAY?_MODULE_i_nRST in <nx_chip.h>.
 *	@see		NX_RSTCON_Enter,
 *				NX_RSTCON_Leave,
 *				NX_RSTCON_GetStatus
 */
U32 NX_DUALDISPLAY_GetResetNumber ( U32 ModuleIndex )
{
	const U32 ResetNumber[] =
	{
		RESETINDEX_LIST( DUALDISPLAY, i_nRST )
	};
	NX_CASSERT( NUMBER_OF_DUALDISPLAY_MODULE == (sizeof(ResetNumber)/sizeof(ResetNumber[0])) );
	NX_ASSERT( NUMBER_OF_DUALDISPLAY_MODULE > ModuleIndex );
	return	ResetNumber[ModuleIndex];
}

