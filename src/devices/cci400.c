/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sangjong, Han <hans@nexell.co.kr>
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
#include <cci400.h>

#if (CCI400_COHERENCY_ENABLE == 1)

struct s5p6818_cci400_reg *const g_cci400_reg =
    (struct s5p6818_cci400_reg * const)PHY_BASEADDR_CCI400_MODULE;

void cci400_initialize(void)
{
	/* before set barrier instruction. */
	mmio_set_32(&g_cci400_reg->scr, 1 << 0);				// static bus disable speculative fetches
	mmio_set_32(&g_cci400_reg->scr, 1 << 1);				// SFR bus disable speculative fetches

	mmio_write_32(&g_cci400_reg->cor, (1UL << 3));				// protect to send barrier command to drex

	mmio_write_32(&g_cci400_reg->csi[BUSID_CS].scr, 0);			// snoop request disable
	mmio_write_32(&g_cci400_reg->csi[BUSID_CODA].scr, 0);			// snoop request disable
	mmio_write_32(&g_cci400_reg->csi[BUSID_TOP].scr, 0);			// snoop request disable

#if (MULTICORE_BRING_UP == 1)
	mmio_write_32(&g_cci400_reg->csi[BUSID_CPUG0].scr, 0x3);		// cpu 0~3 Snoop & DVM Req
	while (mmio_read_32(&g_cci400_reg->stsr) & 0x1);

	mmio_write_32(&g_cci400_reg->csi[BUSID_CPUG1].scr, 0x3);		// cpu 4~7 Snoop & DVM Req
	while (mmio_read_32(&g_cci400_reg->stsr) & 0x1);
#else
	mmio_write_32(&g_cci400_reg->csi[BUSID_CPUG0].scr, 0x0);
	mmio_write_32(&g_cci400_reg->csi[BUSID_CPUG1].scr, 0x0);
#endif

}

#endif // #if (CCI400_COHERENCY_ENABLE == 1)
