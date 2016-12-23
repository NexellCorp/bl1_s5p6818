#ifndef __CLKGEN_H__

struct	s5p6818_clkgen_reg
{
	volatile unsigned int	clkenb;			///< 0x40 : Clock Enable Register
	volatile unsigned int	clkgen[4];		///< 0x44 : Clock Generate Register
};

#endif
