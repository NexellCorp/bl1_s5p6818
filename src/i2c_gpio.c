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

#define I2C_READ			1
#define I2C_WRITE			0

static int g_started;
static unsigned char g_i2c_gpio_grp;
static unsigned char g_i2c_gpio_scl;
static unsigned char g_i2c_gpio_sda;

static struct s5p4418_gpio_reg (*const g_gpio_reg)[1] =
    (struct s5p4418_gpio_reg (*const)[])PHY_BASEADDR_GPIOA_MODULE;

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

	return (int)((mmio_read_32( &g_gpio_reg[g_i2c_gpio_grp]->pad) >> g_i2c_gpio_sda) & 1);
}

static int scl_read(void)
{
	mmio_clear_32(&g_gpio_reg[g_i2c_gpio_grp]->outenb, (1<<g_i2c_gpio_scl));

	return (int)((mmio_read_32(&g_gpio_reg[g_i2c_gpio_grp]->pad) >> g_i2c_gpio_scl) & 1);
}

/* START: High -> Low on SDA while SCL is High */
static int i2c_gpio_send_start(void)
{
	int timeout = STRETCHING_TIMEOUT;

	if (g_started == true) {
		sda_read();

		i2c_delay(I2C_DELAY_TIME);

		while (scl_read() == false) {	// clock stretching.... need timeout code
			if (timeout-- == 0) {
				dev_error("(%s) clock timeout arbitration fail! \r\n", __func__);
				return false;
			}
			i2c_delay(I2C_DELAY_TIME);
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	if (sda_read() == false) {
		dev_error("(%s) sda arbitration fail! \r\n", __func__);
		return false;	// arbitration lost
	}

	sda_low();
	i2c_delay(I2C_DELAY_TIME);
	scl_low();

	g_started = true;

	return true;
}

/* STOP: Low -> High on SDA while SCL is High */
static int i2c_gpio_send_stop(void)
{
	int timeout = STRETCHING_TIMEOUT;

	sda_low();

	i2c_delay(I2C_DELAY_TIME);

	while (scl_read() == false) { // need timeout code
		if(timeout-- == 0) {
			dev_error("stop bit clock timeout arbitration fail! \r\n");
			return false;
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	i2c_delay(I2C_DELAY_TIME);

	sda_read();

	g_started = false;
	timeout = STRETCHING_TIMEOUT;
	while (sda_read() == false) {
		if (timeout-- == 0) {
			dev_error("stop bit sda arbitration fail! \r\n");
			return false; // arbitration_lost
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	i2c_delay(I2C_DELAY_TIME);

	return true;
}

static int i2c_read_bit(int *bit)
{
	unsigned int timeout = STRETCHING_TIMEOUT;

	i2c_delay(I2C_DELAY_TIME);
	sda_read();
	i2c_delay(I2C_DELAY_TIME);

	while (scl_read() == false) {	// clock stretching.... need timeout code
		if(timeout-- == 0) {
			dev_error("(%s) clock timeout arbitration fail! \r\n", __func__);
			return false;
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	*bit = sda_read();		// data or nack read
	i2c_delay(I2C_DELAY_TIME);
	scl_low();

	return true;
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

	while (scl_read() == false) { // clock stretching.... need timeout code
		if(timeout-- == 0) {
			dev_error("(%s) clock timeout arbitration fail! \r\n", __func__);
			return false;
		}
		i2c_delay(I2C_DELAY_TIME);
	}

	if (bit && (sda_read() == false)) {
		dev_error("(%s) sda arbitration fail! \r\n", __func__);
		return false;	// arbitration_lost
	}

	i2c_delay(I2C_DELAY_TIME);

	scl_low();

	return true;
}

/* Send 8 bits and look for an acknowledgement */
static int i2c_write_byte(int send_start, int send_stop, char data, int *nack)
{
	int bit;
	int arbitration;

	if (send_start) {
		arbitration = i2c_gpio_send_start();
		if(arbitration == false)
			return false;
	}

	for (bit = 0; bit < 8; bit++) {
		arbitration = i2c_write_bit((int)((data & 0x80) != 0));
		if (arbitration == false)
			return false;
		data <<= 1;
	}

	arbitration = i2c_read_bit(nack);
	if (arbitration == false)
		return false;

	if (*nack == true) {
		dev_error("(%s) nack returned! \r\n", __func__);
		return false;
	}

	if (send_stop) {
		arbitration = i2c_gpio_send_stop();
		if(arbitration == false)
			return false;
	}

	return true;
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
		if (result == false)
			return false;
		byte = (byte << 1) | (rbit ? 1 : 0);
	}
	*pdata = byte;

	result = i2c_write_bit(nack);
	if (result == false)
		return false;

	if (send_stop) {
		result = i2c_gpio_send_stop();
		if(result == false)
			return false;
	}

	return true;
}


int i2c_gpio_read(char dev_addr, char reg_addr, char* pdata, int length)
{
	int nack, result;
	int byte;

	result = i2c_write_byte(true, false, (dev_addr << 1 | I2C_WRITE), &nack);
	if(result == false) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}
	if (nack == true) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}

	result = i2c_write_byte(false, false, reg_addr, &nack);
	if (result == false) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}
	if (nack == true) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}
	result = i2c_write_byte(true, false, (dev_addr << 1 | I2C_READ), &nack);
	if(result == false) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}
	if (nack == true) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}

	for(byte = 0; byte < length; ) {
		byte++;
		result = i2c_read_byte((byte == length) ? true : false,
				 (byte == length) ? true : false, pdata++);
		if (result == false) {
			dev_error("i2c-device data read abitration error! \r\n");
			return false;
		}
	}

	return true;
}

int i2c_gpio_write(char dev_addr, char reg_addr, char* pdata, int length)
{
	int nack, result;
	int byte;

	result = i2c_write_byte(true, false, (dev_addr << 1 | I2C_WRITE), &nack);
	if(result == false) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}
	if(nack == true) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}

	result = i2c_write_byte(false, false, reg_addr, &nack);
	if(result == false) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}
	if(nack == true) {
		dev_error("i2c-device address write abitration error! \r\n");
		return false;
	}

	for (byte = 0; byte < length; ) {
		byte++;
		result = i2c_write_byte(false, (byte == length) ? true : false, *pdata++, &nack);
		if(result == false) {
			dev_error("i2c-device data write abitration error! \r\n");
			return false;
		}

		if(nack == true) {
			dev_error("i2c-device data write abitration error! \r\n");
			return false;
		}
	}

	return true;
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
	g_started = true;

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
