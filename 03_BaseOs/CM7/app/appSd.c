#include "main.h"
#include "bsp.h"
#include "bsp_sdio_sd.h"
#include "ff.h"				/* FatFS文件系统模块*/
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"
#include "appSd.h"
#include "FATFS_drv.h"
#include "Pic_drv.h"

/* Private define ------------------------------------------------------------*/
/* 用于测试读写速度 */
#define TEST_FILE_LEN			(2*1024*1024)	/* 用于测试的文件长度 */
#define BUF_SIZE				(4*1024)		/* 每次读写SD卡的最大数据长度 */


/* Task variables ---------------------------------------------------------*/
const osThreadAttr_t ThreadSdMainProc_Attr =
{
    .name = "osThreadSdMainProc",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal2,
    .stack_size = 4096,
};

const osThreadAttr_t ThreadSdDetect_Attr =
{
    .name = "osThreadSdDetect",
    .attr_bits = osThreadDetached,
    .priority = osPriorityBelowNormal,
    .stack_size = 2048,
};

/* 任务句柄 */
osThreadId_t ThreadIdSdMainProc = NULL;
osThreadId_t ThreadIdSdDetect = NULL;

osEventFlagsId_t gEventId_sdapp = NULL;
// static osSemaphoreId_t  gSemaId_sdapp = NULL;

void AppTaskSdMainProc(void *argument);
void AppTaskSdDetect(void *argument);

/* Private variables ---------------------------------------------------------*/

__attribute__((section (".RAM_D1")))	FATFS fs;
__attribute__((section (".RAM_D1")))	FIL file;
ALIGN_32BYTES(__attribute__((section (".RAM_D1"))) char FsReadBuf[1024]);
ALIGN_32BYTES(__attribute__((section (".RAM_D1"))) char FsWriteBuf[1024]) = {"FatFS Write Demo \r\n 123123123CCAAA ##测试写入## \r\n"};
ALIGN_32BYTES(__attribute__((section (".RAM_D1"))) uint8_t g_TestBuf[BUF_SIZE]);


DIR DirInf;
FILINFO FileInf;
char DiskPath[4]; /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */

/* FatFs API的返回值 */
static const char *FR_Table[] =
{
    "FR_OK:成功",                                           /* (0) Succeeded */
    "FR_DISK_ERR:底层硬件错误",                             /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR:断言失败",                                  /* (2) Assertion failed */
    "FR_NOT_READY:物理驱动没有工作",                        /* (3) The physical drive cannot work */
    "FR_NO_FILE:文件不存在",                                /* (4) Could not find the file */
    "FR_NO_PATH:路径不存在",                                /* (5) Could not find the path */
    "FR_INVALID_NAME:无效文件名",                           /* (6) The path name format is invalid */
    "FR_DENIED:由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST:文件已经存在",                                /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT:文件或者目录对象无效",               /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED:物理驱动被写保护",                  /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE:逻辑驱动号无效",                      /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED:卷中无工作区",                          /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM:没有有效的FAT卷",                     /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED:由于参数错误f_mkfs()被终止",           /* (14) The f_mkfs() aborted due to any parameter error */
    "FR_TIMEOUT:在规定的时间内无法获得访问卷的许可",        /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED:由于文件共享策略操作被拒绝",                 /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE:无法分配长文件名工作区",            /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES:当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
    "FR_INVALID_PARAMETER:参数无效"                         /* (19) Given parameter is invalid */
};

static void appSd_remountFatfs(void);
static void appSd_showSdCardInfo(void);
static void appSd_viewRootDir(void);

static void appSd_CreateNewFile(void);
static void appSd_ReadFileData(void);
static void appSd_CreateDir(void);
static void appSd_DeleteAll(void);
static void appSd_SpeedTest(void);



uint16_t gulTotalPicNum = 0;    //目录下图片数量
uint16_t gusCurPicIdx = 0;		//图片当前索引
uint32_t *picoffsettbl;	//图片文件offset索引表 

void appSd_initPicShow(void);
void appSd_showPicture(void);

/**
  * @brief  task to deal with sd detect event for sd mount
  * @param  None
  * @retval None
  */
void AppTaskSdDetect(void *argument)
{
    (void)argument;
    uint32_t retFlags = 0;

    osThreadYield();

    printf("AppTaskSdDetect start \r\n");

    while (1)
    {
        retFlags = osEventFlagsWait(gEventId_sdapp,
                                    APPSD_EVT_SD_INSERT|APPSD_EVT_SD_REMOVE,
                                    osFlagsWaitAny,
                                    osWaitForever);

        printf("osEventFlagsWait flag = 0x%x\r\n", retFlags);

        if ((retFlags & APPSD_EVT_SD_INSERT) == APPSD_EVT_SD_INSERT)
        {
            // sd card reinsert, need to reinitialize bsp sd module
            printf("SD1 Connected    \r\n");
            appSd_remountFatfs();
        }
        else if ((retFlags & APPSD_EVT_SD_REMOVE) == APPSD_EVT_SD_REMOVE)
        {
            printf("SD1 Not Connected    \r\n");

            /* 卸载文件系统 */
            f_mount(NULL, DiskPath, 0);
        }

        osEventFlagsClear(gEventId_sdapp, retFlags);

        osDelay(20);
    }
}

/**
  * @brief  sd event task
  * @param  None
  * @retval None
  */
void AppTaskSdMainProc(void *argument)
{
    (void)argument;
    uint32_t retFlags = 0;
    uint32_t wait_flags = 0;
    FRESULT result;

    osThreadYield();

    printf("AppTaskSdMainProc start \r\n");

    /* 注册SD卡驱动 */
    FATFS_LinkDriver(&SD_Driver, DiskPath);

    result = f_mount(&fs, DiskPath, 0); /* Mount a logical drive */
    if (result != FR_OK)
    {
        printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
    }

    Pic_DrvInit();

    appSd_initPicShow();

    osEventFlagsSet(gEventId_sdapp, APPSD_EVT_INITDONE);

    while (1)
    {
        wait_flags = APPSD_EVT_INITDONE|APPSD_EVT_SHOW_ROOTDIR;

        // for test
        wait_flags |= APPSD_EVT_NEWFILE | APPSD_EVT_READFILE | APPSD_EVT_NEWDIR | APPSD_EVT_DELETEALL | APPSD_EVT_SPEED_TEST;

        wait_flags |= APPSD_EVT_SHOW_FRT_PIC | APPSD_EVT_SHOW_NEXT_PIC | APPSD_EVT_SHOW_PREV_PIC;

        retFlags = osEventFlagsWait(gEventId_sdapp, \
                                    wait_flags,     \
                                    osFlagsWaitAny, \
                                    osWaitForever);

        osEventFlagsClear(gEventId_sdapp, retFlags);

        if ((retFlags & APPSD_EVT_INITDONE) == APPSD_EVT_INITDONE)
        {
            appSd_showSdCardInfo();

            appSd_initPicShow();
        }

        if ((retFlags & APPSD_EVT_SHOW_ROOTDIR) == APPSD_EVT_SHOW_ROOTDIR)
        {
            appSd_viewRootDir();
        }

        if ((retFlags & APPSD_EVT_NEWFILE) == APPSD_EVT_NEWFILE)
        {
            appSd_CreateNewFile();
        }

        if ((retFlags & APPSD_EVT_READFILE) == APPSD_EVT_READFILE)
        {
            appSd_ReadFileData();
        }

        if ((retFlags & APPSD_EVT_NEWDIR) == APPSD_EVT_NEWDIR)
        {
            appSd_CreateDir();
        }

        if ((retFlags & APPSD_EVT_SPEED_TEST) == APPSD_EVT_SPEED_TEST)
        {
            appSd_SpeedTest();
        }

        if ((retFlags & APPSD_EVT_DELETEALL) == APPSD_EVT_DELETEALL)
        {
            appSd_DeleteAll();
        }

        if ((retFlags & APPSD_EVT_SHOW_FRT_PIC) == APPSD_EVT_SHOW_FRT_PIC)
        {
            gusCurPicIdx = 0;
            appSd_showPicture();
        }

        if ((retFlags & APPSD_EVT_SHOW_NEXT_PIC) == APPSD_EVT_SHOW_NEXT_PIC)
        {
            gusCurPicIdx++;
            appSd_showPicture();
        }

        if ((retFlags & APPSD_EVT_SHOW_PREV_PIC) == APPSD_EVT_SHOW_PREV_PIC)
        {
            if (gusCurPicIdx == 0)
            {
                gusCurPicIdx = gulTotalPicNum - 1;
            }
            else
            {
                gusCurPicIdx--;
            }
            appSd_showPicture();
        }

        osDelay(20);
    }
}

void appSd_CreateTask(void)
{
    gEventId_sdapp = osEventFlagsNew(NULL);
    // gSemaId_sdapp = osSemaphoreNew(0xFF, 0, NULL);

    ThreadIdSdMainProc = osThreadNew(AppTaskSdMainProc, NULL, &ThreadSdMainProc_Attr);
    ThreadIdSdDetect = osThreadNew(AppTaskSdDetect, NULL, &ThreadSdDetect_Attr);
}

/**
* @brief  BSP SD Error Callback.
* @param  None
* @retval None.
*/
void BSP_SD_ErrorCallback(void)
{
    printf("SP_SD_ErrorCallback: 0x%x\r\n", hsd_sdmmc.ErrorCode);
}

/**
* @brief  BSP SD Callback.
* @param  GPIO_Pin: Specifies the pin connected EXTI line
* @retval None.
*/
void BSP_SD_DetectCallback(uint32_t Status)
{
    /* Check if the SD card is plugged in the slot */
    if (Status == SD_PRESENT)
    {
        osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SD_INSERT);
    }
    else
    {
        osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SD_REMOVE);
    }
}


/*******************************************************************************************************/
/*                     sd application Functions implementations                                        */
/*******************************************************************************************************/


static void appSd_remountFatfs(void)
{
    FRESULT result;
    
    BSP_SD_Init();

    result = f_mount(&fs, DiskPath, 0);	/* Mount a logical drive */
    if (result != FR_OK)
    {
        printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
    }
    osEventFlagsSet(gEventId_sdapp, APPSD_EVT_INITDONE);
}


static void appSd_showSdCardInfo(void)
{
    FRESULT result;
    DWORD fre_clust, free_size, total_size;
    FATFS *pfs = &fs;
    uint32_t folderSize = 0;


    float total_sizeGB = 0.0;
    float free_sizeGB = 0.0;
    
    result = f_getfree(DiskPath, &fre_clust, &pfs);

    if (result == FR_OK)
    {
        total_size = ((pfs->n_fatent - 2) * pfs->csize / 2) / 1024;
        free_size = (fre_clust * pfs->csize / 2) / 1024;

        total_sizeGB = (float)(total_size / 1024);
        free_sizeGB = (float)(free_size / 1024);

        printf("total size %d MB (%.2f GB), free size %d MB (%.2f GB)\r\n", \
               (uint32_t)total_size, total_sizeGB, (uint32_t)free_size, free_sizeGB);
    }
    else
    {
        // printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
        FATFS_ErrorReport("f_getfree", result);
    }

    folderSize = FATFS_getFolderSize((uint8_t *)DiskPath);

    printf("root folder size: %d\r\n", folderSize);
#if 0
    FileDirScan_T pDirScan;

    pDirScan.file_name = (char**)AllocMemD1(FATFS_DIR_MAX_NUM * sizeof(char*));

    FATFS_scanDirectoryList(DiskPath, "txt", &pDirScan);
    printf("scan dir[%s] \".txt\" %d files:\r\n", DiskPath, pDirScan.file_num);
    for (uint8_t iloop = 0; iloop < pDirScan.file_num; iloop++)
    {
        printf("/ %s \r\n", pDirScan.file_name[iloop]);
    }

    FreeMemD1(pDirScan.file_name);
#endif

#if 0
    /* 打印卡速度信息 */
    if(hsd_sdmmc.SdCard.CardSpeed == CARD_NORMAL_SPEED)
    {
        printf("Normal Speed Card <12.5MB/S, MAX Clock < 25MHz, Spec Version 1.01\r\n");           
    }
    else if (hsd_sdmmc.SdCard.CardSpeed == CARD_HIGH_SPEED)
    {
        printf("High Speed Card <25MB/s, MAX Clock < 50MHz, Spec Version 2.00\r\n");            
    }
    else if (hsd_sdmmc.SdCard.CardSpeed == CARD_ULTRA_HIGH_SPEED)
    {
        printf("UHS-I SD Card <50MB/S for SDR50, DDR50 Cards, MAX Clock < 50MHz OR 100MHz\r\n");
        printf("UHS-I SD Card <104MB/S for SDR104, MAX Clock < 108MHz, Spec version 3.01\r\n");   
    }
#endif
}

static void appSd_viewRootDir(void)
{
    FRESULT result;
    uint32_t cnt = 0;
    FILINFO fno;

    result = f_mount(&fs, DiskPath, 0); /* Mount a logical drive */
    if (result != FR_OK)
    {
        printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
    }

    /* 打开根文件夹 */
    result = f_opendir(&DirInf, DiskPath); /* 如果不带参数，则从当前目录开始 */
    if (result != FR_OK)
    {
        printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
        return;
    }

    printf("属性        |  File Size | 短文件名 | 长文件名\r\n");
    for (cnt = 0; ;cnt++)
    {
        result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
        if (result != FR_OK || FileInf.fname[0] == 0)
        {
            break;
        }

        if (FileInf.fname[0] == '.')
        {
            continue;
        }

        /* 判断是文件还是子目录 */
        if (FileInf.fattrib & AM_DIR)
        {
            printf("(0x%02d)目录  ", FileInf.fattrib);
        }
        else
        {
            printf("(0x%02d)文件  ", FileInf.fattrib);
        }

        f_stat(FileInf.fname, &fno);
        
        /* 打印文件大小, 最大4G */
        printf(" %10d", (int)fno.fsize);

        printf("  %s\r\n", (char *)FileInf.fname);	/* 长文件名 */
    }
}

static void appSd_CreateNewFile(void)
{
    FRESULT result;
    uint32_t bw;
    char path[32];

    /* 打开文件 */
    sprintf(path, "%swritetest.txt", DiskPath);
    result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
    if (result == FR_OK)
    {
        printf("writetest.txt 文件打开成功\r\n");
    }
    else
    {
        printf("writetest.txt 文件打开失败  (%s)\r\n", FR_Table[result]);
    }

    /* 写一串数据 */
    result = f_write(&file, FsWriteBuf, strlen(FsWriteBuf), &bw);
    if (result == FR_OK)
    {
        printf("writetest.txt 文件写入成功\r\n");
    }
    else
    {
        printf("writetest.txt 文件写入失败  (%s)\r\n", FR_Table[result]);
    }

    /* 关闭文件*/
    f_close(&file);
}

static void appSd_ReadFileData(void)
{
    FRESULT result;
    uint32_t bw;
    char path[64];

    /* 打开文件 */
    sprintf(path, "%swritetest.txt", DiskPath);
    result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
    if (result !=  FR_OK)
    {
        printf("Don't Find File : writetest.txt\r\n");
        return;
    }

    /* 读取文件 */
    result = f_read(&file, FsReadBuf, sizeof(FsReadBuf), &bw);
    if (bw > 0)
    {
        FsReadBuf[bw] = 0;
        printf("\r\nwritetest.txt 文件内容 : \r\n%s\r\n", FsReadBuf);
    }
    else
    {
        printf("\r\nwritetest.txt 文件内容 : \r\n");
    }

    /* 关闭文件*/
    f_close(&file);
}

static void appSd_CreateDir(void)
{
    FRESULT result;
    char path[64];

    /* 创建目录/Dir1 */
    sprintf(path, "%sDir1", DiskPath);
    result = f_mkdir(path);
    if (result == FR_OK)
    {
        printf("f_mkdir Dir1 Ok\r\n");
    }
    else if (result == FR_EXIST)
    {
        printf("Dir1 目录已经存在(%d)\r\n", result);
    }
    else
    {
        printf("f_mkdir Dir1 失败 (%s)\r\n", FR_Table[result]);
        return;
    }

    /* 创建目录/Dir2 */
    sprintf(path, "%sDir2", DiskPath);
    result = f_mkdir(path);
    if (result == FR_OK)
    {
        printf("f_mkdir Dir2 Ok\r\n");
    }
    else if (result == FR_EXIST)
    {
        printf("Dir2 目录已经存在(%d)\r\n", result);
    }
    else
    {
        printf("f_mkdir Dir2 失败 (%s)\r\n", FR_Table[result]);
        return;
    }

    /* 创建子目录 /Dir1/Dir1_1	   注意：创建子目录Dir1_1时，必须先创建好Dir1 */
    sprintf(path, "%sDir1/Dir1_1", DiskPath);
    result = f_mkdir(path); /* */
    if (result == FR_OK)
    {
        printf("f_mkdir Dir1_1 成功\r\n");
    }
    else if (result == FR_EXIST)
    {
        printf("Dir1_1 目录已经存在 (%d)\r\n", result);
    }
    else
    {
        printf("f_mkdir Dir1_1 失败 (%s)\r\n", FR_Table[result]);
        return;
    }
}

static void appSd_DeleteAll(void)
{
    FRESULT result;
    uint8_t i;
    char path[64];

    /* 删除目录/Dir1 【因为还存在目录非空（存在子目录)，所以这次删除会失败】*/
    sprintf(path, "%sDir1", DiskPath);
    result = f_unlink(path);
    if (result == FR_OK)
    {
        printf("删除目录Dir1成功\r\n");
    }
    else if (result == FR_NO_FILE)
    {
        printf("没有发现文件或目录 :%s\r\n", "/Dir1");
    }
    else
    {
        printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
    }

    /* 先删除目录/Dir1/Dir1_1 */
    sprintf(path, "%sDir1/Dir1_1", DiskPath);
    result = f_unlink(path);
    if (result == FR_OK)
    {
        printf("删除子目录/Dir1/Dir1_1成功\r\n");
    }
    else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
    {
        printf("没有发现文件或目录 :%s\r\n", "/Dir1/Dir1_1");
    }
    else
    {
        printf("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
    }

    /* 先删除目录/Dir1 */
    sprintf(path, "%sDir1", DiskPath);
    result = f_unlink(path);
    if (result == FR_OK)
    {
        printf("删除目录Dir1成功\r\n");
    }
    else if (result == FR_NO_FILE)
    {
        printf("没有发现文件或目录 :%s\r\n", "/Dir1");
    }
    else
    {
        printf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
    }

    /* 删除目录/Dir2 */
    sprintf(path, "%sDir2", DiskPath);
    result = f_unlink(path);
    if (result == FR_OK)
    {
        printf("删除目录 Dir2 成功\r\n");
    }
    else if (result == FR_NO_FILE)
    {
        printf("没有发现文件或目录 :%s\r\n", "/Dir2");
    }
    else
    {
        printf("删除Dir2 失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
    }

    /* 删除文件 writetest.txt */
    sprintf(path, "%swritetest.txt", DiskPath);
    result = f_unlink(path);
    if (result == FR_OK)
    {
        printf("删除文件 writetest.txt 成功\r\n");
    }
    else if (result == FR_NO_FILE)
    {
        printf("没有发现文件或目录 :%s\r\n", "writetest.txt");
    }
    else
    {
        printf("删除writetest.txt失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
    }

    /* 删除文件 speed1.txt */
    for (i = 0; i < 20; i++)
    {
        sprintf(path, "%sSpeed%02d.txt", DiskPath, i); /* 每写1次，序号递增 */
        result = f_unlink(path);
        if (result == FR_OK)
        {
            printf("删除文件%s成功\r\n", path);
        }
        else if (result == FR_NO_FILE)
        {
            printf("没有发现文件:%s\r\n", path);
        }
        else
        {
            printf("删除%s文件失败(错误代码 = %d) 文件只读或目录非空\r\n", path, result);
        }
    }
}


static void appSd_SpeedTest(void)
{
    FRESULT result;
    char path[64];
    uint32_t bw;
    uint32_t i, k;
    uint32_t runtime1, runtime2, timelen;
    uint8_t err = 0;
    static uint8_t s_ucTestSn = 0;

    for (i = 0; i < sizeof(g_TestBuf); i++)
    {
        g_TestBuf[i] = (i / 512) + '0';
    }

    /* 挂载文件系统 */
    result = f_mount(&fs, DiskPath, 0); /* Mount a logical drive */
    if (result != FR_OK)
    {
        printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
    }

    /* 打开文件 */
    sprintf(path, "%sSpeed%02d.txt", DiskPath, s_ucTestSn++); /* 每写1次，序号递增 */
    result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);

    /* 写一串数据 */
    printf("开始写文件%s %dKB ...\r\n", path, TEST_FILE_LEN / 1024);

    runtime1 = bsp_GetRunTime(); /* 读取系统运行时间 */
    for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
    {
        result = f_write(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
        if (result == FR_OK)
        {
            if (((i + 1) % 8) == 0)
            {
                printf(".");
            }
        }
        else
        {
            err = 1;
            printf("%s文件写失败\r\n", path);
            break;
        }
    }
    runtime2 = bsp_GetRunTime(); /* 读取系统运行时间 */

    if (err == 0)
    {
        timelen = (runtime2 - runtime1);
        printf("\r\n  写耗时 : %dms   平均写速度 : %dB/S (%dKB/S)\r\n",
               timelen,
               (TEST_FILE_LEN * 1000) / timelen,
               ((TEST_FILE_LEN / 1024) * 1000) / timelen);
    }

    f_close(&file); /* 关闭文件*/

    /* 开始读文件测试 */
    result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
    if (result != FR_OK)
    {
        printf("没有找到文件: %s\r\n", path);
        return;
    }

    printf("开始读文件 %dKB ...\r\n", TEST_FILE_LEN / 1024);

    runtime1 = bsp_GetRunTime(); /* 读取系统运行时间 */
    for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
    {
        result = f_read(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
        if (result == FR_OK)
        {
            if (((i + 1) % 8) == 0)
            {
                printf(".");
            }

            /* 比较写入的数据是否正确，此语句会导致读卡速度结果降低到 3.5MBytes/S */
            for (k = 0; k < sizeof(g_TestBuf); k++)
            {
                if (g_TestBuf[k] != (k / 512) + '0')
                {
                    err = 1;
                    printf("Speed1.txt 文件读成功，但是数据出错\r\n");
                    break;
                }
            }
            if (err == 1)
            {
                break;
            }
        }
        else
        {
            err = 1;
            printf("Speed1.txt 文件读失败\r\n");
            break;
        }
    }

    runtime2 = bsp_GetRunTime(); /* 读取系统运行时间 */

    if (err == 0)
    {
        timelen = (runtime2 - runtime1);
        printf("\r\n  读耗时 : %dms   平均读速度 : %dB/S (%dKB/S)\r\n", timelen,
               (TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
    }

    /* 关闭文件*/
    f_close(&file);
}


void appSd_initPicShow(void)
{
    uint8_t res;

    DIR picdir;	 		//图片目录
    FILINFO *picfileinfo;//文件信息 
    uint8_t *pname;      //带路径的文件名
    uint16_t temp;

    gulTotalPicNum = FATFS_getPictureNum("0:/PICTURE");

    if (gulTotalPicNum > 0)
    {
        picfileinfo = AllocMemD1(sizeof(FILINFO));
        pname = AllocMemD1(_MAX_LFN * 2 + 1);     // 为带路径的文件名分配内存
        picoffsettbl = AllocMemD1(4 * gulTotalPicNum); // 申请4*totpicnum个字节的内存,用于存放图片索引

        if (!picfileinfo||!pname||!picoffsettbl)
        {
            printf("内存分配失败 \r\n");
            return ;
        }

        // 记录索引
        res = f_opendir(&picdir, "0:/PICTURE"); // 打开目录
        if (res == FR_OK)
        {
            gusCurPicIdx = 0; // 当前索引为0
            while (1)     // 全部查询一遍
            {
                temp = picdir.dptr;                    // 记录当前dptr偏移
                res = f_readdir(&picdir, picfileinfo); // 读取目录下的一个文件
                if (res != FR_OK || picfileinfo->fname[0] == 0)
                    break; // 错误了/到末尾了,退出
                res = FATFS_getFileType((uint8_t *)picfileinfo->fname);
                if ((res & 0xF0) == 0x50) // 取高四位,看看是不是图片文件
                {
                    picoffsettbl[gusCurPicIdx] = temp; // 记录索引
                    gusCurPicIdx++;
                }
            }

            res = f_closedir(&picdir);
        }

        FreeMemD1(picfileinfo);
        FreeMemD1(pname);
    }

    Pic_DrvInit();
}

void appSd_showPicture(void)
{
    uint8_t res;

    DIR picdir;	 		//图片目录
    FILINFO *picfileinfo;//文件信息 
    uint8_t *pname;      //带路径的文件名

    if (gulTotalPicNum > 0)
    {
        picfileinfo = AllocMemD1(sizeof(FILINFO));
        pname = AllocMemD1(_MAX_LFN * 2 + 1);       // 为带路径的文件名分配内存

        if (!picfileinfo || !pname || !picoffsettbl)
        {
            printf("内存分配失败 \r\n");
            return;
        }

        res = f_opendir(&picdir, (const TCHAR *)"0:/PICTURE"); // 打开目录

        if (gusCurPicIdx >= gulTotalPicNum)
        {
            gusCurPicIdx = 0;
        }

        if (res == FR_OK) // 打开成功
        {
            dir_sdi(&picdir, picoffsettbl[gusCurPicIdx]); // 改变当前目录索引

            res = f_readdir(&picdir, picfileinfo); // 读取目录下的一个文件

            if (res != FR_OK || picfileinfo->fname[0] == 0)
            {
                printf("未找到图片 \r\n");
                FATFS_ErrorReport("f_readdir", (FRESULT)res);
                return; // 错误了/到末尾了,退出
            }
            strcpy((char *)pname, "0:/PICTURE/");                    // 复制路径(目录)
            strcat((char *)pname, (const char *)picfileinfo->fname); // 将文件名接在后面
            LCD_UTIL_Clear(LCD_UTIL_COLOR_LIGHTGRAY);
            LCD_UTIL_SetBackColor(0xFFFFFFFFUL);
            Pic_loadPictureFile(pname, 0, 0, 800, 480, 1);                                                // 显示图片
            LCD_UTIL_DisplayString(2, 2, FONTSIZE_16, LCD_UTIL_COLOR_BLUE, (uint8_t*)picfileinfo->fname, LEFT_MODE); // 显示图片名字
        }

        res = f_closedir(&picdir);

        FreeMemD1(picfileinfo);
        FreeMemD1(pname);
    }
    else
    {
        LCD_UTIL_DisplayString(2, 2, FONTSIZE_16, LCD_UTIL_COLOR_RED, "not find any pictures", LEFT_MODE);
    }
}

