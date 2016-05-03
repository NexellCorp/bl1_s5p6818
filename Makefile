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
 #      File            :                                                       
 #      Description     :                                                       
 #      Author          : Firware Team                                          
 #      History         :                                                       
 #
include config.mak

LDFLAGS		=	-Bstatic							\
			-Wl,-Map=$(DIR_TARGETOUTPUT)/$(TARGET_NAME).map,--cref		\
			-T$(LDS_NAME).lds						\
			-Wl,--start-group						\
			-Lsrc/$(DIR_OBJOUTPUT)						\
			-Wl,--end-group							\
			-Wl,--build-id=none						\
			-nostdlib

SYS_OBJS	=	startup_$(OPMODE).o $(OPMODE)_libs.o $(OPMODE)_exception_handler.o secondboot.o subcpu.o sleep.o	\
				resetcon.o GPIO.o CRC32.o	SecureManager.o															\
				clockinit.o debug.o lib2ndboot.o buildinfo.o														\
				printf.o
SYS_OBJS	+=	sysbus.o

ifeq ($(MEMTYPE),DDR3)
SYS_OBJS	+=	init_DDR3.o
endif
ifeq ($(MEMTYPE),LPDDR3)
SYS_OBJS	+=	init_LPDDR3.o
endif

#SYS_OBJS	+=	CRYPTO.o
#SYS_OBJS	+=	nx_tieoff.o

ifeq ($(INITPMIC),YES)
SYS_OBJS	+=	i2c_gpio.o pmic.o
endif

ifeq ($(SUPPORT_USB_BOOT),y)
CFLAGS		+= -DSUPPORT_USB_BOOT
SYS_OBJS	+=	iUSBBOOT.o
endif

ifeq ($(SUPPORT_SPI_BOOT),y)
CFLAGS		+= -DSUPPORT_SPI_BOOT
SYS_OBJS	+=	iSPIBOOT.o
endif

ifeq ($(SUPPORT_SDMMC_BOOT),y)
CFLAGS		+= -DSUPPORT_SDMMC_BOOT
SYS_OBJS	+=	iSDHCBOOT.o
endif

ifeq ($(SUPPORT_SDFS_BOOT),y)
CFLAGS		+= -DSUPPORT_SDFS_BOOT
SYS_OBJS	+=	diskio.o fatfs.o iSDHCFSBOOT.o
endif

ifeq ($(SUPPORT_NAND_BOOT),y)
CFLAGS		+= -DSUPPORT_NAND_BOOT
SYS_OBJS	+=	iNANDBOOTEC.o
endif

ifeq ($(SUPPORT_UART_BOOT),y)
CFLAGS		+= -DSUPPORT_UART_BOOT
SYS_OBJS	+=	iUARTBOOT.o
endif

ifeq ($(MEMTEST),y)
SYS_OBJS	+=	memtester.o
endif


SYS_OBJS_LIST	=	$(addprefix $(DIR_OBJOUTPUT)/,$(SYS_OBJS))

SYS_INCLUDES	=	-I src				\
			-I prototype/base 		\
			-I prototype/module

###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/%.S
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(ASFLAG) $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################


all: mkobjdir $(SYS_OBJS_LIST) link bin

mkobjdir:
ifeq ($(OS),Windows_NT)
	@if not exist $(DIR_OBJOUTPUT)			\
		@$(MKDIR) $(DIR_OBJOUTPUT)
	@if not exist $(DIR_TARGETOUTPUT)		\
		@$(MKDIR) $(DIR_TARGETOUTPUT)
else
#	@if [ ! -L prototype ] ; then			\
#		ln -s ../../../prototype/s5p6818/ prototype ; \
	fi
	@if	[ ! -e $(DIR_OBJOUTPUT) ]; then 	\
		$(MKDIR) $(DIR_OBJOUTPUT);		\
	fi;
	@if	[ ! -e $(DIR_TARGETOUTPUT) ]; then 	\
		$(MKDIR) $(DIR_TARGETOUTPUT);		\
	fi;
endif

link:
	@echo [link.... $(DIR_TARGETOUTPUT)/$(TARGET_NAME).elf]

	$(Q)$(CC) $(SYS_OBJS_LIST) $(LDFLAGS) -o $(DIR_TARGETOUTPUT)/$(TARGET_NAME).elf

bin:
	@echo [binary.... $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin]
	$(Q)$(MAKEBIN) -O binary $(DIR_TARGETOUTPUT)/$(TARGET_NAME).elf $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin
ifeq ($(OS),Windows_NT)
	@if exist $(DIR_OBJOUTPUT)			\
		@$(RM) $(DIR_OBJOUTPUT)\buildinfo.o
else
	@if	[ -e $(DIR_OBJOUTPUT) ]; then 		\
		$(RM) $(DIR_OBJOUTPUT)/buildinfo.o;	\
	fi;
endif

###################################################################################################
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(DIR_OBJOUTPUT)			\
		@$(RMDIR) $(DIR_OBJOUTPUT)
	@if exist $(DIR_TARGETOUTPUT)			\
		@$(RMDIR) $(DIR_TARGETOUTPUT)
else
	@if [ -L prototype ] ; then			\
		$(RM) prototype ;			\
	fi
	@if	[ -e $(DIR_OBJOUTPUT) ]; then 		\
		$(RMDIR) $(DIR_OBJOUTPUT);		\
	fi;
	@if	[ -e $(DIR_TARGETOUTPUT) ]; then 	\
		$(RMDIR) $(DIR_TARGETOUTPUT);		\
	fi;
endif

-include $(SYS_OBJS_LIST:.o=.d)

