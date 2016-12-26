#ifndef __S5P6818_H__
#define __S5P6818_H__

#define CFG_NSIH_EN				1

/* NXPXX30 - Chip Product */
#define ARCH_NXP5430

/* Support the Kernel Version */
#if defined(KERNEL_VER_3_4)
#define SUPPORT_KERNEL_3_4			1
#else
#define SUPPORT_KERNEL_3_4			0
#endif

/* System Option */
#if (SUPPORT_KERNEL_3_4 == 1)
#define MULTICORE_SLEEP_CONTROL			1
#define MULTICORE_BRING_UP			1
#define CCI400_COHERENCY_ENABLE			1
#define CONFIG_RESET_AFFINITY_ID		0				// Dependent kernel 3.18.x
#else	// #if (SUPPORT_KERNEL_3_4 == 1)
#define MULTICORE_SLEEP_CONTROL			1
#define MULTICORE_BRING_UP			0
#define CCI400_COHERENCY_ENABLE			1
#define CONFIG_RESET_AFFINITY_ID		1				// Dependent kernel 3.18.x
#endif

/* DRAM(DDR3/LPDDR3) Memory Configuration */
#ifdef MEMTYPE_DDR3
#define MEM_TYPE_DDR3
#endif
#ifdef MEMTYPE_LPDDR3
#define MEM_TYPE_LPDDR23
#endif

/* Clock (PLLx) Configuration */
#define OSC_HZ					(24000000)
#define OSC_KHZ					(OSC_HZ/1000)
#define OSC_MHZ					(OSC_KHZ/1000)

#define CLKSRC_PLL_0				0
#define CLKSRC_PLL_1				1
#define CLKSRC_PLL_2				2
#define CLKSRC_PLL_3				3

#define CLKSRC_UART				CLKSRC_PLL_2
#define CLKSRC_SDMMC				CLKSRC_PLL_2
#define CLKSRC_SPI				CLKSRC_PLL_0

/* Serial Console Configuration */
#define CONFIG_S5P_SERIAL
#define CONFIG_S5P_SERIAL_INDEX			0
#define CONFIG_S5P_SERIAL_CLOCK			50000000

#define CONFIG_S5P_SERIAL_SRCCLK		0
#define CONFIG_S5P_SERIAL_DIVID			4
#define CONFIG_S5P_SERIAL_

#define CONFIG_BAUDRATE				115200

#define CONFIG_UART_CLKGEN_CLOCK_HZ		0

/* CRC Check Configuration */
#define SUPPORT_SW_CRC_CHECK			1

#endif // __S5P6818_H__
