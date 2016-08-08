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
include ../../../config.mak

SYS_OBJS	= nx_gpio.o nx_alive.o nx_tieoff.o nx_bit_accessor.o nx_intc.o nx_gic400.o		\
		  nx_rstcon.o nx_clkgen.o nx_mcus.o nx_rtc.o nx_wdt.o nx_usb20otg.o			\
		  nx_dma.o nx_crypto.o nx_wdt.o nx_ecid.o nx_pwm.o nx_timer.o nx_ppm.o 			\
		  nx_adc.o nx_tmu.o nx_i2c.o nx_ssp.o nx_uart.o nx_i2s.o nx_spdiftx.o 			\
                  nx_spdifrx.o nx_sdmmc.o nx_clkpwr.o
                              
SYS_OBJS_LIST	=	$(addprefix $(DIR_OBJOUTPUT)/,$(SYS_OBJS))

SYS_INCLUDES	=	-I ./base 							\
					-I ./module					\
					-I ../include 					\
					-I ../../Platform/Support/network		\
					-I ../../config

###################################################################################################
$(DIR_OBJOUTPUT)/%.o: module/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: module/%.S
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(ASFLAG) $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################
$(DIR_OBJOUTPUT)/%.o: base/%.c
	@echo [compile....$<]
	$(Q)$(CC) -MMD $< -c -o $@ $(CFLAGS) $(SYS_INCLUDES)
###################################################################################################


all: mkobjdir $(SYS_OBJS_LIST) link

mkobjdir:
ifeq ($(OS),Windows_NT)
	@if not exist $(DIR_OBJOUTPUT)			\
		@$(MKDIR) $(DIR_OBJOUTPUT)
else
	@if	[ ! -e $(DIR_OBJOUTPUT) ]; then		\
		$(MKDIR) $(DIR_OBJOUTPUT);			\
	fi;
endif

link:
	@echo [link.... libprototype.a]
	$(Q)$(AR) $(ARFLAGS) $(DIR_OBJOUTPUT)/libprototype.a $(SYS_OBJS_LIST)

###################################################################################################
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(DIR_OBJOUTPUT)				\
		@$(RMDIR) $(DIR_OBJOUTPUT)
else
	@if	[ -e $(DIR_OBJOUTPUT) ]; then		\
		$(RMDIR) $(DIR_OBJOUTPUT);			\
	fi;
endif

-include $(SYS_OBJS_LIST:.o=.d)
