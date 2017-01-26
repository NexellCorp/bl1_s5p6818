/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: DeokJin, Lee <truevirtue@nexell.co.kr>
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

//#define dev_error			ERROR
#define dev_error			printf
//#define dev_error(x)

#define STRETCHING_TIMEOUT		100
#define I2C_DELAY_TIME			2

#define I2C_READ_BIT			1
#define I2C_WRITE_BIT			0

static int g_started;
static unsigned char g_i2c_gpio_grp;
static unsigned char g_i2c_gpio_scl;
static unsigned char g_i2c_gpio_sda;

static struct s5p6818_gpio_reg (*const g_gpio_reg)[1] =
    (struct s5p6818_gpio_reg (*const)[])PHY_BASEADDR_GPIOA_MODULE;

static inline void i2c_delay(unsigned int us)
{
	volatile unsigned int i = 0, j = 0;
	for (i = 0; i < us * 2; i++)
		for (j = 0; j < 5; j++);
}

static void sda_low(void)
{
	mmio_set_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_sda));
}

static void scl_low(void)
{
	mmio_set_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_scl));
}

static int sda_read(void)
{
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1<<g_i2c_gpio_sda));

	return (int)((mmio_read_32(&g_gpio_reg[g_i2c_gpio_grp]->pad) >> g_i2c_gpio_sda) & 1);
}

static int scl_read(void)
{
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_scl));

	return (int)((mmio_read_32(&g_gpio_reg[g_i2c_gpio_grp]->pad) >> g_i2c_gpio_scl) & 1);
}

/* START: High -> Low on SDA while SCL is High */
static int i2c_gpio_send_start(void)
{
	int timeout = STRETCHING_TIMEOUT;

	if (g_started == TRUE) {
		sda_read();

		i2c_delay(I2C_DELAY_TIME);

		while (scl_read() == FALSE) {	// clock stretching.... need timeout code
			if (timeout-- == 0) {
				dev_error("(%s) clock timeout arbitration fail! \r\n", __func__);
				return FALSE;
			}
			i2c_delay(I2C_DELAY_TIME);
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	if (sda_read() == FALSE) {
		dev_error("(%s) sda arbitration fail! \r\n", __func__);
		return FALSE;	// arbitration lost
	}

	sda_low();
	i2c_delay(I2C_DELAY_TIME);
	scl_low();

	g_started = TRUE;

	return TRUE;
}

/* STOP: Low -> High on SDA while SCL is High */
static int i2c_gpio_send_stop(void)
{
	int timeout = STRETCHING_TIMEOUT;

	sda_low();

	i2c_delay(I2C_DELAY_TIME);

	while (scl_read() == FALSE) { // need timeout code
		if(timeout-- == 0) {
			dev_error("stop bit clock timeout arbitration fail! \r\n");
			return FALSE;
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	i2c_delay(I2C_DELAY_TIME);

	sda_read();

	g_started = FALSE;
	timeout = STRETCHING_TIMEOUT;
	while (sda_read() == FALSE) {
		if (timeout-- == 0) {
			dev_error("stop bit sda arbitration fail! \r\n");
			return FALSE; // arbitration_lost
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	i2c_delay(I2C_DELAY_TIME);

	return TRUE;
}

static int i2c_read_bit(int *bit)
{
	unsigned int timeout = STRETCHING_TIMEOUT;

	i2c_delay(I2C_DELAY_TIME);
	sda_read();
	i2c_delay(I2C_DELAY_TIME);

	while (scl_read() == FALSE) {	// clock stretching.... need timeout code
		if(timeout-- == 0) {
			dev_error("(%s) clock timeout arbitration fail! \r\n", __func__);
			return FALSE;
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	*bit = sda_read();		// data or nack read
	i2c_delay(I2C_DELAY_TIME);
	scl_low();

	return TRUE;
}

static int i2c_write_bit(int bit)
{
	int timeout = STRETCHING_TIMEOUT;

	i2c_delay(I2C_DELAY_TIME);

	if (bit)
		sda_read();
	else
		sda_low();

	i2c_delay(I2C_DELAY_TIME);

	while (scl_read() == FALSE) { // clock stretching.... need timeout code
		if(timeout-- == 0) {
			dev_error("(%s) clock timeout arbitration fail! \r\n", __func__);
			return FALSE;
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	if (bit && (sda_read() == FALSE)) {
		dev_error("(%s) sda arbitration fail! \r\n", __func__);
		return FALSE;	// arbitration_lost
	}

	i2c_delay(I2C_DELAY_TIME);

	scl_low();

	return TRUE;
}

/* Send 8 bits and look for an acknowledgement */
static int i2c_write_byte(int send_start, int send_stop, char data, int *nack)
{
	int bit;
	int arbitration;

	if (send_start) {
		arbitration = i2c_gpio_send_start();
		if(arbitration == FALSE)
			return FALSE;
	}

	for (bit = 0; bit < 8; bit++) {
		arbitration = i2c_write_bit((int)((data & 0x80) != 0));
		if (arbitration == FALSE)
			return FALSE;
		data <<= 1;
	}

	arbitration = i2c_read_bit(nack);
	if (arbitration == FALSE)
		return FALSE;

	if (*nack == TRUE) {
		dev_error("(%s) nack returned! \r\n", __func__);
		return FALSE;
	}

	if (send_stop) {
		arbitration = i2c_gpio_send_stop();
		if(arbitration == FALSE)
			return FALSE;
	}

	return TRUE;
}

/**
 * if ack == I2C_ACK, ACK the byte so can continue reading, else
 * send I2C_NOACK to end the read.
 */
static int i2c_read_byte(int nack, int send_stop, char *pdata)
{
	char byte = 0;
	unsigned int bit;
	int result, rbit;

	for (bit = 0; bit < 8; bit++) {
		result = i2c_read_bit(&rbit);
		if (result == FALSE)
			return FALSE;
		byte = (byte << 1) | (rbit ? 1 : 0);
	}
	*pdata = byte;

	result = i2c_write_bit(nack);
	if (result == FALSE)
		return FALSE;

	if (send_stop) {
		result = i2c_gpio_send_stop();
		if(result == FALSE)
			return FALSE;
	}

	return TRUE;
}


int i2c_gpio_read(char dev_addr, char reg_addr, char* pdata, int length)
{
	int nack, result;
	int byte;

	result = i2c_write_byte(TRUE, FALSE, (dev_addr << 1 | I2C_WRITE_BIT), &nack);
	if(result == FALSE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}
	if (nack == TRUE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}

	result = i2c_write_byte(FALSE, FALSE, reg_addr, &nack);
	if (result == FALSE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}
	if (nack == TRUE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}
	result = i2c_write_byte(TRUE, FALSE, (dev_addr << 1 | I2C_READ_BIT), &nack);
	if(result == FALSE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}
	if (nack == TRUE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}

	for(byte = 0; byte < length; ) {
		byte++;
		result = i2c_read_byte((byte == length) ? TRUE : FALSE,
				 (byte == length) ? TRUE : FALSE, pdata++);
		if (result == FALSE) {
			dev_error("i2c-device data read abitration error! \r\n");
			return FALSE;
		}
	}

	return TRUE;
}

int i2c_gpio_write(char dev_addr, char reg_addr, char* pdata, int length)
{
	int nack, result;
	int byte;

	result = i2c_write_byte(TRUE, FALSE, (dev_addr << 1 | I2C_WRITE_BIT), &nack);
	if(result == FALSE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}
	if(nack == TRUE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}

	result = i2c_write_byte(FALSE, FALSE, reg_addr, &nack);
	if(result == FALSE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}
	if(nack == TRUE) {
		dev_error("i2c-device address write abitration error! \r\n");
		return FALSE;
	}

	for (byte = 0; byte < length;) {
		byte++;
		result = i2c_write_byte(FALSE, (byte == length) ? TRUE : FALSE, *pdata++, &nack);
		if(result == FALSE) {
			dev_error("i2c-device data write abitration error! \r\n");
			return FALSE;
		}

		if(nack == TRUE) {
			dev_error("i2c-device data write abitration error! \r\n");
			return FALSE;
		}
	}

	return TRUE;
}

#if 0
void i2c_gpio_deinit(void)
{
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->out,	(1 << g_i2c_gpio_scl));		// low
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->out,	(1 << g_i2c_gpio_sda));		// low
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_scl));	// input
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_sda));	// input

	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_scl>>4],
		3 << ((g_i2c_gpio_scl & 0xF) << 1));
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_scl>>4],
		0 << ((g_i2c_gpio_scl & 0xF) << 1));

	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_sda>>4],
		3 << ((g_i2c_gpio_sda & 0xF) << 1));
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_sda>>4],
		0 << ((g_i2c_gpio_sda & 0xF) << 1));

	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullsel, (1 << g_i2c_gpio_sda));	// pullup
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullenb, (1 << g_i2c_gpio_sda));	// pull enable
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullsel, (1 << g_i2c_gpio_scl));	// pullup
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullenb, (1 << g_i2c_gpio_scl));	// pull enable
}
#endif

void i2c_gpio_init(unsigned char gpio_grp, unsigned char gpio_scl, unsigned char gpio_sda,
	unsigned int gpio_scl_alt, unsigned int gpio_sda_alt)
{
	g_started = TRUE;

	g_i2c_gpio_grp = gpio_grp;
	g_i2c_gpio_scl = gpio_scl;
	g_i2c_gpio_sda = gpio_sda;

	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->out,	(1 << g_i2c_gpio_scl));		// low
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->out,	(1 << g_i2c_gpio_sda));		// low
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_scl));	// input
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1 << g_i2c_gpio_sda));	// input

	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_scl >> 4],
		(3 << ((g_i2c_gpio_scl & 0xF) << 1)));
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_scl >> 4],
		(gpio_scl_alt << ((g_i2c_gpio_scl & 0xF) << 1)));

	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_sda >> 4],
		(3 << ((g_i2c_gpio_sda & 0xF) << 1)));
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->altfn[g_i2c_gpio_sda >> 4],
		(gpio_sda_alt << ((g_i2c_gpio_sda & 0xF) << 1)));

	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullsel, (1 << g_i2c_gpio_sda));	// pullup
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullenb, (1 << g_i2c_gpio_sda));	// pull enable
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullsel, (1 << g_i2c_gpio_scl));	// pullup
	mmio_set_32  (&g_gpio_reg[g_i2c_gpio_grp]->pullenb, (1 << g_i2c_gpio_scl));	// pull enable
}
