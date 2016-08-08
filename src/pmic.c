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

#include "sysheader.h"
void DMC_Delay(int milisecond);

#define AUTO_VOLTAGE_CONTROL (1)
#define ARM_VOLTAGE_CONTROL_SKIP (0)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
//------------------------------------------------------------------------------
#if defined(INITPMIC_YES)

//#define AVN_PMIC_INIT
//#define SVT_PMIC_INIT
//#define ASB_PMIC_INIT
//#define DRONE_PMIC_INIT
//#define BF700_PMIC_INIT
//#define RAPTOR_PMIC_INIT

#define AXP_I2C_GPIO_GRP (-1)
#define NXE2000_I2C_GPIO_GRP (-1)

#ifdef DRONE_PMIC_INIT
#undef AXP_I2C_GPIO_GRP
#define AXP_I2C_GPIO_GRP 		3  // D group
#define AXP_I2C_SCL 			20
#define AXP_I2C_SDA 			16
#define AXP_I2C_SCL_ALT			0
#define AXP_I2C_SDA_ALT			0
#endif

#ifdef BF700_PMIC_INIT
#define MP8845_CORE_I2C_GPIO_GRP	4 // E group, FineDigital VDDA_1.2V (core)
#define MP8845_CORE_I2C_SCL 		9
#define MP8845_CORE_I2C_SDA 		8
#define MP8845_CORE_I2C_SCL_ALT 	0
#define MP8845_CORE_I2C_SDA_ALT		0

#define MP8845_ARM_I2C_GPIO_GRP 	4 // E group, FineDigital VDDB_1.2V (arm)
#define MP8845_ARM_I2C_SCL		11
#define MP8845_ARM_I2C_SDA		10
#define MP8845_ARM_I2C_SCL_ALT		0	
#define MP8845_ARM_I2C_SDA_ALT 		0

#define MP8845_PMIC_INIT (1)
#endif

#ifdef AVN_PMIC_INIT
#define MP8845_CORE_I2C_GPIO_GRP	4 // E group, FineDigital VDDA_1.2V (core)
#define MP8845_CORE_I2C_SCL 		11
#define MP8845_CORE_I2C_SDA 		10
#define MP8845_CORE_I2C_SCL_ALT		0
#define MP8845_CORE_I2C_SDA_ALT		0

#define MP8845_ARM_I2C_GPIO_GRP 	4 // E group, FineDigital VDDB_1.2V (arm)
#define MP8845_ARM_I2C_SCL 		9
#define MP8845_ARM_I2C_SDA 		8
#define MP8845_ARM_I2C_SCL_ALT 		0
#define MP8845_ARM_I2C_SDA_ALT 		0

#define MP8845_PMIC_INIT 		(1)
#endif

#ifdef SVT_PMIC_INIT
#undef NXE2000_I2C_GPIO_GRP
#define NXE2000_I2C_GPIO_GRP 		3 // D group, VCC1P0_CORE, NXE2000, MP8845
#define NXE2000_I2C_SCL 		6
#define NXE2000_I2C_SDA 		7
#define NXE2000_I2C_SCL_ALT 		0
#define NXE2000_I2C_SDA_ALT		0

#define MP8845_I2C_GPIO_GRP 		3 // D group , VCC1P0_ARM, MP8845
#define MP8845_I2C_SCL 			2
#define MP8845_I2C_SDA 			3
#define MP8845_I2C_SCL_ALT		0
#define MP8845_I2C_SDA_ALT		0

#define MP8845_PMIC_INIT 	(1)
#endif

#ifdef ASB_PMIC_INIT
#undef NXE2000_I2C_GPIO_GRP
#define NXE2000_I2C_GPIO_GRP 		3 // D group, VCC1P0_ARM, NXE2000, MP8845
#define NXE2000_I2C_SCL 		2
#define NXE2000_I2C_SDA 		3
#define NXE2000_I2C_SCL_ALT 		0
#define NXE2000_I2C_SDA_ALT 		0

#define MP8845_I2C_GPIO_GRP 		3 // D group , VCC1P0_CORE, MP8845
#define MP8845_I2C_SCL 			6
#define MP8845_I2C_SDA 			7
#define MP8845_I2C_SCL_ALT		0
#define MP8845_I2C_SDA_ALT		0		

#define MP8845_PMIC_INIT 		(1)
#endif

#ifdef RAPTOR_PMIC_INIT
#undef NXE2000_I2C_GPIO_GRP
#define NXE2000_I2C_GPIO_GRP 		3 // D group, VCC1P0_CORE, NXE2000, MP8845
#define NXE2000_I2C_SCL 		6
#define NXE2000_I2C_SDA 		7
#define NXE2000_I2C_SCL_ALT 		0
#define NXE2000_I2C_SDA_ALT 		0

#define MP8845_I2C_GPIO_GRP 		3 // D group , VCC1P0_ARM, MP8845
#define MP8845_I2C_SCL 			2
#define MP8845_I2C_SDA 			3
#define MP8845_I2C_SCL_ALT		0
#define MP8845_I2C_SDA_ALT		0

#define MP8845_PMIC_INIT 		(1)
#endif

#if (AXP_I2C_GPIO_GRP > -1)
#include "pmic_axp228.h"
#endif
#if (NXE2000_I2C_GPIO_GRP > -1)
#include "pmic_nxe2000.h"
#endif
#if (MP8845_PMIC_INIT == 1)
#include "pmic_mp8845.h"
#endif

extern void I2C_Init(U8 gpioGRP, U8 gpioSCL, U8 gpioSDA, U32 gpioSCLAlt, U32 gpioSDAAlt);
// extern void	I2C_Deinit( void );
extern CBOOL I2C_Read(U8 DeviceAddress, U8 RegisterAddress, U8 *pData,
		      U32 Length);
extern CBOOL I2C_Write(U8 DeviceAddress, U8 RegisterAddress, U8 *pData,
		       U32 Length);

#if (MP8845_PMIC_INIT == 1)
static const U8 MP8845_mV_list[] = {
    90, // 12021
    86, // 11753
    83, // 11553
    75, // 11018
    68  // 10549
};

#define MP8845_VOUT_ARRAY_SIZE                                                 \
	(int)(sizeof(MP8845_mV_list) / sizeof(MP8845_mV_list[0]))
#endif // #if (MP8845_PMIC_INIT == 1)

#if (AUTO_VOLTAGE_CONTROL == 1)
struct vdd_core_tb_info {
	U8 ids;
	U8 ro;
	U16 mV;
};

static const struct vdd_core_tb_info vdd_core_tables[] =
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

int getASVIndex(U32 ecid_1)
{
	const struct vdd_core_tb_info *tb = &vdd_core_tables[0];
	int field = 0;
	int ids;
	int ro;
	int ids_L, ro_L;
	int i = 0;

	ids = MtoL((ecid_1 >> 16) & 0xFF, 8);
	ro = MtoL((ecid_1 >> 24) & 0xFF, 8);

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

#if (AXP_I2C_GPIO_GRP > -1)
static U8 axp228_get_dcdc_step(int want_vol, int step, int min, int max)
{
	U32 vol_step = 0;

	if (want_vol < min) {
		want_vol = min;
	} else if (want_vol > max) {
		want_vol = max;
	}

	vol_step = (want_vol - min + step - 1) / step;

	return (U8)(vol_step & 0xFF);
}
#endif

#if (NXE2000_I2C_GPIO_GRP > -1)
static U8 nxe2000_get_ldo7_step(int want_vol)
{
    U32    vol_step = 0;
    U32    temp = 0;

    if (want_vol < NXE2000_DEF_LDOx_VOL_MIN)
        want_vol = NXE2000_DEF_LDOx_VOL_MIN;
    else if (want_vol > NXE2000_DEF_LDOx_VOL_MAX)
        want_vol = NXE2000_DEF_LDOx_VOL_MAX;
    temp = (want_vol - NXE2000_DEF_LDOx_VOL_MIN);

    vol_step    = (temp / NXE2000_DEF_LDOx_VOL_STEP);

    return    (U8)(vol_step & 0x7F);
}

static U8 nxe2000_get_dcdc_step(int want_vol)
{
	U32 vol_step = 0;

	if (want_vol < NXE2000_DEF_DDCx_VOL_MIN) {
		want_vol = NXE2000_DEF_DDCx_VOL_MIN;
	} else if (want_vol > NXE2000_DEF_DDCx_VOL_MAX) {
		want_vol = NXE2000_DEF_DDCx_VOL_MAX;
	}

	vol_step = (want_vol - NXE2000_DEF_DDCx_VOL_MIN +
		    NXE2000_DEF_DDCx_VOL_STEP - 1) /
		   NXE2000_DEF_DDCx_VOL_STEP;

	return (U8)(vol_step & 0xFF);
}
#endif

#ifdef DRONE_PMIC_INIT
inline void PMIC_Drone(void)
{
	U8 pData[4];

	I2C_Init(AXP_I2C_GPIO_GRP, AXP_I2C_SCL, AXP_I2C_SDA,
			AXP_I2C_SCL_ALT, AXP_I2C_SDA_ALT);

	I2C_Read(I2C_ADDR_AXP228, 0x80, pData, 1);
	pData[0] = (pData[0] & 0x1F) | DCDC_SYS | DCDC_DDR;
	I2C_Write(I2C_ADDR_AXP228, 0x80, pData, 1);

	// Set bridge DCDC2 and DCDC3
	I2C_Read(I2C_ADDR_AXP228, 0x37, pData, 1);
	pData[0] |= 0x10;
	I2C_Write(I2C_ADDR_AXP228, 0x37, pData, 1);

//
// ARM voltage change
//
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	pData[0] = axp228_get_dcdc_step(
	    AXP228_DEF_DDC2_VOL, AXP228_DEF_DDC234_VOL_STEP,
	    AXP228_DEF_DDC234_VOL_MIN, AXP228_DEF_DDC24_VOL_MAX);
	I2C_Write(I2C_ADDR_AXP228, AXP228_REG_DC2VOL, pData, 1);
#endif

#if 0
	// Set voltage of DCDC4.
	pData[0] = axp228_get_dcdc_step(AXP228_DEF_DDC4_VOL, AXP228_DEF_DDC234_VOL_STEP, AXP228_DEF_DDC234_VOL_MIN, AXP228_DEF_DDC24_VOL_MAX);
	I2C_Write(I2C_ADDR_AXP228, AXP228_REG_DC4VOL, pData, 1);

	// Set voltage of DCDC5.
	pData[0] = axp228_get_dcdc_step(AXP228_DEF_DDC5_VOL, AXP228_DEF_DDC5_VOL_STEP, AXP228_DEF_DDC5_VOL_MIN, AXP228_DEF_DDC5_VOL_MAX);
	I2C_Write(I2C_ADDR_AXP228, AXP228_REG_DC5VOL, pData, 1);
#endif
}
#endif // DRONE

#if defined(BF700_PMIC_INIT) || defined(AVN_PMIC_INIT)
inline void PMIC_AVN(void)
{
	U8 pData[4];
	U32 ecid_1 = ReadIO32(PHY_BASEADDR_ECID_MODULE + (1 << 2));
	int asv_idx = getASVIndex(ecid_1);

	//
	// I2C init for CORE power.
	//
	I2C_Init(MP8845_CORE_I2C_GPIO_GRP, MP8845_CORE_I2C_SCL, MP8845_CORE_I2C_SDA,
			 MP8845_CORE_I2C_SCL_ALT, MP8845_CORE_I2C_SDA_ALT);

	// PFM -> PWM mode
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
	pData[0] |= 1 << 0;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

	//
	// Core voltage change
	//
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);
	pData[0] |= 1 << 5;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);

#if (AUTO_VOLTAGE_CONTROL == 1) && !defined(BF700_PMIC_INIT)
	if (ecid_1) {
		U8 Data;

		pData[0] = MP8845_mV_list[asv_idx] | 1 << 7;
		Data = pData[0];
		I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);

		I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);

		if (Data != pData[0]) {
			printf("verify arm voltage code write:%d, read:%d\r\n",
			       Data, pData[0]);
		}
	}
#else
#if defined(BF700_PMIC_INIT)
	pData[0] = 90 | 1 << 7; // 1.2V
#endif
#if defined(AVN_PMIC_INIT)
	pData[0] = 75 | 1 << 7; // 1.1V
#endif
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);
#endif

	//
	// I2C init for ARM power.
	//
	I2C_Init(MP8845_ARM_I2C_GPIO_GRP, MP8845_ARM_I2C_SCL, MP8845_ARM_I2C_SDA,
			MP8845_ARM_I2C_SCL_ALT, MP8845_ARM_I2C_SDA_ALT);

	// PFM -> PWM mode
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
	pData[0] |= 1 << 0;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

//
// ARM voltage change
//
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);
	pData[0] |= 1 << 5;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);

#if defined(BF700_PMIC_INIT)
	pData[0] = 90 | 1 << 7; // 1.2V
#endif
#if defined(AVN_PMIC_INIT)
	//    pData[0] = 90 | 1<<7;   // 90: 1.2V
	//    pData[0] = 80 | 1<<7;   // 80: 1.135V
	pData[0] = 75 | 1 << 7; // 75: 1.1V
#endif
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);
#endif
}
#endif // BF700

#ifdef SVT_PMIC_INIT
inline void PMIC_SVT(void)
{
	U8 pData[4];

	//
	// I2C init for CORE & NXE2000 power.
	//
	I2C_Init(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
			NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);

	// PFM -> PWM mode
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
	pData[0] |= 1 << 0;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

//
// Core voltage change
//
#if 1
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);
	pData[0] |= 1 << 5;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);

	//    pData[0] = 90 | 1<<7;   // 90: 1.2V
	//    pData[0] = 80 | 1<<7;   // 80: 1.135V
	pData[0] = 75 | 1 << 7; // 75: 1.1V
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);
#else
	pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC2VOL, pData,
		  1); // Core - second power
#endif

//
// ARM voltage change
//
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC1VOL, pData, 1);
#endif

	pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC4VOL, pData, 1);

	//    pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC5VOL, pData, 1);
}
#endif // SVT

#ifdef ASB_PMIC_INIT
inline void PMIC_ASB(void)
{
	U8 pData[4];
#if 0
	U32 ecid_1 = ReadIO32(PHY_BASEADDR_ECID_MODULE + (1<<2));
	int asv_idx = getASVIndex(ecid_1);
	const struct vdd_core_tb_info *vdd_tb = &vdd_core_tables[asv_idx];
#endif

	//
	// I2C init for Core power.
	//
	I2C_Init(MP8845_I2C_GPIO_GRP, MP8845_I2C_SCL, MP8845_I2C_SDA,
			MP8845_I2C_SCL_ALT, MP8845_I2C_SDA_ALT);

	// PFM -> PWM mode
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
	pData[0] |= 1 << 0;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

	//
	// Core voltage change
	//
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);
	pData[0] |= 1 << 5;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);

#if (AUTO_VOLTAGE_CONTROL == 1)
	if (ecid_1) {
		U8 Data;

		pData[0] = MP8845_mV_list[asv_idx] | 1 << 7;
		Data = pData[0];
		I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);

		I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);

		if (Data != pData[0]) {
			printf("verify arm voltage code write:%d, read:%d\r\n",
			       Data, pData[0]);
		}
	}
#else
	//    pData[0] = 90 | 1<<7;   // 90: 1.2V
	//    pData[0] = 80 | 1<<7;   // 80: 1.135V
	pData[0] = 75 | 1 << 7; // 75: 1.1V
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);
#endif

	//
	// I2C init for ARM & NXE2000 power.
	//
	I2C_Init(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA);

	// PFM -> PWM mode
	I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
	pData[0] |= 1 << 0;
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

//
// ARM voltage change
//
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
#if 1
	//    pData[0] = 90 | 1<<7;   // 90: 1.2V
	//    pData[0] = 80 | 1<<7;   // 80: 1.135V
	pData[0] = 75 | 1 << 7; // 75: 1.1V
	I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);
#else
	pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC1VOL, pData, 1);
#endif
#endif

	//    pData[0] = nxe2000_get_dcdc_step(vdd_tb->mV * 1000);
	pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC2VOL, pData,
		  1); // Core - second power
}
#endif // ASB

#ifdef RAPTOR_PMIC_INIT

extern unsigned int raptor_check_hw_revision(void);

void PMIC_RAPTOR(void)
{
	U8 pData[4];
	U32 board_rev = 0;

	/* Raptor Board Revision Check */
	board_rev = raptor_check_hw_revision();
	if (board_rev >= 0x2) {
		/* I2C init for CORE & NXE2000 power. */
		/* GPIOC, SCL:15(ALT1), SDA:16(ALT1) */
		I2C_Init(2, 15, 16, 1, 1);
	} else {
		/* I2C init for CORE & NXE2000 power. */
		/* GPIOD, SCL:6, SDA:7 */
		I2C_Init(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
				NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);
	}

	// PFM -> PWM mode

	/* ARM voltage change */// 1.25V
	pData[0] = nxe2000_get_dcdc_step(1250000);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC1VOL, pData, 1);
	/* Core voltage change */ // 1.2V
	pData[0] = nxe2000_get_dcdc_step(1200000);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC2VOL, pData, 1);
	/* DDR3 voltage change */ // 1.5
	pData[0] = nxe2000_get_dcdc_step(1500000);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC4VOL, pData, 1);
	/* DDR3 IO voltage change */
	pData[0] = nxe2000_get_dcdc_step(1500000);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC5VOL, pData, 1);

	/*
	 * Raptor Board Rev0X
	 * VDD33_USB0 : VCC3P3_SYS --> VCC2P8_LDO7 (Sleep Mode Power Off)
	 * AVDD18_PLL : VCC1P8_SYS --> VCC1P8_LDO4 (Sleep Mode Power OFF)
	 */
	/* LDO7 IO voltage change*/ // 3.3V
	pData[0] = nxe2000_get_ldo7_step(3300000);
	I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_LDO7VOL, pData, 1);
}
#endif // RAPTOR

void initPMIC(void)
{
#ifdef DRONE_PMIC_INIT
	PMIC_Drone();
#endif // DRONE

#if defined(BF700_PMIC_INIT) || defined(AVN_PMIC_INIT)
	PMIC_AVN();
#endif // BF700

#ifdef SVT_PMIC_INIT
	PMIC_SVT();
#endif // SVT

#ifdef ASB_PMIC_INIT
	PMIC_ASB();
#endif // ASB

#ifdef RAPTOR_PMIC_INIT
	PMIC_RAPTOR();
#endif
	DMC_Delay(100 * 1000);
}
#endif // #if defined( INITPMIC_YES )
