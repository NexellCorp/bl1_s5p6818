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

/*----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.07a                    (C)ChaN, 2009
/-----------------------------------------------------------------------------/
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
//----------------------------------------------------------------------------*/
#include <nx_type.h>
#include "fatfs.h"  /* FatFs configurations and declarations */
#include "diskio.h" /* Declarations of low level disk I/O functions */
#include "printf.h"
#include <nx_debug2.h>

#ifdef DEBUG
#define dprintf(x, ...) printf(x, ...)
#else
#define dprintf(x, ...)
#endif

/*--------------------------------------------------------------------------

   Module Private Definitions

---------------------------------------------------------------------------*/

#define ENTER_FF(fs)
#define LEAVE_FF(fs, res) return res

#define ABORT(fs, res)                                                         \
	{                                                                      \
		fp->flag |= FA__ERROR;                                         \
		LEAVE_FF(fs, res);                                             \
	}

#ifndef NULL
#define NULL 0
#endif

/*--------------------------------------------------------------------------

   Private Work Area

---------------------------------------------------------------------------*/

#define NAMEBUF(sp, lp) U8 sp[12]
#define INITBUF(dj, sp, lp) dj.fn = sp

/*--------------------------------------------------------------------------

   Private Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/* Copy memory to memory */
static void mem_cpy(void *dst, const void *src, int cnt)
{
	char *d = (char *)dst;
	const char *s = (const char *)src;
	while (cnt--)
		*d++ = *s++;
}

/* Fill memory */
static void mem_set(void *dst, int val, int cnt)
{
	char *d = (char *)dst;
	while (cnt--)
		*d++ = (char)val;
}

/* Compare memory to memory */
static int mem_cmp(const void *dst, const void *src, int cnt)
{
	const char *d = (const char *)dst, *s = (const char *)src;
	int r = 0;
	while (cnt-- && (r = *d++ - *s++) == 0)
		;
	return r;
}

/* Check if chr is contained in the string */
static int chk_chr(const char *str, int chr)
{
	while (*str && *str != chr)
		str++;
	return *str;
}

/*-----------------------------------------------------------------------*/
/* Change window offset                                                  */
/*-----------------------------------------------------------------------*/

static FRESULT
move_window(FATFS *fs, /* File system object */
	    U32 sector /* Sector number to make apperance in the fs->win[] */
	    )	  /* Move to zero only writes back dirty window */
{
	U32 wsect;

	wsect = fs->winsect;
	if (wsect != sector) /* Changed current window */
	{
		if (sector) {
			if (disk_read(fs->drive, fs->win, sector, 1,
				      fs->diskhandle) != RES_OK)
				return FR_DISK_ERR;
			fs->winsect = sector;
		}
	}

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Get a cluster status                                                  */
/*-----------------------------------------------------------------------*/

static U32
get_cluster(/* 0xFFFFFFFF:Disk error, 1:Interal error, Else:Cluster status */
	    FATFS *fs, /* File system object */
	    U32 clst   /* Cluster# to get the link information */
	    )
{
	U16 wc, bc;
	U32 fsect;

	if (clst < 2 || clst >= fs->max_clust) /* Check cluster address range */
		return 1;

	fsect = fs->fatbase;
	switch (fs->fs_type) {
	case FS_FAT12:
		bc = (U16)clst * 3 / 2;
		if (move_window(fs, fsect + (bc / SS(fs))))
			break;
		wc = fs->win[bc & (SS(fs) - 1)];
		bc++;
		if (move_window(fs, fsect + (bc / SS(fs))))
			break;
		wc |= (U16)fs->win[bc & (SS(fs) - 1)] << 8;
		return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);

	case FS_FAT16:
		if (move_window(fs, fsect + (clst / (SS(fs) / 2))))
			break;
		return LD_WORD(&fs->win[((U16)clst * 2) & (SS(fs) - 1)]);

	case FS_FAT32:
		if (move_window(fs, fsect + (clst / (SS(fs) / 4))))
			break;
		return LD_DWORD(&fs->win[((U16)clst * 4) & (SS(fs) - 1)]) &
		       0x0FFFFFFF;
	}

	return 0xFFFFFFFF; /* An error occured at the disk I/O layer */
}

/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/

static U32 clust2sect(/* !=0: sector number, 0: failed - invalid cluster# */
		      FATFS *fs, /* File system object */
		      U32 clst   /* Cluster# to be converted */
		      )
{
	clst -= 2;
	if (clst >= (fs->max_clust - 2)) /* Invalid cluster# */
		return 0;
	return clst * fs->csize + fs->database;
}

/*-----------------------------------------------------------------------*/
/* Seek directory index                                                  */
/*-----------------------------------------------------------------------*/

static FRESULT dir_seek(DIR *dj, /* Pointer to directory object */
			U16 idx  /* Directory index number */
			)
{
	U32 clst;
	U16 ic;

	dj->index = idx;
	clst = dj->sclust;
	if (clst == 1 ||
	    clst >= dj->fs->max_clust) /* Check start cluster range */
	{
		dprintf("start cluster range err\r\n");
		return FR_INT_ERR;
	}

	if (clst == 0) /* Static table */
	{
		//		if (idx >= dj->fs->n_rootdir)		/* Index
		//is out of range */
		if (idx > dj->fs->n_rootdir) /* Index is out of range */
		{
			dprintf("out of range\r\n");
			return FR_INT_ERR;
		}
		dj->sect = dj->fs->dirbase + idx / (SS(dj->fs) / 32);
	} else /* Dynamic table */
	{
		ic = SS(dj->fs) / 32 * dj->fs->csize; /* Indexes per cluster */
		while (idx >= ic)		      /* Follow cluster chain */
		{
			clst = get_cluster(dj->fs, clst); /* Get next cluster */
			if (clst == 0xFFFFFFFF) {
				dprintf("get cluster disk error\r\n");
				return FR_DISK_ERR; /* Disk error */
			}
			if (clst < 2 ||
			    clst >= dj->fs->max_clust) /* Reached to end of
							  table or int error */
			{
				dprintf("end of cluster\r\n");
				return FR_INT_ERR;
			}
			idx -= ic;
		}
		dj->clust = clst;
		dj->sect = clust2sect(dj->fs, clst) + idx / (SS(dj->fs) / 32);
	}
	dj->dir = dj->fs->win + (idx % (SS(dj->fs) / 32)) * 32;

	return FR_OK; /* Seek succeeded */
}

/*-----------------------------------------------------------------------*/
/* Move directory index next                                             */
/*-----------------------------------------------------------------------*/

static FRESULT dir_next(	 /* FR_OK:Succeeded, FR_NO_FILE:End of table,
				    FR_DENIED:EOT and could not streach */
			DIR *dj, /* Pointer to directory object */
			CBOOL streach /* FALSE: Do not streach table, TRUE:
					 Streach table if needed */
			)
{
	U32 clst;
	U16 i;

	streach = streach; // unused parameter
	dprintf("find next dir\r\n");

	i = dj->index + 1;
	if (!i || !dj->sect) /* Report EOT when index has reached 65535 */
	{
		dprintf("end of table\r\n");
		return FR_NO_FILE;
	}

	if (!(i % (SS(dj->fs) / 32))) /* Sector changed? */
	{
		dj->sect++; /* Next sector */
		dprintf("next sector\r\n");

		if (dj->sclust == 0) /* Static table */
		{
			if (i >=
			    dj->fs
				->n_rootdir) /* Report EOT when end of table */
			{
				dprintf("end of table no file\r\n");
				return FR_NO_FILE;
			}
		} else /* Dynamic table */
		{
			if (((i / (SS(dj->fs) / 32)) & (dj->fs->csize - 1)) ==
			    0) /* Cluster changed? */
			{
				clst = get_cluster(
				    dj->fs, dj->clust); /* Get next cluster */
				if (clst <= 1) {
					dprintf("low cluster\r\n");
					return FR_INT_ERR;
				}
				if (clst == 0xFFFFFFFF) {
					dprintf("too high cluster\r\n");
					return FR_DISK_ERR;
				}
				if (clst >=
				    dj
					->fs->max_clust) { /* When it reached
							      end of dinamic
							      table */
					dprintf("max cluster no file\r\n");
					return FR_NO_FILE; /* Report EOT */
				}
				dj->clust =
				    clst; /* Initialize data for new cluster */
				dj->sect = clust2sect(dj->fs, clst);
				dprintf("cluster changed\r\n");
			}
		}
	}

	dj->index = i;
	dj->dir = dj->fs->win + (i % (SS(dj->fs) / 32)) * 32;
	//	NX_DEBUG_MSG( dj->dir );

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Find an object in the directory                                       */
/*-----------------------------------------------------------------------*/

static FRESULT
dir_find(DIR *dj /* Pointer to the directory object linked to the file name */
	 )
{
	FRESULT res;
	U8 a, c, *dir;

	res = dir_seek(dj, 0); /* Rewind directory object */
	if (res != FR_OK) {
		dprintf("rewind dir obj fail\r\n");
		return res;
	}

	do {
		res = move_window(dj->fs, dj->sect);
		if (res != FR_OK) {
			dprintf("cannot see next window\r\n");
			break;
		}
		dir = dj->dir; /* Ptr to the directory entry of current index */
		dprintf("%s\r\n", dir);
		c = dir[DIR_Name];
		if (c == 0) {
			dprintf("no file\r\n");
			res = FR_NO_FILE;
			break;
		} /* Reached to end of table */
		a = dir[DIR_Attr] & AM_MASK;
		if (c != 0xE5 && c != '.' && !(a & AM_VOL) &&
		    !mem_cmp(dir, dj->fn, 11)) /* Is it a valid entry? */
		{
			dprintf("catch valid entry\r\n");
			break;
		}
		res = dir_next(dj, CFALSE); /* Next entry */
	} while (res == FR_OK);

	return res;
}

/*-----------------------------------------------------------------------*/
/* Pick a segment and create the object name in directory form           */
/*-----------------------------------------------------------------------*/

static FRESULT create_name(
    DIR *dj,	  /* Pointer to the directory object */
    const char **path /* Pointer to pointer to the segment in the path string */
    )
{
	U8 c, d, b, *sfn;
	int ni, si, i;
	const char *p;

	/* Create file name in directory form */
	sfn = dj->fn;
	mem_set(sfn, ' ', 11);
	si = i = b = 0;
	ni = 8;
	p = *path;
	for (;;) {
		c = p[si++];
		if (c < ' ' || c == '/' ||
		    c == '\\') /* Break on end of segment */
			break;
		if (c == '.' || i >= ni) {
			if (ni != 8 || c != '.') {
				dprintf("not . or 8\r\n");
				return FR_INVALID_NAME;
			}
			i = 8;
			ni = 11;
			b <<= 2;
			continue;
		}
		if (c >=
		    0x80) /* If there is any extended char, eliminate NT flag */
			b |= 3;
		if (IsDBCS1(c)) /* If it is DBC 1st byte */
		{
			d = p[si++];			/* Get 2nd byte */
			if (!IsDBCS2(d) || i >= ni - 1) /* Reject invalid DBC */
			{
				dprintf("invalid DBC\r\n");
				return FR_INVALID_NAME;
			}
			sfn[i++] = c;
			sfn[i++] = d;
		} else {
			if (chk_chr(" +,;[=]\"*:<>\?|\x7F",
				    c)) /* Reject unallowable chrs for SFN */
			{
				dprintf("unallowable char\r\n");
				return FR_INVALID_NAME;
			}
			if (IsUpper(c)) {
				b |= 2;
			} else {
				if (IsLower(c)) {
					b |= 1;
					c -= 0x20;
				}
			}
			sfn[i++] = c;
		}
	}
	*path = &p[si];	/* Rerurn pointer to the next segment */
	c = (c < ' ') ? 4 : 0; /* Set last segment flag if end of path */

	if (!i) /* Reject null string */
	{
		dprintf("null string\r\n");
		return FR_INVALID_NAME;
	}
	if (sfn[0] ==
	    0xE5) /* When first char collides with 0xE5, replace it with 0x05 */
		sfn[0] = 0x05;

	if (ni == 8)
		b <<= 2;
	if ((b & 0x03) == 0x01) /* NT flag (Extension has only small capital) */
		c |= 0x10;
	if ((b & 0x0C) == 0x04) /* NT flag (Filename has only small capital) */
		c |= 0x08;

	sfn[11] = c; /* Store NT flag, File name is created */

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

static FRESULT
follow_path(	 /* FR_OK(0): successful, !=0: error code */
	    DIR *dj, /* Directory object to return last directory and found
			object */
	    const char *path /* Full-path string to find a file or directory */
	    )
{
	FRESULT res;
	U8 *dir, last;

	if (*path == '/' || *path == '\\') /* Strip heading separator */
		path++;

	dj->sclust = /* Set start directory (root dir) */
	    (dj->fs->fs_type == FS_FAT32) ? dj->fs->dirbase : 0;
	dprintf("dj->fs->dirbase = 0x%08X dj->sclust = 0x%08X\r\n",
		dj->fs->dirbase, dj->sclust);

	dprintf("%s\r\n", path);
	if ((U8)*path < ' ') /* Null path means the root directory */
	{
		res = dir_seek(dj, 0);
		dj->dir = NULL;
		dprintf("it's root\r\n");
	} else { /* Follow path */
		for (;;) {
			res = create_name(dj, &path); /* Get a segment */
			if (res != FR_OK) {
				dprintf("create name fail\r\n");
				break;
			}
			dprintf("find dir: %s\r\n", dj->fn);
			res = dir_find(dj); /* Find it */
			last = *(dj->fn + 11) & 4;
			if (res != FR_OK) /* Could not find the object */
			{
				dprintf("no dir found\r\n");
				if (res == FR_NO_FILE && !last)
					res = FR_NO_PATH;
				break;
			}
			if (last) {
				dprintf("last path\r\n");
				break; /* Last segment match. Function
					  completed. */
			}
			dir =
			    dj->dir; /* There is next segment. Follow the sub
					directory */
			dprintf("%s\r\n", dir);
			if (!(dir[DIR_Attr] &
			      AM_DIR)) /* Cannot follow because it is a file */
			{
				dprintf("it's directory\r\n");
				res = FR_NO_PATH;
				break;
			}
			dj->sclust = ((U32)LD_WORD(dir + DIR_FstClusHI) << 16) |
				     LD_WORD(dir + DIR_FstClusLO);
			dprintf("dj->sclust = 0x%08X\r\n", dj->sclust);
		}
	}

	return res;
}

/*-----------------------------------------------------------------------*/
/* Load boot record and check if it is an FAT boot record                */
/*-----------------------------------------------------------------------*/

static U8
check_fs(/* 0:The FAT boot record, 1:Valid boot record but not an FAT, 2:Not a
	    boot record, 3:Error */
	 FATFS *fs, /* File system object */
	 U32 sect   /* Sector# (lba) to check if it is an FAT boot record or not
		       */
	 )
{
	if (disk_read(fs->drive, fs->win, sect, 1, fs->diskhandle) !=
	    RES_OK) /* Load boot record */
	{
		dprintf("Cannot read disk\r\n");
		return 3;
	}
	if (LD_WORD(
		&fs->win
		     [BS_55AA]) != 0xAA55) /* Check record signature (always
					      placed at offset 510 even if the
					      sector size is >512) */
	{
		dprintf("Cannot find record signature\r\n");
		return 2;
	}

	if (!mem_cmp(&fs->win[BS_FilSysType], "FAT",
		     3)) /* Check FAT signature */
	{
		dprintf("FAT\r\n");
		return 0;
	}
	if (!mem_cmp(&fs->win[BS_FilSysType32], "FAT32", 5) &&
	    !(fs->win[BPB_ExtFlags] & 0x80)) {
		dprintf("FAT32\r\n");
		return 0;
	}

	return 1;
}

/*-----------------------------------------------------------------------*/
/* Make sure that the file system is valid                               */
/*-----------------------------------------------------------------------*/
int getquotient(int mk, int na);

// static
FRESULT
f_mount(/* FR_OK(0): successful, !=0: any error occured */
	const char **
	    path,   /* Pointer to pointer to the path name (drive number) */
	FATFS *rfs, /* Pointer to pointer to the found file system object */
	U8 chk_wp   /* !=0: Check media write protection for write access */
	)
{
	FRESULT res;
	U8 vol, fmt, *tbl;
#ifdef DEBUG
	U8 partition_id = 0
#endif
	    DSTATUS stat;
	U32 bsect, fsize, tsect, mclst;
	const char *p = *path;
	FATFS *fs;

	chk_wp = chk_wp;

	/* Get logical drive number from the path name */
	vol = p[0] - '0'; /* Is there a drive number? */
	if (vol <= 9 && p[1] == ':') {
		p += 2;    /* Found a drive number, get and strip it */
		*path = p; /* Return pointer to the path name */
	} else {
		vol = 0; /* No drive number is given, use drive number 0 as
			    default */
	}

	/* Check if the logical drive number is valid or not */
	if (vol >= _DRIVES) {
		dprintf("invalid drive\r\n");
		return FR_INVALID_DRIVE; /* Is the drive number valid? */
	}
	fs = rfs; /* Returen pointer to the corresponding file system object */
	if (!fs) {
		dprintf("invalid fs obj\r\n");
		return FR_NOT_ENABLED; /* Is the file system object registered?
					  */
	}

	ENTER_FF(fs); /* Lock file system */

	if (fs->fs_type) /* If the logical drive has been mounted */
	{
		stat = disk_status(fs->drive);
		if (!(stat &
		      STA_NOINIT)) /* and physical drive is kept initialized
				      (has not been changed), */
		{
			dprintf("disk is ready already\r\n");
			return FR_OK; /* The file system object is valid */
		}
	}

	/* The logical drive must be re-mounted. Following code attempts to
	 * mount the volume */

	fs->fs_type = 0; /* Clear the file system object */
	fs->drive =
	    LD2PD(vol); /* Bind the logical drive and a physical drive */
	stat = disk_initialize(
	    fs->drive);	/* Initialize low level disk I/O layer */
	if (stat & STA_NOINIT) /* Check if the drive is ready */
	{
		dprintf("disk not ready\r\n");
		return FR_NOT_READY;
	}

	/* Search FAT partition on the drive */
	fmt = check_fs(fs, bsect = 0); /* Check sector 0 as an SFD format */
	dprintf("check MBR :%d\r\n", fmt);
	if (fmt == 1 ||
	    fmt == 0) /* Not an FAT boot record, it may be patitioned */
	{
		dprintf("it may be patitioned\r\n");
		/* Check a partition listed in top of the partition table */
		tbl =
		    &fs->win[MBR_Table + LD2PT(vol) * 16]; /* Partition table */
		if (tbl[4]) /* Is the partition existing? */
		{
#ifdef DEBUG
			partition_id = tbl[4];
#endif
			bsect = LD_DWORD(&tbl[8]); /* Partition offset in LBA */
			dprintf("partition_id: 0x%08X\r\n", partition_id);
			dprintf("check partition\r\n");
			fmt = check_fs(fs, bsect); /* Check the partition */
		}
	}
	if (fmt == 3) {
		dprintf("disk error\r\n");
		return FR_DISK_ERR;
	}
	if (fmt ||
	    LD_WORD(fs->win + BPB_BytsPerSec) !=
		SS(fs)) /* No valid FAT patition is found */
	{
		dprintf("fs no catch\r\n");
		return FR_NO_FILESYSTEM;
	}

	dprintf("fs type %d\r\n", fmt);

	/* Initialize the file system object */
	fsize = LD_WORD(fs->win + BPB_FATSz16); /* Number of sectors per FAT */
	if (!fsize)
		fsize = LD_DWORD(fs->win + BPB_FATSz32);
	fs->sects_fat = fsize;
	fs->n_fats = fs->win[BPB_NumFATs]; /* Number of FAT copies */
	fsize *= fs->n_fats;		   /* (Number of sectors in FAT area) */
	fs->fatbase =
	    bsect +
	    LD_WORD(fs->win + BPB_RsvdSecCnt); /* FAT start sector (lba) */
	fs->csize = fs->win[BPB_SecPerClus]; /* Number of sectors per cluster */
	fs->n_rootdir = LD_WORD(
	    fs->win + BPB_RootEntCnt); /* Nmuber of root directory entries */
	tsect = LD_WORD(
	    fs->win + BPB_TotSec16); /* Number of sectors on the file system */
	if (!tsect)
		tsect = LD_DWORD(fs->win + BPB_TotSec32);
	fs->max_clust = mclst =
	    getquotient((tsect /* Last cluster# + 1 */
			 - LD_WORD(fs->win + BPB_RsvdSecCnt) - fsize -
			 fs->n_rootdir / (SS(fs) / 32)),
			fs->csize + 2);
	dprintf("mclst : 0x%08X n_rootdir : 0x%08X\r\n", mclst, fs->n_rootdir);

	fmt = FS_FAT12; /* Determine the FAT sub type */

	if (mclst >= 0xFF7) {
		dprintf("fat 16!\r\n");
		fmt = FS_FAT16; /* Number of clusters >= 0xFF5 */
	}
	if (mclst >= 0xFFF7) {
		dprintf("fat 32!\r\n");
		fmt = FS_FAT32; /* Number of clusters >= 0xFFF5 */
	}

	if (fmt == FS_FAT32) {
		dprintf("bpb root cluster\r\n");
		fs->dirbase = LD_DWORD(
		    fs->win + BPB_RootClus); /* Root directory start cluster */
	} else {
		dprintf("dir base is lba\r\n");
		fs->dirbase =
		    fs->fatbase + fsize; /* Root directory start sector (lba) */
	}
	dprintf("fs type %d\r\n", fmt);
	fs->database =
	    fs->fatbase + fsize +
	    fs->n_rootdir / (SS(fs) / 32); /* Data start sector (lba) */

	fs->winsect = 0;
	fs->fs_type = fmt; /* FAT syb-type */
	fs->id = 0xDEAD;   /* File system mount ID */
	res = FR_OK;

	return res;
}

/*-----------------------------------------------------------------------*/
/* Check if the file/dir object is valid or not                          */
/*-----------------------------------------------------------------------*/

static FRESULT
validate(	   /* FR_OK(0): The object is valid, !=0: Invalid */
	 FATFS *fs, /* Pointer to the file system object */
	 U16 id     /* Member id of the target object to be checked */
	 )
{
	if (!fs || !fs->fs_type || fs->id != id)
		return FR_INVALID_OBJECT;

	ENTER_FF(fs); /* Lock file system */

	if (disk_status(fs->drive) & STA_NOINIT)
		return FR_NOT_READY;

	return FR_OK;
}

/*--------------------------------------------------------------------------

   Public Functions

--------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_open(FIL *fp,		 /* Pointer to the blank file object */
	       const char *path, /* Pointer to the file name */
	       U8 mode,		 /* Access mode and file open mode flags */
	       FATFS *pfs)
{
	FRESULT res;
	DIR dj;
	NAMEBUF(sfn, lfn);
	U8 *dir;

	dj.fs = pfs;
	fp->fs = NULL; /* Clear file object */
	mode &= FA_READ;
#if 0
	res = f_mount(&path, dj.fs, 0);
	if (res != FR_OK)
	{
		NX_DEBUG_MSG( "disk mount failure\r\n" );
		LEAVE_FF(dj.fs, res);
	}
#endif
	INITBUF(dj, sfn, lfn);
	res = follow_path(&dj, path); /* Follow the file path */

	if (res != FR_OK) {
		dprintf("Invalid file path\r\n");
		LEAVE_FF(dj.fs, res); /* Follow failed */
	}
	dir = dj.dir;
	if (!dir || (dir[DIR_Attr] & AM_DIR)) /* It is a directory */
	{
		dprintf("Invalid dir\r\n");
		LEAVE_FF(dj.fs, FR_NO_FILE);
	}
	fp->flag = mode; /* File access mode */
	fp->org_clust =  /* File start cluster */
	    ((U32)LD_WORD(dir + DIR_FstClusHI) << 16) |
	    LD_WORD(dir + DIR_FstClusLO);
	fp->fsize = LD_DWORD(dir + DIR_FileSize); /* File size */
	fp->fptr = 0;
	fp->csect = 255; /* File pointer */
	fp->dsect = 0;
	fp->fs = dj.fs;
	fp->id = dj.fs->id; /* Owner file system object of the file */

	LEAVE_FF(dj.fs, FR_OK);
}

/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_read(FIL *fp,    /* Pointer to the file object */
	       void *buff, /* Pointer to data buffer */
	       U32 btr,    /* Number of bytes to read */
	       U32 *br     /* Pointer to number of bytes read */
	       )
{
	FRESULT res;
	U32 clst, sect, remain;
	U32 rcnt, cc;
	U8 *rbuff = buff;

	*br = 0;

	res = validate(fp->fs, fp->id); /* Check validity of the object */
	if (res != FR_OK) {
		dprintf("Not valid file\r\n");
		LEAVE_FF(fp->fs, res);
	}
	if (fp->flag & FA__ERROR) /* Check abort flag */
	{
		dprintf("file flag error\r\n");
		LEAVE_FF(fp->fs, FR_INT_ERR);
	}
	if (!(fp->flag & FA_READ)) /* Check access mode */
	{
		dprintf("access mode error\r\n");
		LEAVE_FF(fp->fs, FR_DENIED);
	}
	remain = fp->fsize - fp->fptr;
	if (btr > remain)
		btr = (U32)remain; /* Truncate btr by remaining bytes */

	for (; btr; /* Repeat until all data transferred */
	     rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt) {
		if ((fp->fptr % SS(fp->fs)) == 0) /* On the sector boundary? */
		{
			if (fp->csect >=
			    fp->fs->csize) /* On the cluster boundary? */
			{
				clst =
				    (fp->fptr == 0)
					? /* On the top of the file? */
					fp->org_clust
					: get_cluster(fp->fs, fp->curr_clust);
				if (clst <= 1) {
					dprintf("read cluster is 0 or 1\r\n");
					ABORT(fp->fs, FR_INT_ERR);
				}
				if (clst == 0xFFFFFFFF) {
					dprintf("read cluster is MAX!\r\n");
					ABORT(fp->fs, FR_DISK_ERR);
				}
				fp->curr_clust =
				    clst; /* Update current cluster */
				fp->csect =
				    0; /* Reset sector offset in the cluster */
			}
			sect = clust2sect(
			    fp->fs, fp->curr_clust); /* Get current sector */
			if (!sect) {
				dprintf("read sector is zero\r\n");
				ABORT(fp->fs, FR_INT_ERR);
			}
			sect += fp->csect;
			cc = btr / SS(
				       fp->fs); /* When remaining bytes >=
						   sector size, */
			if (cc) /* Read maximum contiguous sectors directly */
			{
				if (fp->csect + cc >
				    fp->fs
					->csize) /* Clip at cluster boundary */
					cc = fp->fs->csize - fp->csect;
				if (disk_read(fp->fs->drive, rbuff, sect,
					      (U8)cc,
					      fp->fs->diskhandle) != RES_OK) {
					dprintf("cannot read disk\r\n");
					ABORT(fp->fs, FR_DISK_ERR);
				}
				fp->csect += (U8)
				    cc; /* Next sector address in the cluster */
				rcnt = SS(fp->fs) *
				       cc; /* Number of bytes transferred */
				continue;
			}
			fp->dsect = sect;
			fp->csect++; /* Next sector address in the cluster */
		}
		rcnt = SS(fp->fs) - (fp->fptr %
				     SS(fp->fs)); /* Get partial sector data
						     from sector buffer */
		if (rcnt > btr)
			rcnt = btr;
		if (move_window(fp->fs, fp->dsect)) /* Move sector window */
		{
			dprintf("cannot read disk or data cache\r\n");
			ABORT(fp->fs, FR_DISK_ERR);
		}
		mem_cpy(rbuff, &fp->fs->win[fp->fptr % SS(fp->fs)],
			rcnt); /* Pick partial sector */
	}

	LEAVE_FF(fp->fs, FR_OK);
}

/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_close(FIL *fp /* Pointer to the file object to be closed */
		)
{
	FRESULT res;

	res = validate(fp->fs, fp->id);
	if (res == FR_OK)
		fp->fs = NULL;
	LEAVE_FF(fp->fs, res);
}
