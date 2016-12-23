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

#define EMA_VALUE		(1)	// Manual setting - 1: 1.1V, 3: 1.0V, 4: 0.95V

/*************************************************************
 * Must be S5P6818
 * EMA(Extra Margin Adjustment)
 * According to the arm voltage, it sets the appropriate ema value in use the sram
 * and instruction cache.
 *************************************************************/
void cache_setup_ema(void)
{
	int reg_value = 0;

	/* Set EMA for CPU Cluster0 */
	reg_value  = mmio_read_32(&pReg_Tieoff->TIEOFFREG[94])
		& ~((0x7 << 23) | (0x7 << 17));
	reg_value |= ((EMA_VALUE << 23) | (EMA_VALUE << 17));
	mmio_write_32(&pReg_Tieoff->TIEOFFREG[94], reg_value);

	/* Set EMA for CPU Cluster1 */
	reg_value  = mmio_read_32(&pReg_Tieoff->TIEOFFREG[111])
		& ~((0x7 << 23) | (0x7 << 17));
	reg_value |= ((EMA_VALUE << 23) | (EMA_VALUE << 17));
	mmio_write_32(&pReg_Tieoff->TIEOFFREG[111], reg_value);

#if (CONFIG_RESET_AFFINITY_ID == 1)
	/* Set Affinity Level1 for CPU Cluster1 */
	reg_value = mmio_read_32(&pReg_Tieoff->TIEOFFREG[95]) & 0x00FFFFFF;
	reg_value |= (1 << 24);
	mmio_write_32(&pReg_Tieoff->TIEOFFREG[95], reg_value);

	/* Set Affinity Level2 for CPU Cluster1 */
	reg_value = mmio_read_32(&pReg_Tieoff->TIEOFFREG[96]) & 0xF0;
//	reg_value |= (1 << 0);
	mmio_write_32(&pReg_Tieoff->TIEOFFREG[96], reg_value);
#endif
}

void ema_information(void)
{
	NOTICE("EMA is %s\r\n", (EMA_VALUE == 1) ? "1.1V" : (EMA_VALUE == 3) ? "1.0V" : "0.95V");
	NOTICE("Working to aarch%d\r\n waiting for pll change..\r\n", sizeof(void *) * 8);
}
