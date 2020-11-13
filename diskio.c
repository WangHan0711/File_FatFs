/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include <time.h>
#pragma warning(disable:4996)

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


#define SECTOR_SIZE 512


static HANDLE fp_fatfs = NULL;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if (NULL == fp_fatfs)
		return STA_NOINIT;
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	const char* FatFsFile = "E:\\Code\\VS_Project\\FatFsTest\\FF14Test1\\FF14Test1\\FatFs_Space";

	if (NULL == fp_fatfs)
	{
		fp_fatfs = CreateFile(FatFsFile,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_FLAG_NO_BUFFERING,
			NULL);
		if (fp_fatfs == INVALID_HANDLE_VALUE)
			return STA_NOINIT;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	DWORD rdBytes = 0;

	SetFilePointer(fp_fatfs, sector * SECTOR_SIZE, NULL, FILE_BEGIN);

	if (!ReadFile(fp_fatfs, buff, count * 512UL, &rdBytes, NULL))
	{
		return RES_ERROR;
	}


	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	DWORD rdBytes = 0;

	SetFilePointer(fp_fatfs, sector * SECTOR_SIZE, NULL, FILE_BEGIN);

	if (!WriteFile(fp_fatfs, buff, count * 512UL, &rdBytes, NULL))
	{
		return RES_ERROR;
	}


	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	res = RES_PARERR;
	switch (cmd) {
	case CTRL_SYNC:			/* Nothing to do */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:	/* Get number of sectors（扇区） on the drive */
		*(DWORD*)buff = 4 * 1024 * 1024;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE:	/* Get size of sector for generic read/write */
		*(WORD*)buff = FF_MAX_SS;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE:	/* Get internal block size in unit of sector （以扇区为单位获取内部块大小）*/
		*(DWORD*)buff = 1;
		res = RES_OK;
		break;
	}

	return res;
}


DWORD get_fattime(void)
{
	time_t timep;
	struct tm* p;
	time(&timep);
	p = localtime(&timep);
	return (((DWORD)(p->tm_year - 80) << 25) | ((DWORD)(1 + p->tm_mon) << 21) | ((DWORD)(p->tm_mday) << 16) |
		((DWORD)(p->tm_hour) << 11) | ((DWORD)(p->tm_min) << 5) | ((DWORD)(p->tm_sec) >> 1));
}

