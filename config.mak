 #                                                                              
 #      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 #      Nexell Co. Proprietary & Confidential                                   
 #                                                                              
 #      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 #      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 #      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 #      FOR A PARTICULAR PURPOSE.                                               
 #                                                                              
 #      Moudle          : Base                                                      
 #      File            : config.mak
 #      Description     :                                                       
 #      Author          : Firware Team                                          
 #      History         :                                                       
 #
###########################################################################
# Build Version info
###########################################################################
VERINFO				= V110

###########################################################################
# Build Environment
###########################################################################
DEBUG				= n

#OPMODE				= aarch64
OPMODE				= aarch32

MEMTYPE				= DDR3
#MEMTYPE			= LPDDR3
MEMTEST				= n

INITPMIC			= YES
#INITPMIC			= NO

CRC_CHECK			= n

CHIPNAME			= S5P6818

CFLAGS				:=

SUPPORT_USB_BOOT		= y
SUPPORT_SPI_BOOT		= n
SUPPORT_SDMMC_BOOT		= y
SUPPORT_SDFS_BOOT		= n
SUPPORT_NAND_BOOT		= n
SUPPORT_UART_BOOT		= n

#BOARD				= SVT
#BOARD				= ASB
#BOARD				= DRONE
#BOARD				= AVN
#BOARD				= BF700
BOARD				?= RAPTOR

# secure boot
SECURE_ON			?= 0

# cross-tool pre-header
ifeq ($(OPMODE), aarch32)
ifeq ($(OS),Windows_NT)
CROSS_TOOL_TOP			=
CROSS_TOOL			= $(CROSS_TOOL_TOP)arm-none-eabi-
else
CROSS_TOOL_TOP			=
CROSS_TOOL			= $(CROSS_TOOL_TOP)arm-eabi-
endif
endif

ifeq ($(OPMODE), aarch64)
ifeq ($(OS),Windows_NT)
CROSS_TOOL_TOP			=
CROSS_TOOL			= $(CROSS_TOOL_TOP)aarch64-none-elf-
else
CROSS_TOOL_TOP			=
CROSS_TOOL			= $(CROSS_TOOL_TOP)aarch64-none-elf-
#CROSS_TOOL			= $(CROSS_TOOL_TOP)aarch64-elf-
endif
endif

###########################################################################
# Top Names
###########################################################################
PROJECT_NAME		= $(CHIPNAME)_2ndboot_$(OPMODE)_$(MEMTYPE)_$(VERINFO)

TARGET_NAME			= bl1-$(shell echo $(BOARD) | tr A-Z a-z)

LDS_NAME			= peridot_2ndboot_$(OPMODE)


###########################################################################
# Directories
###########################################################################
DIR_PROJECT_TOP			=

DIR_OBJOUTPUT			= obj
DIR_TARGETOUTPUT		= out

CODE_MAIN_INCLUDE		=

###########################################################################
# Build Environment
###########################################################################
ifeq ($(OPMODE) , aarch32)
ARCH			= armv7-a
CPU				= cortex-a15
endif
ifeq ($(OPMODE) , aarch64)
ARCH			= armv8-a
CPU				= cortex-a53+crc
endif

CC				= $(CROSS_TOOL)gcc
LD 				= $(CROSS_TOOL)ld
AS 				= $(CROSS_TOOL)as
AR 				= $(CROSS_TOOL)ar
MAKEBIN			= $(CROSS_TOOL)objcopy
OBJCOPY			= $(CROSS_TOOL)objcopy
RANLIB 			= $(CROSS_TOOL)ranlib

GCC_LIB			= $(shell $(CC) -print-libgcc-file-name)

ifeq ($(DEBUG), y)
CFLAGS			+= -DNX_DEBUG -Os
Q				=
else
CFLAGS			+= -DNX_RELEASE -Os
Q				= @
endif

###########################################################################
# MISC tools for MS-DOS
###########################################################################
ifeq ($(OS),Windows_NT)
MKDIR			= mkdir
RM				= del /q /F
MV				= move
CD				= cd
CP				= copy
ECHO			= echo
RMDIR			= rmdir /S /Q
else
MKDIR			= mkdir
RM				= rm -f
MV				= mv
CD				= cd
CP				= cp
ECHO				= echo
RMDIR				= rm -rf
endif
###########################################################################
# FLAGS
###########################################################################
ARFLAGS				= rcs
ARFLAGS_REMOVE			= -d
ARLIBFLAGS			= -v -s

ASFLAG				= -D__ASSEMBLY__ -D$(OPMODE)

CFLAGS				+=	-g -Wall						\
					-Wextra -ffreestanding -fno-builtin	\
					-mlittle-endian						\
					-mcpu=$(CPU)						\
					$(CODE_MAIN_INCLUDE)				\
					-D__arm						\
					-DMEMTYPE_$(MEMTYPE)				\
					-DINITPMIC_$(INITPMIC)				\
					-D$(OPMODE) -D$(BOARD)

ifeq ($(SECURE_ON), 1)
CFLAGS				+=	-DSECURE_ON
endif

ifeq ($(OPMODE) , aarch32)
CFLAGS				+=	-msoft-float					\
					-mstructure-size-boundary=32
endif

ifeq ($(OPMODE) , aarch64)
ASFLAG				+=	-march=$(ARCH) -mcpu=$(CPU)

CFLAGS				+=	-mcmodel=small					\
					-march=$(ARCH)
endif

ifeq ($(INITPMIC), YES)
CFLAGS				+=	-D$(BOARD)_PMIC_INIT
endif

ifeq ($(MEMTEST), y)
MEMTEST_TYPE		+=	SIMPLE
CFLAGS				+=	-D$(MEMTEST_TYPE)_MEMTEST
endif

ifeq ($(CRC_CHECK), y)
CHECKSUM			+=	CRC_CHECK
CFLAGS				+=	-D$(CHECKSUM)_ON
endif
