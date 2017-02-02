#ifndef __RESETCON_H__
#define __RESETCON_H__

#if !defined(NUMBER_OF_RESET_MODULE_PIN)
#undef  NUMBER_OF_RESET_MODULE_PIN
#define NUMBER_OF_RESET_MODULE_PIN	69
#endif

struct s5p6818_resetgen_reg {
	volatile unsigned int regrst[(NUMBER_OF_RESET_MODULE_PIN + 31) >> 5];
};

enum
{
	RSTCON_ASSERT	= 0UL,
	RSTCON_NEGATE	= 1UL
};

/* Function Define */
void reset_con(unsigned int device_num, int enable);

#endif
