#ifndef __DREX_H__
#define __DREX_H__

struct s5p6818_drex_qos
{
	volatile unsigned int QOSCONTROL;
	volatile unsigned int _Reserved;
};

struct s5p6818_drex_actiming
{
	volatile unsigned int TIMINGROW;					///< 0x034 : AC Timing for SDRAM Row
	volatile unsigned int TIMINGDATA;					///< 0x038 : AC Timing for SDRAM Data
	volatile unsigned int TIMINGPOWER;					///< 0x03C : AC Timing for Power Mode of SDRAM
};

struct s5p6818_drex_bp
{
	volatile unsigned int BP_CONTROL;					///< 0x2x0 : Back Pressure Control
	volatile unsigned int BP_CONFIG_R;					///< 0x2x4 : Back Pressure Configuration for Read
	volatile unsigned int BP_CONFIG_W;					///< 0x2x8 : Back Pressure Configuration for Write
	volatile unsigned int _Reserved;					///< 0x2xC
};

struct	s5p6818_drex_sdram_reg
{
	volatile unsigned int CONCONTROL;					///< 0x0000 : Controller Control
	volatile unsigned int MEMCONTROL;					///< 0x0004 : Memory Control
	volatile unsigned int CGCONTROL;					///< 0x0008 : Clock Gating Control
	volatile unsigned int _Reserved0;					///< 0x000C
	volatile unsigned int DIRECTCMD;					///< 0x0010 : Memory Direct Command
	volatile unsigned int PRECHCONFIG0;					///< 0x0014 : Precharge Policy Configuration0
	volatile unsigned int PHYCONTROL;					///< 0x0018 : PHY Control0
	volatile unsigned int PRECHCONFIG1;					///< 0x001C : Precharge Policy Configuration1
	volatile unsigned int TIMINGRFCPB;					///< 0x0020 : AC Timing for SDRAM Per Bank Refresh
	volatile unsigned int _Reserved1;					///< 0x0024
	volatile unsigned int PWRDNCONFIG;					///< 0x0028 : Dynamic Power Down Configuration
	volatile unsigned int TIMINGPZQ;					///< 0x002C : AC Timing for DDR3 SDRAM periodic ZQ (ZQCS)
	volatile unsigned int TIMINGAREF;					///< 0x0030 : AC Timing for SDRAM Auto refresh
	struct s5p6818_drex_actiming ACTIMING0;					///< 0x0034~0x03C : AC Timing
	volatile unsigned int PHYSTATUS;					///< 0x0040 : PHY Status
	volatile unsigned int ETCTIMING;					///< 0x0044 : AC Timing for WideIO and Other Parameters
	volatile unsigned int CHIPSTATUS;					///< 0x0048 : Memory Status
	volatile unsigned int _Reserved2[2];					///< 0x004C~0x0050
	volatile unsigned int MRSTATUS;						///< 0x0054 : Memory Mode Registers Status
	volatile unsigned int _Reserved3[2];					///< 0x0058~0x005C
	struct s5p6818_drex_qos QOSCONTROL[16];					///< 0x0060~0x00DC : Quality of Service Control
	volatile unsigned int TIMING_SET_SW;					///< 0x00E0 : Timing Set Switch  Configuration
	struct s5p6818_drex_actiming ACTIMING1;					///< 0x00E4~0x00EC : AC Timing
	volatile unsigned int _Reserved4;					///< 0x00F0
	volatile unsigned int WRTRA_CONFIG;					///< 0x00F4 : Write Training Configuration
	volatile unsigned int RDLVL_CONFIG;					///< 0x00F8 : Read Leveling Configuration (For PHY V5 Only) NXP5430 DDR PHY is V6
	volatile unsigned int PEREVCONFIG0;					///< 0x00FC : Performance Events Configuration
	volatile unsigned int BRBRSVCONTROL;					///< 0x0100 : BRB Reservation Control
	volatile unsigned int BRBRSVCONFIG;					///< 0x0104 : BRB Reservation Configuration
	volatile unsigned int BRBQOSCONFIG;					///< 0x0108 : BRB QoS Configuration
	volatile unsigned int _Reserved5[5];					///< 0x010C~0x11C
	volatile unsigned int WRLVL_CONFIG[2];					///< 0x0120~0x0124 : Write Leveling Configuration
	volatile unsigned int WRLVL_STATUS;					///< 0x0128 : Write Leveling Status
	volatile unsigned int _Reserved6;					///< 0x012C
	volatile unsigned int PPCCLKCON;					///< 0x0130 : Performance Event Clock Control
	volatile unsigned int PEREVCONFIG1[4];					///< 0x0134~0x0140 : Performance Event Configuration
	volatile unsigned int _Reserved7[3];					///< 0x0144~0x014C
	volatile unsigned int CTRL_IO_RDATA;					///< 0x0150 : CTRL_IO_RDATA from PHY
	volatile unsigned int _Reserved8[3];					///< 0x0154~0x015C
	volatile unsigned int CACAL_CONFIG[2];					///< 0x0160~0x0164 : CA Calibration Configuration
	volatile unsigned int CACAL_STATUS;					///< 0x0168 : CA Calibration Status
	volatile unsigned int _Reserved9[37];					///< 0x016C~0x01FC
	volatile unsigned int EMERGENT_CONFIG[2];				///< 0x0200 : Emergent Configuration
	volatile unsigned int _Reserved10[2];					///< 0x0208~0x020C
	struct s5p6818_drex_bp BPCONTROL[4];					///< 0x0210~0x024C
	volatile unsigned int _Reserved11[44];					///< 0x0250~0x02FC
	volatile unsigned int WINCONFIG_ODT_W;					///< 0x0300 : Window Configuration for Write ODT
	volatile unsigned int _Reserved12;					///< 0x0304
	volatile unsigned int WINCONFIG_CTRL_READ;				///< 0x0308 : Window Configuration for CTRLREAD
	volatile unsigned int WINCONFIG_CTRL_GATE;				///< 0x030C : Window Configuration for CTRLGATE
};

struct s5p6818_drex_tzfail
{
	volatile unsigned int TZFAILADDRLOWR;					// 0x00x0 : Read Fail Address Low
	volatile unsigned int TZFAILADDRHIGHR;					// 0x00x4 : Read Fail Address High
	volatile unsigned int TZFAILCTRLR;					// 0x00x8 : Read Fail Control
	volatile unsigned int TZFAILIDR;					// 0x00xC : Read Fail ID
	volatile unsigned int TZFAILADDRLOWW;					// 0x00y0 : Write Fail Address Low
	volatile unsigned int TZFAILADDRHIGH;					// 0x00y4 : Write Fail Address High
	volatile unsigned int TZFAILCTRLW;					// 0x00y8 : Write Fail Control
	volatile unsigned int TZFAILIDW;					// 0x00yC : Write Fail ID
};

struct s5p6818_drex_tzsetup
{
	volatile unsigned int TZRSLOW;						// 0x01x0 : TZ Region Setup Low
	volatile unsigned int TZRSHIGH;						// 0x01x4 : TZ Region Setup High
	volatile unsigned int TZRSATTR;						// 0x01x8 : TZ Region Setup Attribute
	volatile unsigned int _Reserved;					// 0x01xC
};

struct s5p6818_drex_tz_reg
{
	volatile unsigned int TZCONFIG;						// 0x0000 : Configuration
	volatile unsigned int TZACTION;						// 0x0004 : Action
	volatile unsigned int TZLDRANGE;					// 0x0008 : Lockdown Range
	volatile unsigned int TZLDSELECT;					// 0x000C : Lockdown Select
	volatile unsigned int TZINTSTATUS;					// 0x0010 : Interrupt Status
	volatile unsigned int TZINTCLEAR;					// 0x0014 : Interrupt Clear
	volatile unsigned int _Reserved0[10];					// 0x0018~0x003C
	struct s5p6818_drex_tzfail TZFAIL[4];					// 0x0040~0x00BC
	volatile unsigned int _Reserved1[16];					// 0x00C0~0x00FC
	struct s5p6818_drex_tzsetup TZSETUP[9];					// 0x0100~0x018C
	volatile unsigned int _Reserved2[0x31C];				// 0x0190~0x0DFC
	volatile unsigned int TZITCRG;						// 0x0E00 : TZ Integration Test Control
	volatile unsigned int TZITIP;						// 0x0E04 : TZ Integration Test Input
	volatile unsigned int TZITOP;						// 0x0E08 : TZ Integration Test Output
	volatile unsigned int _Reserved3[0x3D];					// 0x0E0C~0x0EFC
	volatile unsigned int MEMBASECONFIG[2];					// 0x0F00~0x0F04 : Memory Chip Base Configuration
	volatile unsigned int _Reserved4[2];					// 0x0F08~0x0F0C
	volatile unsigned int MEMCONFIG[2];					// 0x0F10~0x0F14 : Memory Chip Configuration
};

typedef enum {
	SDRAM_CMD_MRS		= 0x0,						// MRS/EMRS	mode register
	SDRAM_CMD_EMRS		= 0x0,
	SDRAM_CMD_PALL		= 0x1,						// all banks precharge
	SDRAM_CMD_PRE		= 0x2,						// per bank precharge
	SDRAM_CMD_REFS		= 0x4,						// self refresh
	SDRAM_CMD_REFA		= 0x5,						// auto refresh
	// don't use this command if pb_ref_en is enabled in MemControl register
	SDRAM_CMD_CKEL		= 0x6,						// active/precharge power down
	SDRAM_CMD_NOP		= 0x7,						// exit from active/precharge power down
	SDRAM_CMD_REFSX		= 0x8,						// exit from self refresh
	SDRAM_CMD_MRR		= 0x9,						// mode register reading
	SDRAM_CMD_ZQINIT	= 0xa,						// ZQ calibration init
	SDRAM_CMD_ZQOPER	= 0xb,						// ZQ calibration long
	SDRAM_CMD_ZQCS		= 0xc,						// ZQ calibration short
	SDRAM_CMD_SRR		= 0xd						// for Wide IO Memory
} SDRAM_CMD;

#endif // __DREX_H__
