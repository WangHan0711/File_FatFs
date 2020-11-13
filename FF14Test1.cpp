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



