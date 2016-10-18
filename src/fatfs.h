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

/*---------------------------------------------------------------------------/
/  FatFs - FAT file system module include file  R0.07a       (C)ChaN, 2009
/----------------------------------------------------------------------------/
/ FatFs module is an open source software to implement FAT file system to
/ small embedded systems. This is a free software and is opened for education,
/ research and commercial developments under license policy of following trems.
/
/  Copyright (C) 2009, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------/
/ FatFs Configuration Options
/
/ CAUTION! Do not forget to make clean the project after any changes to
/ the configuration options.
/
/----------------------------------------------------------------------------*/
#ifndef _FATFS
#define _FATFS

#define _DRIVES 1
/* Number of volumes (logical drives) to be used. */

#define _MAX_SS 512
/* Maximum sector size to be handled. (512/1024/2048/4096) */
/* 512 for memroy card and hard disk, 1024 for floppy disk, 2048 for MO disk */

/* End of configuration options. Do not change followings without care.     */
/*--------------------------------------------------------------------------*/

/* Definitions corresponds to multiple sector size */

#define SS(fs) 512

/* File system object structure */

typedef struct _FATFS {
	U8 fs_type; /* FAT sub type */
	U8 drive;   /* Physical drive number */
	U8 csize;   /* Number of sectors per cluster */
	U8 n_fats;  /* Number of FAT copies */
	U8 wflag;   /* win[] dirty flag (1:must be written back) */
	U8 pad1[3];
	U16 id;	/* File system mount ID */
	U16 n_rootdir; /* Number of root directory entries (0 on FAT32) */
	U32 sects_fat; /* Sectors per fat */
	U32 max_clust; /* Maximum cluster# + 1. Number of clusters is max_clust
			  - 2 */
	U32 fatbase;   /* FAT start sector */
	U32 dirbase;   /* Root directory start sector (Cluster# on FAT32) */
	U32 database;  /* Data start sector */
	U32 winsect;   /* Current sector appearing in the win[] */
	U32 reserved;
	U32 *diskhandle;
	U8 win[_MAX_SS]; /* Disk access window for Directory/FAT */
} FATFS;

/* Directory object structure */

typedef struct _DIR {
	U16 id;     /* Owner file system mount ID */
	U16 index;  /* Current index number */
	FATFS *fs;  /* Pointer to the owner file system object */
	U32 sclust; /* Table start cluster (0:Static table) */
	U32 clust;  /* Current cluster */
	U32 sect;   /* Current sector */
	U8 *dir;    /* Pointer to the current SFN entry in the win[] */
	U8 *fn;     /* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
} DIR;

/* File object structure */

typedef struct _FIL {
	FATFS *fs;      /* Pointer to the owner file system object */
	U16 id;		/* Owner file system mount ID */
	U8 flag;	/* File status flags */
	U8 csect;       /* Sector address in the cluster */
	U32 fptr;       /* File R/W pointer */
	U32 fsize;      /* File size */
	U32 org_clust;  /* File start cluster */
	U32 curr_clust; /* Current cluster */
	U32 dsect;      /* Current data sector */
} FIL;

/* File status structure */

typedef struct _FILINFO {
	U32 fsize;      /* File size */
	U16 fdate;      /* Last modified date */
	U16 ftime;      /* Last modified time */
	U8 fattrib;     /* Attribute */
	char fname[13]; /* Short file name (8.3 format) */
} FILINFO;

/* Character code support macros */

#define IsUpper(c) (((c) >= 'A') && ((c) <= 'Z'))
#define IsLower(c) (((c) >= 'a') && ((c) <= 'z'))
#define IsDigit(c) (((c) >= '0') && ((c) <= '9'))

#define IsDBCS1(c) 0
#define IsDBCS2(c) 0

/* Definitions corresponds to multi partition */

#define LD2PD(drv) (drv) /* Physical drive# is equal to the logical drive# */
#define LD2PT(drv) 0     /* Always mounts the 1st partition */

/* File function return code (FRESULT) */

typedef enum {
	FR_OK = 0,	  /* 0 */
	FR_DISK_ERR,	/* 1 */
	FR_INT_ERR,	 /* 2 */
	FR_NOT_READY,       /* 3 */
	FR_NO_FILE,	 /* 4 */
	FR_NO_PATH,	 /* 5 */
	FR_INVALID_NAME,    /* 6 */
	FR_DENIED,	  /* 7 */
	FR_EXIST,	   /* 8 */
	FR_INVALID_OBJECT,  /* 9 */
	FR_WRITE_PROTECTED, /* 10 */
	FR_INVALID_DRIVE,   /* 11 */
	FR_NOT_ENABLED,     /* 12 */
	FR_NO_FILESYSTEM,   /* 13 */
	FR_MKFS_ABORTED,    /* 14 */
	FR_TIMEOUT	  /* 15 */
} FRESULT;

/*--------------------------------------------------------------*/
/* FatFs module application interface                           */

FRESULT f_mount(const char **path, FATFS *rfs,
		U8 chk_wp);			  /* mount filesystem to disk */
FRESULT f_open(FIL *, const char *, U8, FATFS *); /* Open or create a file */
FRESULT f_read(FIL *, void *, U32, U32 *);	/* Read data from a file */
FRESULT f_close(FIL *); /* Close an open file object */

/*--------------------------------------------------------------*/
/* Flags and offset address                                     */

/* File access control and file status flags (FIL.flag) */

#define FA_READ 0x01
#define FA_OPEN_EXISTING 0x00
#define FA__ERROR 0x80

/* FAT sub type (FATFS.fs_type) */

#define FS_FAT12 1
#define FS_FAT16 2
#define FS_FAT32 3

/* File attribute bits for directory entry */

#define AM_RDO 0x01  /* Read only */
#define AM_HID 0x02  /* Hidden */
#define AM_SYS 0x04  /* System */
#define AM_VOL 0x08  /* Volume label */
#define AM_LFN 0x0F  /* LFN entry */
#define AM_DIR 0x10  /* Directory */
#define AM_ARC 0x20  /* Archive */
#define AM_MASK 0x3F /* Mask of defined bits */

/* FatFs refers the members in the FAT structures with byte offset instead
/ of structure member because there are incompatibility of the packing option
/ between various compilers. */

#define BS_jmpBoot 0
#define BS_OEMName 3
#define BPB_BytsPerSec 11
#define BPB_SecPerClus 13
#define BPB_RsvdSecCnt 14
#define BPB_NumFATs 16
#define BPB_RootEntCnt 17
#define BPB_TotSec16 19
#define BPB_Media 21
#define BPB_FATSz16 22
#define BPB_SecPerTrk 24
#define BPB_NumHeads 26
#define BPB_HiddSec 28
#define BPB_TotSec32 32
#define BS_55AA 510

#define BS_DrvNum 36
#define BS_BootSig 38
#define BS_VolID 39
#define BS_VolLab 43
#define BS_FilSysType 54

#define BPB_FATSz32 36
#define BPB_ExtFlags 40
#define BPB_FSVer 42
#define BPB_RootClus 44
#define BPB_FSInfo 48
#define BPB_BkBootSec 50
#define BS_DrvNum32 64
#define BS_BootSig32 66
#define BS_VolID32 67
#define BS_VolLab32 71
#define BS_FilSysType32 82

#define FSI_LeadSig 0
#define FSI_StrucSig 484
#define FSI_Free_Count 488
#define FSI_Nxt_Free 492

#define MBR_Table 446

#define DIR_Name 0
#define DIR_Attr 11
#define DIR_NTres 12
#define DIR_CrtTime 14
#define DIR_CrtDate 16
#define DIR_FstClusHI 20
#define DIR_WrtTime 22
#define DIR_WrtDate 24
#define DIR_FstClusLO 26
#define DIR_FileSize 28
#define LDIR_Ord 0
#define LDIR_Attr 11
#define LDIR_Type 12
#define LDIR_Chksum 13
#define LDIR_FstClusLO 26

/*--------------------------------*/
/* Multi-byte word access macros  */
#define LD_WORD(ptr)                                                           \
	(U16)(((U16) * (U8 *)((ptr) + 1) << 8) | (U16) * (U8 *)(ptr))
#define LD_DWORD(ptr)                                                          \
	(U32)(((U32) * (U8 *)((ptr) + 3) << 24) |                              \
	      ((U32) * (U8 *)((ptr) + 2) << 16) |                              \
	      ((U16) * (U8 *)((ptr) + 1) << 8) | *(U8 *)(ptr))
#define ST_WORD(ptr, val)                                                      \
	*(U8 *)(ptr) = (U8)(val);                                              \
	*(U8 *)((ptr) + 1) = (U8)((U16)(val) >> 8)
#define ST_DWORD(ptr, val)                                                     \
	*(U8 *)(ptr) = (U8)(val);                                              \
	*(U8 *)((ptr) + 1) = (U8)((U16)(val) >> 8);                            \
	*(U8 *)((ptr) + 2) = (U8)((U32)(val) >> 16);                           \
	*(U8 *)((ptr) + 3) = (U8)((U32)(val) >> 24)

#endif /* _FATFS */
