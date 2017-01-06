#ifndef __S5P6818_AVN_REF_H__
#define __S5P6818_AVN_REF_H__

#include <cfgFreqDefine.h>
#include <clock.h>

/* system clock macro */
#define CONFIG_S5P_PLL0_FREQ			800
#define CONFIG_S5P_PLL1_FREQ			800
#define CONFIG_S5P_PLL2_FREQ			614
#define CONFIG_S5P_PLL3_FREQ			800

// CPUDVOREG
#define CONFIG_S5P_PLLx_DVO0			((NX_CLKSRC_PLL_1 << CLKSRC_BITPOS) |	/* PLL Select */			\
						((1 - 1) << DVO0_BITPOS)	    |	/* FCLK ==> CPU Group 0 */		\
	                			((4 - 1) << DVO1_BITPOS))		/* HCLK ==> CPU bus (max 250MHz) */
// CPUDVOREG
#define CONFIG_S5P_PLLx_DVO7			((NX_CLKSRC_PLL_1 << CLKSRC_BITPOS) |	/* PLL Select */			\
	                			((1 - 1) << DVO0_BITPOS)	    |	/* FCLK ==> CPU Group1 */		\
	                			((4 - 1) << DVO1_BITPOS))		/* HCLK ==> CPU bus (max 250MHz) */
// BUSDVOREG
#define CONFIG_S5P_PLLx_DVO1			((NX_CLKSRC_PLL_0 << CLKSRC_BITPOS) |	/* PLL Select */				\
	                			((2 - 1) << DVO0_BITPOS)	    |	/* BCLK ==> System bus (max 333MHz) */	\
	                			((2 - 1) << DVO1_BITPOS))		/* PCLK ==> Peripheral bus (max 166MHz) */
// MEMDVOREG
#define CONFIG_S5P_PLLx_DVO2			((NX_CLKSRC_PLL_3 << CLKSRC_BITPOS) |	/* PLL Select */					\
						((1 - 1) << DVO0_BITPOS)	    |	/* MDCLK ==> Memory DLL (max 800MHz) */	\
						((1 - 1) << DVO1_BITPOS) 	    |	/* MCLK  ==> Memory DDR (max 800MHz) */	\
						((2 - 1) << DVO2_BITPOS) 	    |	/* MBCLK ==> MCU bus (max 400MHz) */		\
						((2 - 1) << DVO3_BITPOS))		/* MPCLK ==> MCU Peripheral bus (max 200MHz) */ \

#define CONFIG_S5P_PLLx_DVO3			((NX_CLKSRC_PLL_0 << CLKSRC_BITPOS) |	/*GRP3DVOREG*/					\
						((2 - 1) << DVO0_BITPOS)	    |	/*GR3DBCLK ==> GPU bus & core (max 333MHz) */	\
						((2 - 1) << DVO1_BITPOS))		/* GR3DPCLK ==> not used */


#define CONFIG_S5P_PLLx_DVO4			((NX_CLKSRC_PLL_0 << CLKSRC_BITPOS) |	/* MPEGDVOREG */					\
						((2 - 1) << DVO0_BITPOS)	    |	/* MPEGBCLK ==> MPEG bus & core (max 300MHz) */	\
						((2 - 1) << DVO1_BITPOS))		/* MPEGPCLK ==> MPEG control if (max 150MHz) */

#define CONFIG_S5P_PLLx_DVO5			((NX_CLKSRC_PLL_0 << CLKSRC_BITPOS) |	/* DISPLAY BUS */					\
						((2 - 1) << DVO0_BITPOS)	    |	/* DISPLAY BCLK ==> DISPLAY bus (max 300MHz) */	\
						((2 - 1) << DVO1_BITPOS))		/* DISPLAY PCLK ==> DISPLAY control if (max 150MHz) */

#define CONFIG_S5P_PLLx_DVO6			((NX_CLKSRC_PLL_0 << CLKSRC_BITPOS) |	/* HDMI (max 100MHz) */				\
						((8 - 1) << DVO0_BITPOS))

#define CONFIG_S5P_PLLx_DVO8			((NX_CLKSRC_PLL_3 << CLKSRC_BITPOS) |	/* FAST BUS (max 400MHz) */					\
						((2 - 1) << DVO0_BITPOS)	    |	/* FAST BUS BCLK ==> FAST bus (max 400MHz) */		\
						((2 - 1) << DVO1_BITPOS))		/* FAST BUS PCLK ==> FAST BUS control if (max 200MHz) */

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

#define CONFIG_BAUDRATE				115200

#define CONFIG_UART_CLKGEN_CLOCK_HZ		0

#endif // #ifndef __S5P6818_AVN_REF_H__
