/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : Secure Manager
 *      File            : securemanager.h
 *      Description     : 
 *      Author          : Firmware Team
 *      History         : 2015.06.10 Hans Create
 */


#include "sysHeader.h"

struct NX_TZPC_Test {
	U8	module_index;
	U8	port_index;
	U8	set_bit;
	U8	target_type;
	U32	target_data;
	U32	target_addr;
};

