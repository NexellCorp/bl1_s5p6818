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

#ifndef __NX_TYPE_H__
#define	__NX_TYPE_H__

//------------------------------------------------------------------------------
/// @name Basic data types
///
/// @brief Basic data type define and Data type constants are implemen \n
///	tation-dependent ranges of values allowed for integral data types. \n
/// The constants listed below give the ranges for the integral data types
//------------------------------------------------------------------------------
/// @{
///
typedef char				S8;				///< 8bit signed integer(s.7) value
typedef short				S16;			///< 16bit signed integer(s.15) value
typedef int					S32;			///< 32bit signed integer(s.31) value
typedef long				S64;			///< 64bit signed integer(s.63) value
typedef unsigned char		U8;				///< 8bit unsigned integer value
typedef unsigned short		U16;			///< 16bit unsigned integer value
typedef unsigned int		U32;			///< 32bit unsigned integer value
typedef unsigned long		U64;			///< 64bit unsigned integer value


#define S8_MIN			-128				///< signed char min value
#define S8_MAX			127					///< signed char max value
#define S16_MIN			-32768				///< signed short min value
#define S16_MAX			32767				///< signed short max value
#define S32_MIN			-2147483648			///< signed integer min value
#define S32_MAX			2147483647			///< signed integer max value
#define S64_MIN			-9223372036854775808	///< signed long min value
#define S64_MAX			9223372036854775807		///< signed long max value

#define U8_MIN			0					///< unsigned char min value
#define U8_MAX			255					///< unsigned char max value
#define U16_MIN			0					///< unsigned short min value
#define U16_MAX			65535				///< unsigned short max value
#define U32_MIN			0					///< unsigned integer min value
#define U32_MAX			4294967295			///< unsigned integer max value
#define U64_MIN			0						///< unsigned long min value
#define U64_MAX			18446744073709551615	///< unsigned long max value
/// @}

//==============================================================================
/// @name Boolean data type
///
/// C and C++ has difference boolean type. so we use signed integer type \n
/// instead of bool and you must use CTRUE or CFALSE macro for CBOOL type
//
//==============================================================================
/// @{
typedef S32	CBOOL;							///< boolean type is 32bits signed integer
#define CTRUE	1							///< true value is	integer one
#define CFALSE	0							///< false value is	integer zero
/// @}

//==============================================================================
/// @name NULL data type
///
///	CNULL is the null-pointer value used with many pointer operations \n
/// and functions. and this value is signed integer zero
//==============================================================================
/// @{
#define CNULL	0							///< null pointer is integer zero
/// @}

#if 0
//------------------------------------------------------------------------------
/// @name Basic type's size check
///
/// @brief this is compile time assert for check type's size. if exist don't intended size\n
/// of Basic type then it's occur compile type error
//------------------------------------------------------------------------------
/// @{
#ifndef __GNUC__
#define NX_CASSERT(expr) typedef char __NX_C_ASSERT__[(expr)?1:-1]
#else
#define NX_CASSERT_CONCAT_(a, b) a##b
#define NX_CASSERT_CONCAT(a, b) NX_CASSERT_CONCAT_(a, b)
#define NX_CASSERT(expr) typedef char NX_CASSERT_CONCAT(__NX_C_ASSERT__,__LINE__)[(expr)?1:-1]
#endif

NX_CASSERT	(sizeof(S8)		== 1);
NX_CASSERT	(sizeof(S16)	== 2);
NX_CASSERT	(sizeof(S32)	== 4);
NX_CASSERT	(sizeof(S64)	== 8);
NX_CASSERT	(sizeof(U8)		== 1);
NX_CASSERT	(sizeof(U16)	== 2);
NX_CASSERT	(sizeof(U32)	== 4);
NX_CASSERT	(sizeof(U64)	== 8);
NX_CASSERT	(sizeof(CBOOL)	== 4);

#else
#define NX_CASSERT(x)
#endif
/// @}

#ifdef aarch32
#define MPTRS	U32
#endif

#ifdef aarch64
#define MPTRS	U64
#endif

#endif	// __NX_TYPE_H__
