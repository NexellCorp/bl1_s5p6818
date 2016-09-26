//------------------------------------------------------------------------------
/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          :
 *      File	: debug.h
 *      Description     :
 *      Author          : Hans
 *      History         : 
 *			 
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <nx_type.h>

#define LOG_LEVEL			30

#define LOG_LEVEL_NONE			0
#define LOG_LEVEL_ERROR			10
#define LOG_LEVEL_NOTICE		20
#define LOG_LEVEL_WARNING		30
#define LOG_LEVEL_INFO			40
#define LOG_LEVEL_VERBOSE		50


#if LOG_LEVEL >= LOG_LEVEL_NOTICE
# define NOTICE(...)	printf("NOTICE:  " __VA_ARGS__)
#else
# define NOTICE(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
# define ERROR(...)	printf("ERROR:   " __VA_ARGS__)
#else
# define ERROR(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
# define WARN(...)	printf("WARNING: " __VA_ARGS__)
#else
# define WARN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
# define INFO(...)	printf("INFO:    " __VA_ARGS__)
#else
# define INFO(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define VERBOSE(...)	printf("VERBOSE: " __VA_ARGS__)
#else
# define VERBOSE(...)
#endif

CBOOL   DebugInit( U32 port );
void    DebugPutch( S8 ch );
//S8      DebugGetch( void );
CBOOL	DebugIsUartTxDone(void);

CBOOL	DebugIsTXEmpty(void);
CBOOL   DebugIsBusy( void );
S8	DebugGetch( void );

#endif // __DEBUG_H__
