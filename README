2nd bootloader for Nexell s5pxx18 series SoC
============================================

This README contains information about Nexell s5pxx18 series Soc's
2nd bootloader source. 


What is 2nd bootloader?
----------------------

Rom Boot loads 2nd bootloader from non volatile storage to internal SRAM 
and jump to it, and 2nd bootloader does cpu initialization and set up,
and the memory initialization. In the end, 2nd bootloader loads 3rd bootloader
(u-boot or ARM ATF so on) from non volatile storage to DRAM and jump to it.


Description of Important files
------------------------------

StartUp Entry
 - Rom Boot jump to here
File Path : src/startup_arch{XX}.S

C Main Code File
 - after stack set up, Assem code jump to BootMain entry 
File Path : src/secondboot.c

Power Manager IC 
 - PMIC Control code for ARM & DDR Voltage
File Path : src/pmic.c

DDR3 Memory Init
 - DDR3 Memory Initialize Code.
File Path : src/init_DDR3.c

LPDDR3 Memory Init
 - LPDDR3 Memory Initialize Code.
File Path : src/init_LPDDR3.c

Config File
 - Global Defines and compiler setting
File Path : config.mak


Source Tree
-----------
.
├── src
├── prototype
├── ┐── base
│   ┗── module
├── nsih-generator
└── refernce-nsih
 
    
How to build
------------

Get Toolchain
  # wget https://releases.linaro.org/14.04/components/toolchain/binaries/gcc-linaro-aarch64-none-elf-4.8-2014.04_linux.tar.bz2
  # tar xvjf gcc-linaro-aarch64-none-elf-4.8-2014.04_linux.tar.bz2
  # export PATH={gcc-linaro-path}/bin:$PATH

Fix config.mak
 - select OPMODE
 - select BOARD
 - set CROSS_TOOL_TOP: {gcc-linaro-aarch64-path}/bin

Build
   # make
