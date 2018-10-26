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
include config.mak

LDFLAGS		=	-Bstatic							\
			-Wl,-Map=$(DIR_TARGETOUTPUT)/$(TARGET_NAME).map,--cref		\
			-T$(LDS_NAME).lds						\
			-Wl,--start-group						\
			-Lsrc/$(DIR_OBJOUTPUT)						\
			-Wl,--end-group							\
			-Wl,--build-id=none						\
			-nostdlib

SYS_OBJS	=	startup_$(OPMODE).o $(OPMODE)_libs.o $(OPMODE)_exception_handler.o plat_pm.o sub_cpu.o			\
			clock.o cci400.o resetcon.o gpio.o secure_manager.o lib2ndboot.o build_info.o				\
			serial.o printf.o crc.o ema.o clkpwr.o board_$(BOARD).o ${MEMTYPE}_sdram.o memory.o main.o

SYS_OBJS	+=	sysbus.o

SYS_OBJS	+=	CRYPTO.o
#SYS_OBJS	+=	nx_tieoff.o

ifeq ($(PMIC_ON),y)
SYS_OBJS	+=	i2c_gpio.o pmic.o nxe2000.o mp8845.o axp228.o
endif

ifeq ($(SUPPORT_USB_BOOT),y)
CFLAGS		+= -DSUPPORT_USB_BOOT
SYS_OBJS	+=	iUSBBOOT.o
endif

ifeq ($(SUPPORT_SDMMC_BOOT),y)
CFLAGS		+= -DSUPPORT_SDMMC_BOOT
SYS_OBJS	+=	iSDHCBOOT.o
endif

ifeq ($(SUPPORT_SPI_BOOT),y)
CFLAGS		+= -DSSUPPORT_SPI_BOOT
SYS_OBJS	+=	iSPIBOOT.o
endif

ifeq ($(SUPPORT_NAND_BOOT),y)
CFLAGS		+= -DSUPPORT_NAND_BOOT
SYS_OBJS	+=	iNANDBOOTEC.o
endif

ifeq ($(SUPPORT_SDFS_BOOT),y)
CFLAGS		+= -DSUPPORT_SDFS_BOOT
SYS_OBJS	+=	iSDHCFSBOOT.o
SYS_OBJS	+=	diskio.o fatfs.o
endif

ifeq ($(MEMTEST),y)
SYS_OBJS	+=	memtester.o
endif

SYS_OBJS_LIST	=	$(addprefix $(DIR_OBJOUTPUT)/,$(SYS_OBJS))

SYS_INCLUDES	=	-I src								\
			-I src/boot							\
			-I src/devices							\
			-I src/devices/memory						\
			-I src/devices/memory/ddr3					\
			-I src/devices/memory/lpddr3					\
			-I src/devices/pmic						\
			-I src/board							\
			-I src/configs							\
			-I src/test							\
			-I prototype/base 						\
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
$(DIR_OBJOUTPUT)/%.o: src/boot/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/devices/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/devices/memory/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/devices/memory/ddr3/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/devices/memory/lpddr3/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/devices/pmic/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: src/board/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################

$(DIR_OBJOUTPUT)/%.o: src/test/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################

all: mkobjdir $(SYS_OBJS_LIST) link bin bingen

mkobjdir:
ifeq ($(OS),Windows_NT)
	@if not exist $(DIR_OBJOUTPUT)				\
		@$(MKDIR) $(DIR_OBJOUTPUT)
	@if not exist $(DIR_TARGETOUTPUT)			\
		@$(MKDIR) $(DIR_TARGETOUTPUT)
else
#	@if [ ! -L prototype ] ; then				\
#		ln -s ../../../prototype/s5p6818/ prototype ; 	\
	fi
	@if	[ ! -e $(DIR_OBJOUTPUT) ]; then 		\
		$(MKDIR) $(DIR_OBJOUTPUT);			\
	fi;
	@if	[ ! -e $(DIR_TARGETOUTPUT) ]; then 		\
		$(MKDIR) $(DIR_TARGETOUTPUT);			\
	fi;
endif

link:
	@echo [link.... $(DIR_TARGETOUTPUT)/$(TARGET_NAME).elf]

	$(Q)$(CC) $(SYS_OBJS_LIST) $(LDFLAGS) -o $(DIR_TARGETOUTPUT)/$(TARGET_NAME).elf

bin:
	@echo [binary.... $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin]
	$(Q)$(MAKEBIN) -O binary $(DIR_TARGETOUTPUT)/$(TARGET_NAME).elf $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin

bingen:
ifeq ($(KERNEL_VER), 3)
	./tools/bingen -c $(CHIPNAME) -i $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin -o $(DIR_TARGETOUTPUT)/bl1-emmcboot.bin -l 0xFFFF0000 -e 0xFFFF0000 -d SDMMC -u $(DEVICE_PORT) -a 0x8000
else
	./tools/bingen -c $(CHIPNAME) -i $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin -o $(DIR_TARGETOUTPUT)/bl1-emmcboot.bin -l 0xFFFF0000 -e 0xFFFF0000 -d SDMMC -u $(DEVICE_PORT) -a 0x10200
	./tools/bingen -c $(CHIPNAME) -i $(DIR_TARGETOUTPUT)/$(TARGET_NAME).bin -o $(DIR_TARGETOUTPUT)/bl1-sdboot.bin -l 0xFFFF0000 -e 0xFFFF0000 -d SDMMC -u 0 -a 0x10200
endif

ifeq ($(OS),Windows_NT)
	@if exist $(DIR_OBJOUTPUT)				\
		@$(RM) $(DIR_OBJOUTPUT)\buildinfo.o
else
	@if	[ -e $(DIR_OBJOUTPUT) ]; then 			\
		$(RM) $(DIR_OBJOUTPUT)/buildinfo.o;		\
	fi;
endif

###################################################################################################
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(DIR_OBJOUTPUT)				\
		@$(RMDIR) $(DIR_OBJOUTPUT)
	@if exist $(DIR_TARGETOUTPUT)				\
		@$(RMDIR) $(DIR_TARGETOUTPUT)
else
	@if [ -L prototype ] ; then				\
		$(RM) prototype ;				\
	fi
	@if	[ -e $(DIR_OBJOUTPUT) ]; then 			\
		$(RMDIR) $(DIR_OBJOUTPUT);			\
	fi;
	@if	[ -e $(DIR_TARGETOUTPUT) ]; then 		\
		$(RMDIR) $(DIR_TARGETOUTPUT);			\
	fi;
endif

-include $(SYS_OBJS_LIST:.o=.d)
