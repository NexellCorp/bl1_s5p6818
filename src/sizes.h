/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : 
 *      File            : 
 *      Description     : 
 *      Author          : Hans
 *      History         : 2013.01.10 First Implementation
 */
/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2010 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains some macro definitions for handling 32/64 bit platforms.
 *
 */

#define	rand()		(0xaaaa5555UL)

#define rand32() ((unsigned int) rand() | ( (unsigned int) rand() << 16))

#define rand_ul() rand32()
#define UL_ONEBITS 0xffffffffUL
#define UL_LEN 32UL
#define CHECKERBOARD1 0x55555555UL
#define CHECKERBOARD2 0xaaaaaaaaUL
#define UL_BYTE(x) ((x | x << 8 | x << 16 | x << 24))


