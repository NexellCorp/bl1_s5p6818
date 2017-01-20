#ifndef __DDRPHY_H__
#define __DDRPHY_H__

struct	s5p6818_ddrphy_reg
{
	volatile unsigned int PHY_CON[6];			///< 0x000 ~ 0x014 : PHY Control
	volatile unsigned int LP_CON;				///< 0x018 : Low Power Control
	volatile unsigned int RODT_CON;				///< 0x01C : Read ODT Control
	volatile unsigned int OFFSETR_CON[3];			///< 0x020 ~ 0x028 : READ Code Control
	volatile unsigned int _Reserved0;			///< 0x02C
	volatile unsigned int OFFSETW_CON[3];			///< 0x030 ~ 0x038 : Write Code Control
	volatile unsigned int _Reserved1;			///< 0x03C
	volatile unsigned int OFFSETC_CON[3];			///< 0x040 ~ 0x048 : GATE Code Control
	volatile unsigned int SHIFTC_CON;			///< 0x04C : GATE Code Shift Control
	volatile unsigned int OFFSETD_CON;			///< 0x050 : CMD Code Control
	volatile unsigned int _Reserved2;			///< 0x054
	volatile unsigned int LP_DDR_CON[5];			///< 0x058 ~ 0x068 : LPDDR Control
	volatile unsigned int WR_LVL_CON[4];			///< 0x06C ~ 0x078 : Write Leveling Control
	volatile unsigned int CA_DSKEW_CON[4];			///< 0x07C ~ 0x088 : CA Deskew Control
	volatile unsigned int _Reserved3[2];			///< 0x08C ~ 0x090
	volatile unsigned int CA_DSKEW_CON4;			///< 0x094 : CA Deskew Control
	volatile unsigned int _Reserved4;			///< 0x098
	volatile unsigned int DRVDS_CON[2];			///< 0x09C ~ 0x0A0 : Driver Strength Control
	volatile unsigned int _Reserved5[3];			///< 0x0A4 ~ 0x0AC
	volatile unsigned int MDLL_CON[2];			///< 0x0B0 ~ 0x0B4 : MDLL Control
	volatile unsigned int _Reserved6[2];			///< 0x0B8 ~ 0x0BC
	volatile unsigned int ZQ_CON;				///< 0x0C0 : ZQ Control
	volatile unsigned int ZQ_STATUS;				///< 0x0C4 : ZQ Status
	volatile unsigned int ZQ_DIVIDER;			///< 0x0C8 : ZQ Divider Control
	volatile unsigned int ZQ_TIMER;				///< 0x0CC : ZQ Timer Control
	volatile unsigned int T_RDDATA_CON[3];			///< 0x0D0 ~ 0x0D8 : Read Data Enable Status
	volatile unsigned int CAL_WL_STAT;			///< 0x0DC : WL Calibration Fail Status
	volatile unsigned int CAL_FAIL_STAT[4];			///< 0x0E0 ~ 0x0EC : Calibration Fail Status
	volatile unsigned int CAL_GT_VWMC[3];			///< 0x0F0 ~ 0x0F8 : Calibration Gate Training Centering Code
	volatile unsigned int CAL_GT_CYC;			///< 0x0FC : Calibration Gate Training Cycle
	volatile unsigned int CAL_RD_VWMC[3];			///< 0x100 ~ 0x108 : Calibration Read Center Code
	volatile unsigned int _Reserved7;			///< 0x10C
	volatile unsigned int CAL_RD_VWML[3];			///< 0x110 ~ 0x118 : Calibration Read Left Code
	volatile unsigned int _Reserved8;			///< 0x11C
	volatile unsigned int CAL_RD_VWMR[3];			///< 0x120 ~ 0x128 : Calibration Read Right Code
	volatile unsigned int _Reserved9;			///< 0x12C
	volatile unsigned int CAL_WR_VWMC[3];			///< 0x130 ~ 0x138 : Calibration Write Center Code
	volatile unsigned int _Reserved10;			///< 0x13C
	volatile unsigned int CAL_WR_VWML[3];			///< 0x140 ~ 0x148 : Calibration Write Left Code
	volatile unsigned int _Reserved11;			///< 0x14C
	volatile unsigned int CAL_WR_VWMR[3];			///< 0x150 ~ 0x158 : Calibration Write Right Code
	volatile unsigned int _Reserved12;			///< 0x15C
	volatile unsigned int CAL_DM_VWMC[3];			///< 0x160 ~ 0x168 : Calibration DM Center Code
	volatile unsigned int _Reserved13;			///< 0x16C
	volatile unsigned int CAL_DM_VWML[3];			///< 0x170 ~ 0x178 : Calibration DM Left Code
	volatile unsigned int _Reserved14;			///< 0x17C
	volatile unsigned int CAL_DM_VWMR[3];			///< 0x180 ~ 0x188 : Calibration DM Right Code
	volatile unsigned int _Reserved15;			///< 0x18C
	volatile unsigned int RD_DESKEW_CON[24];			///< 0x190 ~ 0x1EC : READ DeSkew Control
	volatile unsigned int WR_DESKEW_CON[24];			///< 0x1F0 ~ 0x24C : WRITE DeSkew Control
	volatile unsigned int DM_DESKEW_CON[3];			///< 0x250 ~ 0x258 : DM DeSkew Control
	volatile unsigned int VWMC_STAT[24];			///< 0x25C ~ 0x2B8 : VWMC Status
	volatile unsigned int DM_VWMC_STAT[3];			///< 0x2BC ~ 0x2C4 : DM VWMC
	volatile unsigned int VWML_STAT[24];			///< 0x2C8 ~ 0x324 : VWML Status
	volatile unsigned int DM_VWML_STAT[3];			///< 0x328 ~ 0x330 : DM VWML
	volatile unsigned int VWMR_STAT[24];			///< 0x334 ~ 0x390 : VWMR Status
	volatile unsigned int DM_VWMR_STAT[3];			///< 0x394 ~ 0x39C : DM VWMR
	volatile unsigned int DQ_IO_RDATA[3];			///< 0x3A0 ~ 0x3A8 : DQ I/O Read Data Status
	volatile unsigned int VERSION_INFO;			///< 0x3AC : Version Information
};

#endif // __DDRPHY_H__
