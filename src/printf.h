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
 *      File            :
 *      Description     :
 *      Author          : Hans
 *      History         :
 */
#ifndef __K_PRINTF__
#define __K_PRINTF__

#include <stdarg.h>
#include "debug.h"
#define putchar DebugPutch

int printf(const char *format, ...);

#define UartPrintf printf
#endif
