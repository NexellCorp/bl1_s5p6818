 # Copyright (C) 2016  Nexell Co., Ltd.
 # Author: Sangjong, Han <hans@nexell.co.kr>
 #
 # This program is free software; you can redistribute it and/or
 # modify it under the terms of the GNU General Public License
 #
 # as published by the Free Software Foundation; either version 2
 # of the License, or (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Build Version info
VERINFO				= V110

#########################################################################
# build environment
#########################################################################
CFLAGS				:=
DEBUG				?= n

# chip name
CHIPNAME			= S5P6818

# archtecture type
#OPMODE				?= aarch64
OPMODE				?= aarch32

# (ddr3/lpddr3) memory type
MEMTYPE				?= DDR3
#MEMTYPE			?= LPDDR3
MEMTEST				?= n

# power management ic(pmic) on/off
#INITPMIC			?= YES
INITPMIC			?= NO

CRC_CHECK			?= n

# supported (thridboot) boot mode
SUPPORT_USB_BOOT		?= y
SUPPORT_SDMMC_BOOT		?= y
SUPPORT_SPI_BOOT		?= n
SUPPORT_NAND_BOOT		?= n
SUPPORT_SDFS_BOOT		?= n

#BOARD				?= SVT
#BOARD				?= ASB
#BOARD				?= DRONE
BOARD				?= AVN
#BOARD				?= BF700
#BOARD				?= RAPTOR

# supported kernel version (3.18-3.4/4.1-4.4)
KERNEL_VER			?= 3
#KERNEL_VER			?= 4

# system log message
SYSLOG				?= y

# secure Boot
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
endif
endif

# Top Names
PROJECT_NAME			= $(CHIPNAME)_2ndboot_$(OPMODE)_$(MEMTYPE)_$(VERINFO)
TARGET_NAME			= bl1-$(shell echo $(BOARD) | tr A-Z a-z)
LDS_NAME			= peridot_2ndboot_$(OPMODE)

# Directories
DIR_PROJECT_TOP			=

DIR_OBJOUTPUT			= obj
DIR_TARGETOUTPUT		= out

CODE_MAIN_INCLUDE		=

# Build Environment
ifeq ($(OPMODE) , aarch32)
ARCH				= armv7-a
CPU				= cortex-a15
endif
ifeq ($(OPMODE) , aarch64)
ARCH				= armv8-a
CPU				= cortex-a53+crc
endif

CC				= $(CROSS_TOOL)gcc
LD 				= $(CROSS_TOOL)ld
AS 				= $(CROSS_TOOL)as
AR 				= $(CROSS_TOOL)ar
MAKEBIN				= $(CROSS_TOOL)objcopy
OBJCOPY				= $(CROSS_TOOL)objcopy
RANLIB 				= $(CROSS_TOOL)ranlib

GCC_LIB				= $(shell $(CC) -print-libgcc-file-name)

ifeq ($(DEBUG), y)
CFLAGS				= -DNX_DEBUG -O0
Q				=
else
CFLAGS				= -DNX_RELEASE -Os
Q				= @
endif

# MISC tools for MS-DOS
ifeq ($(OS),Windows_NT)
MKDIR				= mkdir
RM				= del /q /F
MV				= move
CD				= cd
CP				= copy
ECHO				= echo
RMDIR				= rmdir /S /Q
else
MKDIR				= mkdir
RM				= rm -f
MV				= mv
CD				= cd
CP				= cp
ECHO				= echo
RMDIR				= rm -rf
endif

#########################################################################
# flags variables
#########################################################################
# FLAGS
ARFLAGS				= rcs
ARFLAGS_REMOVE			= -d
ARLIBFLAGS			= -v -s

ASFLAG				= -D__ASSEMBLY__ -D$(OPMODE)

CFLAGS				+=	-g -Wall				\
					-Wextra -ffreestanding -fno-builtin	\
					-mlittle-endian				\
					-mcpu=$(CPU)				\
					$(CODE_MAIN_INCLUDE)			\
					-D__arm -DLOAD_FROM_$(BOOTFROM)		\
					-DMEMTYPE_$(MEMTYPE)			\
					-DINITPMIC_$(INITPMIC)			\
					-D$(OPMODE) -D$(BOARD)
ifeq ($(OPMODE) , aarch32)
CFLAGS				+=	-msoft-float				\
					-mstructure-size-boundary=32
endif

ifeq ($(OPMODE) , aarch64)
ASFLAG				+=	-march=$(ARCH) -mcpu=$(CPU)

CFLAGS				+=	-mcmodel=small				\
					-march=$(ARCH)
endif

# secure boot - (thirdboot) decrypt
ifeq ($(SECURE_ON), 1)
CFLAGS				+=	-DSECURE_ON
endif

# supported kernel version (3.18-4.3/4.1-4.4)
ifeq ($(KERNEL_VER), 3)
CFLAGS				+=	-DKERNEL_VER_3_4
endif

# system log messgae
ifeq ($(SYSLOG), y)
CFLAGS				+=	-DSYSLOG_ON
endif

# power managemnt ic(pmic) on/off
ifeq ($(INITPMIC), YES)
CFLAGS				+=	-D$(BOARD)_PMIC_INIT
endif

# memory test
ifeq ($(MEMTEST), y)
MEMTEST_TYPE			+=	SIMPLE
CFLAGS				+=	-D$(MEMTEST_TYPE)_MEMTEST
endif

# supported crc check
ifeq ($(CRC_CHECK), y)
CHECKSUM			+=	CRC_CHECK
CFLAGS				+=	-D$(CHECKSUM)_ON
endif
