# 文件模拟FatFs

## 概述

因为最近在忙关于FatFs的项目，但是之前没有接触过FatFs，听说FatFs入门就是用文件模拟文件系统，所以分享一下自己的经历。

移植用到的FatFs版本是FF14，网址：[http://elm-chan.org/fsw/ff/00index_e.html](http://elm-chan.org/fsw/ff/00index_e.html).

在移植FatFs时，我们只需要对其中几个文件进行修改，分别是：

- ffconf.h: FatFs的配置
- diskio.c: 实现读写等操作的文件，也是我们主要进行修改的文件

## 移植过程

### ffconf.h

此项目主要是将```FF_USE_MKFS```设置成1，使程序能够使用f_mkfs()函数对FatFs进行格式化。

如果想要支持长文件名（默认只支持短文件名，文件名8为加后缀3位，具体长度不确定了，有点忘记了），需要将```FF_USE_LFN```设置为1，同时要在项目中包含```ffunicode.c```.


### diskio.c

diskio.c是主要修改的文件，按照函数功能分类主要包含以下几个函数：

- 存储设备控制
  - disk_status： 获取设备状态
  - disk_initialize： 初始化设备
  - disk_read： 读数据
  - disk_write： 写数据
  - disk_ioctl： 控制设备相关功能
- 实时时钟
  - get_fattime： 获取当前时间

下面开始撸代码：

#### 一些设置的全局变量

因为是文件模拟FatFs，所以文件是根本，需要设置一个文件的全局变量，以供下面那些函数进行使用。同时，需要用一个宏定义设置扇区大小。

```C
#define SECTOR_SIZE 512
static HANDLE fp_fatfs = NULL;
```

#### disk_status函数

查询状态的函数，当文件句柄为NULL时返回没有初始化，其他时间都返回成功即可。

```C
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if (NULL == fp_fatfs)
		return STA_NOINIT;
	return RES_OK;
}
```

#### disk_initialize函数

初始化FatFs，在这个函数里面创建文件。

```C
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
```

#### disk_read函数

读取数据的函数，需要将文件指针跳转到正确的位置，读取指定数量的数据。

```C
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
```

#### disk_write函数

写入数据的函数，需要将文件指针跳转到正确的位置，写入指定数量的数据。

```C
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
```

#### disk_ioctl函数

设置一些FatFs的基本信息，这里有几个需要注意的地方，如果要设置文件系统大小，则要指定```GET_SECTOR_COUNT```的值，计算方法如下：扇区数量 = 总容量 / 扇区大小。

因此，如果设置2G的文件系统的话，```GET_SECTOR_COUNT```的值应该为：```2 * 1024 * 1024 * 1024 / 512```。

```C
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
```

#### get_fattime函数

这个函数是照扒别人的，如果使用这个函数，需要```#include <time.h>```。

```C
DWORD get_fattime(void)
{
	time_t timep;
	struct tm* p;
	time(&timep);
	p = localtime(&timep);
	return (((DWORD)(p->tm_year - 80) << 25) | ((DWORD)(1 + p->tm_mon) << 21) | ((DWORD)(p->tm_mday) << 16) |
		((DWORD)(p->tm_hour) << 11) | ((DWORD)(p->tm_min) << 5) | ((DWORD)(p->tm_sec) >> 1));
}
```

到这里，移植已经完成。下面写个文件进行测试。

### 测试文件

```C
#include <iostream>
#include "ff.h"


using namespace std;

int FatFsTest()
{
    FATFS fatfs;
    FATFS* fs = &fatfs;
    FRESULT res;
    FIL fil;
    UINT bw = 0;

    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    MKFS_PARM mkfs = {
        FM_FAT32,
        0,
        0,
        0,
        0
    };


    //cout << "----------------------------------------------------------------" << endl;

    //res = f_mount(fs, "0:", 0);

    //if (res != FR_OK)
    //{
    //    cout << "mount failed! res: " << res << endl;
    //    //return -1;
    //}

    //cout << "----------------------------------------------------------------" << endl;

    ////MKFS_PARM opt = { FM_FAT32 };
    //BYTE Buff[FF_MAX_SS];			/* Working buffer */

    ////res = f_mkfs("0:", 0, 1);
    res = f_mkfs("0:", &mkfs, work, sizeof work);
    ////res = f_mkfs("0:", 0, work, sizeof work);

    if (res != FR_OK)
    {
        cout << "f_mkfs failed! res: " << res << endl;
        return -1;
    }

    cout << "----------------------------------------------------------------" << endl;

    res = f_mount(fs, "0:", 1);

    if (res != FR_OK)
    {
        cout << "mount failed! res: " << res << endl;
        //return -1;
    }

    cout << "----------------------------------------------------------------" << endl;

    res = f_open(&fil, "0:\\test.txt", FA_CREATE_NEW | FA_WRITE);
    if (res != FR_OK)
    {
        cout << "open failed! res: " << res << endl;
        return -2;
    }

    const char* str_write = "Hello From FATFS";
    //const char* str_write2 = "str_write2";

    res = f_write(&fil, str_write, strlen(str_write), &bw);
    if (res != FR_OK)
    {
        cout << "write failed! res: " << res << endl;
        return -3;
    }
    cout << "Write Num: " << bw << endl;

    f_close(&fil);


    cout << "----------------------------------------------------------------" << endl;


    res = f_open(&fil, "0:\\test.txt", FA_READ);
    if (res != FR_OK)
    {
        cout << "open failed! res: " << res << endl;
        return -4;
    }

    char str_read[100] = { 0 };

    res = f_read(&fil, str_read, 100, &bw);
    if (res != FR_OK)
    {
        cout << "read failed! res: " << res << endl;
        return -5;
    }
    cout << "Read Num: " << bw << endl;

    cout << "str_read: " << str_read << endl;
    f_close(&fil);


    cout << "----------------------------------------------------------------" << endl;

    DWORD nclst = 0;
    f_getfree("0:", &nclst, &fs);
    cout << "f_getfree nclst == " << nclst << endl;

    cout << "----------------------------------------------------------------" << endl;

    return 0;
}


int main(int argc, char** argv)
{
    std::cout << "Hello World!\n";

    int Code = 0;

    Code = FatFsTest();
    //Code = test();

    cout << "Code: " << Code << endl;
}
```

## 结尾

程序到这已经结束，如果有不对的地方，还请大神多多指正。

