#ifndef __GPIO_H__
#define __GPIO_H__

struct	s5p4418_gpio_reg {
	volatile unsigned int out;			///< 0x00 : Output Register
	volatile unsigned int outenb;			///< 0x04 : Output Enable Register
	volatile unsigned int detmode[2];		///< 0x08 : Event Detect Mode Register
	volatile unsigned int intenb;			///< 0x10 : Interrupt Enable Register
	volatile unsigned int det;			///< 0x14 : Event Detect Register
	volatile unsigned int pad;			///< 0x18 : PAD Status Register
	volatile unsigned int puenb;			///< 0x1C : Pull Up Enable Register
	volatile unsigned int altfn[2];			///< 0x20 : Alternate Function Select Register
	volatile unsigned int detmodeex;		///< 0x28 : Event Detect Mode extended Register

	volatile unsigned int reserved[4];		///< 0x2B :
	volatile unsigned int detenb;			///< 0x3C : IntPend Detect Enable Register

	volatile unsigned int slew;			///< 0x40 : Slew Register
	volatile unsigned int slew_disalbe_default;	///< 0x44 : Slew set On/Off Register
	volatile unsigned int drv1;			///< 0x48 : drive strength LSB Register
	volatile unsigned int drv1_disable_default;	///< 0x4C : drive strength LSB set On/Off Register
	volatile unsigned int drv0;			///< 0x50 : drive strength MSB Register
	volatile unsigned int drv0_disable_default;	///< 0x54 : drive strength MSB set On/Off Register
	volatile unsigned int pullsel;			///< 0x58 : Pull UP/DOWN Selection Register
	volatile unsigned int pullsel_disable_default;	///< 0x5C : Pull UP/DOWN Selection On/Off Register
	volatile unsigned int pullenb;			///< 0x60 : Pull Enable/Disable Register
	volatile unsigned int pullenb_disable_default;	///< 0x64 : Pull Enable/Disable selection On/Off Register
	volatile unsigned int input_mux_select0;	///< 0x68
	volatile unsigned int input_mux_select1;	///< 0x6C
	volatile unsigned char reserved1[0x1000-0x70];
};

#endif
