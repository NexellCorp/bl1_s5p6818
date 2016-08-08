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
#ifndef __NX_CLOCKCONTROL_H__
#define __NX_CLOCKCONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
///	@name	Clock Control types
//------------------------------------------------------------------------------
//@{

///	@brief	type for PCLK control mode
typedef enum
{
	NX_PCLKMODE_DYNAMIC = 0UL,		///< PCLK is provided only when CPU has access to registers of this module.
	NX_PCLKMODE_ALWAYS	= 1UL		///< PCLK is always provided for this module.
} NX_PCLKMODE ;

///	@brief type for BCLK control mode
typedef enum
{
	NX_BCLKMODE_DISABLE	= 0UL,		///< BCLK is disabled.
	NX_BCLKMODE_DYNAMIC	= 2UL,		///< BCLK is provided only when this module requests it.
	NX_BCLKMODE_ALWAYS	= 3UL		///< BCLK is always provided for this module.
} NX_BCLKMODE ;

//@}

#ifdef __cplusplus
}
#endif

#endif // __NX_CLOCKCONTROL_H__

