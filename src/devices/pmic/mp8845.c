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

#if defined(MP8845_PMIC_ENABLE)
#include <i2c_gpio.h>
#include <mp8845.h>

const unsigned char MP8845_mV_list[] = {
	90, // 12021
	86, // 11753
	83, // 11553
	75, // 11018
	68  // 10549
};

#define MP8845_VOUT_ARRAY_SIZE                                                 \
	(int)(sizeof(MP8845_mV_list) / sizeof(MP8845_mV_list[0]))

#if (AUTO_VOLTAGE_CONTROL == 1)
struct vdd_core_tb_info {
	unsigned char ids;
	unsigned char ro;
	U16 mV;
};

const struct vdd_core_tb_info vdd_core_tables[] =
    {[0] = {.ids = 6, .ro = 90, .mV = 1200},
     [1] = {.ids = 15, .ro = 130, .mV = 1175},
     [2] = {.ids = 38, .ro = 170, .mV = 1150},
     [3] = {.ids = 78, .ro = 200, .mV = 1100},
     [4] = {.ids = 78, .ro = 200, .mV = 1050}};

#define VDD_CORE_ARRAY_SIZE                                                    \
	(int)(sizeof(vdd_core_tables) / sizeof(vdd_core_tables[0]))

static inline U32 MtoL(U32 data, int bits)
{
	U32 result = 0, mask = 1;
	int i = 0;

	for (i = 0; i < bits; i++) {
		if (data & (1 << i))
			result |= mask << (bits - i - 1);
	}

	return result;
}

int get_asv_index(U32 ecid_1)
{
	const struct vdd_core_tb_info *tb = &vdd_core_tables[0];
	int field = 0;
	int ids;
	int ro;
	int ids_L, ro_L;
	int i = 0;

	ids = MtoL((ecid_1 >> 16) & 0xFF, 8);
	ro  = MtoL((ecid_1 >> 24) & 0xFF, 8);

	/* find ids Level */
	for (i = 0; i < VDD_CORE_ARRAY_SIZE; i++) {
		tb = &vdd_core_tables[i];
		if (ids <= tb->ids)
			break;
	}
	ids_L = i < VDD_CORE_ARRAY_SIZE ? i : (VDD_CORE_ARRAY_SIZE - 1);

	/* find ro Level */
	for (i = 0; i < VDD_CORE_ARRAY_SIZE; i++) {
		tb = &vdd_core_tables[i];
		if (ro <= tb->ro)
			break;
	}
	ro_L = i < VDD_CORE_ARRAY_SIZE ? i : (VDD_CORE_ARRAY_SIZE - 1);

	/* find Lowest ASV Level */
	field = ids_L > ro_L ? ro_L : ids_L;

	return field;
}
#endif // #if (AUTO_VOLTAGE_CONTROL == 1)


/* Do not use it yet. The reason is that code is not generally available. */
int mp8845_write(char addr, char *pdata, int size)
{
	return i2c_gpio_write(I2C_ADDR_MP8845, addr, pdata, size);
}

int mp8845_read(char addr, char *pdata, int size)
{
	return i2c_gpio_read(I2C_ADDR_MP8845, addr, pdata, size);
}

/* Do not use it yet. The reason is that code is not generally available. */
#if 0

#if (defined(SVT_PMIC) && defined(BF700_PMIC))
#undef  MP8845_SYSCCNTL2_CORE
#define MP8845_SYSCCNTL2_CORE		1
#endif

void pmic_mp8845(void)
{
	unsigned char data[4];

	/* mp8845 i2c initialize off core & arm & power */
	i2c_gpio_init(MP8845_I2C_GPIO_GRP, MP8845_I2C_SCL, MP8845_I2C_SDA,
			MP8845_I2C_SCL_ALT, MP8845_I2C_SDA_ALT);

	/* PFM -> PWM mode */
	mp8845_read(MP8845C_REG_SYSCNTL1, data, 1);
	data[0] |= 1 << 0;
	mp8845_write(MP8845C_REG_SYSCNTL1, data, 1);

#if defined(MP8845_SYSCCNTL2_CORE)
	/* Core Voltage */
	mp8845_read(MP8845C_REG_SYSCNTL2, data, 1);
	data[[0] |= 1 << 5;
	mp8845_write(MP8845C_REG_SYSCNTL2, data, 1);
#endif

	/* set the arm voltage */
#if (AUTO_VOLTAGE_CONTROL == 1)
	int ecid_1  = mmio_read_32(PHY_BASEADDR_ECID_MODULE + (1 << 2));
	int asv_idx = get_asv_index(ecid_1);
	const struct vdd_core_tb_info *vdd_tb = &vdd_core_tables[asv_idx];

	if (ecid_1) {
		unsigned char t_data;

		data[0] = MP8845_mV_list[asv_idx] | 1 << 7;
		t_data = data[0];
		mp8845_write(MP8845C_REG_VSEL, data, 1);

		mp8845_read(MP8845C_REG_VSEL, data, 1);

		if (t_data != data[0])
			printf("verify arm voltage code write:%d, read:%d\r\n",
				t_data, data[0]);
	}
#else
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	data[0] = 90 | 1<<7;		// 90: 1.2V
//	data[0] = 80 | 1<<7;		// 80: 1.135V
//	data[[0] = 75 | 1 << 7;		// 75: 1.1V
	mp8845_write(MP8845C_REG_VSEL, data, 1);
#endif
#endif
}
#endif

#endif // #if defined(MP8845_PMIC_ENABLE)
