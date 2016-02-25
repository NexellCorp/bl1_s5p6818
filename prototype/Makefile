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
 #      File            : Makefile
 #      Description     :                                                       
 #      Author          : Firware Team                                          
 #      History         :                                                       
 #
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
