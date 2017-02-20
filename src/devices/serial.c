/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Deokjin, Lee <truevirtue@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sysheader.h>

/* External Function */
extern void clkpwr_set_oscfreq(unsigned int freq_khz);
extern  int clkpwr_get_pllfreq(unsigned int pll_num);

/* Global Variables */
static struct s5p6818_uart_reg *g_uart_reg;

static const unsigned int g_alt_num[10] =
{
	PADINDEX_OF_UART0_UARTRXD,
	PADINDEX_OF_UART0_UARTTXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTRXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTTXD,
	PADINDEX_OF_UART1_UARTRXD,
	PADINDEX_OF_UART1_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTTXD,
};

static const unsigned int g_uart_smc[18] =
{
	TIEOFFINDEX_OF_UART0_USESMC,
	TIEOFFINDEX_OF_UART0_SMCTXENB,
	TIEOFFINDEX_OF_UART0_SMCRXENB,
	TIEOFFINDEX_OF_UART2_USESMC,
	TIEOFFINDEX_OF_UART2_SMCTXENB,
	TIEOFFINDEX_OF_UART2_SMCRXENB,
	TIEOFFINDEX_OF_UART1_USESMC,
	TIEOFFINDEX_OF_UART1_SMCTXENB,
	TIEOFFINDEX_OF_UART1_SMCRXENB,
	TIEOFFINDEX_OF_UART3_USESMC,
	TIEOFFINDEX_OF_UART3_SMCTXENB,
	TIEOFFINDEX_OF_UART3_SMCRXENB,
	TIEOFFINDEX_OF_UART4_USESMC,
	TIEOFFINDEX_OF_UART4_SMCTXENB,
	TIEOFFINDEX_OF_UART4_SMCRXENB,
	TIEOFFINDEX_OF_UART5_USESMC,
	TIEOFFINDEX_OF_UART5_SMCTXENB,
	TIEOFFINDEX_OF_UART5_SMCRXENB
};

static int serial_clkgen_get_baseaddr(unsigned int channel)
{
	const unsigned int clkgen_baseaddr[5] =
	{
		PHY_BASEADDR_CLKGEN22_MODULE,
		PHY_BASEADDR_CLKGEN24_MODULE,
		PHY_BASEADDR_CLKGEN23_MODULE,
		PHY_BASEADDR_CLKGEN25_MODULE,
		PHY_BASEADDR_CLKGEN26_MODULE
	};

	return clkgen_baseaddr[channel];
}

static int serial_get_baseaddr(unsigned int channel)
{
	const unsigned int baseaddr[5] = {
		PHY_BASEADDR_UART0_MODULE,
		PHY_BASEADDR_pl01115_Uart_modem_MODULE,
		PHY_BASEADDR_UART1_MODULE,
		PHY_BASEADDR_pl01115_Uart_nodma0_MODULE,
		PHY_BASEADDR_pl01115_Uart_nodma1_MODULE,
	};

	return baseaddr[channel];
}

static int serial_get_resetnum(unsigned int channel)
{
	const unsigned char reset_num[5] =
	{
		RESETINDEX_OF_UART0_MODULE_nUARTRST,
		RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST,
		RESETINDEX_OF_UART1_MODULE_nUARTRST,
		RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST,
		RESETINDEX_OF_pl01115_Uart_nodma1_MODULE_nUARTRST
	};

	return reset_num[channel];
}

int serial_init(unsigned int channel)
{
	struct s5p6818_clkgen_reg *clkgen_reg
		= (struct s5p6818_clkgen_reg *)(serial_clkgen_get_baseaddr(channel));
	g_uart_reg = (struct s5p6818_uart_reg *)(serial_get_baseaddr(channel));

	int clk_num = CONFIG_S5P_SERIAL_SRCCLK;
	int clk_freq = 0, reg_value = 0;

	clkpwr_set_oscfreq(OSC_KHZ);

	clk_freq = clkpwr_get_pllfreq(clk_num);

	/* Disable the UartX - SmartCard Interface (default:0) */
	reg_value = (1 << ((g_uart_smc[channel * 3 + 0]) & 0x1F));
	mmio_clear_32(&pReg_Tieoff->TIEOFFREG[((g_uart_smc[channel * 3 + 0])& 0xFFFF) >> 5], reg_value);
	reg_value = (1 << ((g_uart_smc[channel * 3 + 1]) & 0x1F));
	mmio_clear_32(&pReg_Tieoff->TIEOFFREG[((g_uart_smc[channel * 3 + 1]) & 0xFFFF) >> 5], reg_value);
	reg_value = (1 << ((g_uart_smc[channel * 3 + 2]) & 0x1F));
	mmio_clear_32(&pReg_Tieoff->TIEOFFREG[((g_uart_smc[channel * 3 + 2]) & 0xFFFF) >> 5], reg_value);

	/* step xx. change the (tx, rx)io gpio-alternate  */
	gpio_set_alt_function(g_alt_num[channel * 2 + 0]);
	gpio_set_alt_function(g_alt_num[channel * 2 + 1]);

	/* step xx. change the reset state in uart block */
	reset_con(serial_get_resetnum(channel), 1);				// reset on
	reset_con(serial_get_resetnum(channel), 0);				// reset negate

	/* step xx. set the (ext:uart clock)clock in uart block*/
	mmio_write_32(&clkgen_reg->clkenb, (1<<3));				// PCLKMODE : always, Clock Gen Disable
	mmio_write_32(&clkgen_reg->clkgen[0],
		((CONFIG_S5P_SERIAL_DIVID-1)<<5) | (clk_num<<2));

	/* step xx. set the uart config */
	mmio_write_32(&g_uart_reg->lcon, 0x3);					// No Parrity, Word bit: 8bit, Stop bit: 1bit
	mmio_write_32(&g_uart_reg->ucon, 0x113340);
	mmio_write_32(&g_uart_reg->fcon, 0x441);				// [6:4] Triger Level : 40byte,  Tx, Rx FIFO Reset
	mmio_write_32(&g_uart_reg->mcon, 0x00);	

	/* step xx. calculates an integer at the baud rate */
	reg_value = getquotient(getquotient(clk_freq, CONFIG_S5P_SERIAL_DIVID),
		((CONFIG_BAUDRATE / 1) * 16)) - 1;
	mmio_write_32(&g_uart_reg->brdr, reg_value);

	/* step xx. calculates an fractional at the baud rate */
	reg_value = getquotient(((getremainder(getquotient(clk_freq, CONFIG_S5P_SERIAL_DIVID),
		((CONFIG_BAUDRATE/1)*16))+32)*16), ((CONFIG_BAUDRATE/1)*16));
	mmio_write_32(&g_uart_reg->fracval, reg_value);

	/* step xx. enable the (ext:uart clock)clock in uart block*/
	mmio_write_32(&clkgen_reg->clkenb, (1<<3) | (1<<2));			// PCLKMODE : always, Clock Gen Enable

	mmio_write_32(&g_uart_reg->ucon, 0x113345);

	return CTRUE;
}

int serial_getch(void)
{
	const unsigned int rx_fifo_empty = (1 << 4);
	while (mmio_read_32(&g_uart_reg->fstatus) & rx_fifo_empty);
	return mmio_read_32(&g_uart_reg->rbr);
}

void serial_putch(int ch)
{
	const unsigned int tx_fifo_full	= (1 << 24);
	while (mmio_read_32(&g_uart_reg->fstatus) & tx_fifo_full);
	mmio_write_32(&g_uart_reg->thr, ch);
}

int serial_is_uart_tx_done(void)
{
	const unsigned int tx_trans_empty = (1 << 2);
	if ((mmio_read_32(&g_uart_reg->fstatus) >> 16) & 0xFF)
		return 0;

	return (mmio_read_32(&g_uart_reg->ustatus) & tx_trans_empty) ? 1 : 0;
}

int serial_is_tx_empty(void)
{
	return ((mmio_read_32(&g_uart_reg->fstatus) >> 16) & 0xFF) ? 0 : 1;
}

int serial_is_busy(void)
{
	const unsigned int tx_trans_empty = (1 << 2);
	return (mmio_read_32(&g_uart_reg->ustatus) & tx_trans_empty) ? 1 : 0;
}
