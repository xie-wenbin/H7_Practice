#include "FATFS_drv.h"
#include "bsp_memory.h"
#include "sd_diskio_dma.h"
#include "utils.h"

/* Private define ------------------------------------------------------------*/
#define FILE_TYPE_MAX_NUM 7      // 最多个大类
#define FILE_TYPE_SUBITEM_NUM 4  // 最多个子类



/* Private variables ---------------------------------------------------------*/
// 文件类型列表
uint8_t *const FILE_TYPE_TBL[FILE_TYPE_MAX_NUM][FILE_TYPE_SUBITEM_NUM] =
{
    {"BIN"},                       // BIN文件
    {"LRC"},                       // LRC文件
    {"NES", "SMS"},                // NES/SMS文件
    {"TXT", "C", "H"},             // 文本文件
    {"WAV", "MP3", "APE", "FLAC"}, // 支持的音乐文件
    {"BMP", "JPG", "JPEG", "GIF"}, // 图片文件
    {"AVI"},                       // 视频文件
};

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


/* Public extended variables ---------------------------------------------------------*/
__attribute__((section (".RAM_D1"))) FATFS gFs[_VOLUMES]; // 逻辑磁盘工作区.
__attribute__((section (".RAM_D1"))) FIL gCurFatFile;     // 文件1

char gDiskPath[_VOLUMES][4]; /* 逻辑驱动路径，比盘符0，就是"0:/" */
FIL *ftemp;          // 文件2
UINT br, bw;         // 读写变量
FILINFO gFileInfo;   // 文件信息
DIR gDirInfo;        // 目录



/* Public extended functions ---------------------------------------------------------*/

void FATFS_diskInitialize(void)
{
    FRESULT result;
    /* 注册SD卡驱动 */
    FATFS_LinkDriver(&SD_Driver, gDiskPath[FATFS_DEV_MMCSD_0]);

    result = f_mount(&gFs[FATFS_DEV_MMCSD_0], gDiskPath[FATFS_DEV_MMCSD_0], 0); /* Mount a logical drive */
    if (result != FR_OK)
    {
        printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
    }
}

FATFS *FATFS_getFsHandler(uint8_t devid)
{
    if (devid < _VOLUMES)
        return &gFs[devid];
    else
        return NULL;
}

void FATFS_ErrorReport(const char * api_name, FRESULT result)
{
    printf("%s:[%s]\r\n", api_name, FR_Table[result]);
}

/**
 * @brief 获取磁盘剩余空间
 * 
 * @param drvPath 磁盘路径， 如("0:")
 * @param total 总大小 (KB)
 * @param free 剩余大小 (KB)
 * @return uint8_t FRESULT
 */
uint8_t FATFS_getFree(uint8_t *drvPath, uint32_t *total, uint32_t *free)
{
    FRESULT result;
    DWORD fre_clust, free_sect, total_sect;
    FATFS *pfs;

    result = f_getfree((const TCHAR*)drvPath, &fre_clust, &pfs);

    if (result == FR_OK)
    {
        total_sect = (pfs->n_fatent - 2) * pfs->csize;
        free_sect = fre_clust * pfs->csize;
#if _MAX_SS != 512 // 扇区大小不是512字节,则转换为512字节
        total_sect *= fs1->ssize / 512;
        free_sect *= fs1->ssize / 512;
#endif
        *total = total_sect >> 1; // 单位为KB
        *free = free_sect >> 1;  // 单位为KB
    }
    else
    {
        printf("(%s)\r\n", FR_Table[result]);
    }

    return result;
}

/**
 * @brief 获取文件类型
 * 
 * @param fname 文件名
 * @return uint8_t 0xFF - 无法识别的文件名
 *                 其它 - 高4位代表所属文件大类，低4位表示所属子类，文件类型定义见头文件
 */
uint8_t FATFS_getFileType(uint8_t *fname)
{
    uint8_t tbuf[5];
    uint8_t *attr = '\0'; // 后缀名
    uint8_t i = 0, j;
    while (i < 250)
    {
        i++;
        if (*fname == '\0')
            break; // 偏移到了最后了.
        fname++;
    }
    if (i == 250)
        return 0XFF;        // 错误的字符串.
    for (i = 0; i < 5; i++) // 得到后缀名
    {
        fname--;
        if (*fname == '.')
        {
            fname++;
            attr = fname;
            break;
        }
    }
    strcpy((char *)tbuf, (const char *)attr); // copy
    for (i = 0; i < 4; i++)
        tbuf[i] = char_upper(tbuf[i]);      // 全部变为大写
    for (i = 0; i < FILE_TYPE_MAX_NUM; i++) // 大类对比
    {
        for (j = 0; j < FILE_TYPE_SUBITEM_NUM; j++) // 子类对比
        {
            if (*FILE_TYPE_TBL[i][j] == 0)
                break;                                                              // 此组已经没有可对比的成员了.
            if (strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0) // 找到了
            {
                return (i << 4) | j;
            }
        }
    }
    return 0XFF; // 没找到
}

/**
 * @brief 拷贝文件, 注意文件大小不要超过4GB
 *        将 pSrc文件内容拷贝到 pDst文件
 * 
 * @param pSrc 源文件
 * @param pDst 目标文件
 * @param totalSize 文件大小(0 代表单个文件拷贝)
 * @param cpdSize 已拷贝大小
 * @param fw_mode 文件写入模式 (0 不覆盖原有文件, 1 覆盖原有文件)
 * @param showmsg 显示拷贝时的信息
 * @return uint8_t (0: OK, others: error)
 */
uint8_t FATFS_fileCopy(uint8_t *pSrc, uint8_t *pDst, uint32_t totalSize, uint32_t cpdSize, uint8_t fw_mode, fscpymsg showmsg)
{
    uint8_t res;
    uint16_t br = 0;
    uint16_t bw = 0;
    FIL *fsrc = 0;
    FIL *fdst = 0;
    uint8_t *fbuf = 0;
    uint8_t curpct = 0;
    unsigned long long lcpdsize = cpdSize;

    fsrc = (FIL *)AllocMemD1(sizeof(FIL)); // 申请内存
    fdst = (FIL *)AllocMemD1(sizeof(FIL));
    fbuf = (uint8_t *)AllocMemD1(8192);

    if (fsrc == NULL || fdst == NULL || fbuf == NULL)
    {
        res = 100; // 前面的值留给fatfs
    }
    else
    {
        if (fw_mode == 0)
            fw_mode = FA_CREATE_NEW; // 不覆盖
        else
            fw_mode = FA_CREATE_ALWAYS; // 覆盖存在的文件

        res = f_open(fsrc, (const TCHAR *)pSrc, FA_READ | FA_OPEN_EXISTING); // 打开只读文件

        if (0 == res)
            res = f_open(fdst, (const TCHAR *)pDst, FA_WRITE | fw_mode); // 第一个打开成功,才开始打开第二个

        if (0 == res)                                                    // 两个都打开成功了
        {
            if (totalSize == 0) // 仅仅是单个文件复制
            {
                totalSize = fsrc->obj.objsize;
                lcpdsize = 0;
                curpct = 0;
            }
            else
            {
                curpct = (lcpdsize * 100) / totalSize; // 得到新百分比
            }

            showmsg(pSrc, curpct, 0x02);               // 更新百分比

            while (0 == res)                           // 开始复制
            {
                res = f_read(fsrc, fbuf, 8192, (UINT *)&br); // 源头读出512字节
                if (res || br == 0)
                    break;
                res = f_write(fdst, fbuf, (UINT)br, (UINT *)&bw); // 写入目的文件
                lcpdsize += bw;
                if (curpct != (lcpdsize * 100) / totalSize) // 是否需要更新百分比
                {
                    curpct = (lcpdsize * 100) / totalSize;
                    if (showmsg(pSrc, curpct, 0x02)) // 更新百分比
                    {
                        res = 0XFF; // 强制退出
                        break;
                    }
                }
                if (res || bw < br)
                    break;
            }

            f_close(fsrc);
            f_close(fdst);
        }
    }

    FreeMemD1(fsrc); // 释放内存
    FreeMemD1(fdst);
    FreeMemD1(fbuf);
    
    return res;
}

/**
 * @brief 扫描路径下特定后缀的文件
 * @attention 传入的文件列表指针，在使用后续释放内存
 * 
 * @param dir_path 文件夹路径，如 "0:/App"
 * @param file_suffix 文件后缀，如 "txt", 若输入为 "*.__"，则扫描目录下所有带后缀的文件
 * @param scanfile 已识别的文件列表变量指针
 * @return FRESULT 0 - success
 */
FRESULT FATFS_scanDirectoryList(const char *dir_path, const char *file_suffix, FileDirScan_T *scanfile)
{
    FRESULT res;
    FILINFO *fileinfo;
    DIR *dir;

    dir = (DIR*)AllocMemD1(sizeof(DIR));
    fileinfo = (FILINFO*)AllocMemD1(sizeof(FILINFO));
    // scanfile->file_name = (char**)AllocMemD1(FATFS_DIR_MAX_NUM * sizeof(char*));
    if ((dir == NULL) || (fileinfo == NULL) || (scanfile == NULL))
        return FR_INVALID_PARAMETER;

    scanfile->file_num = 0;

    res = f_opendir(dir, dir_path);

    if (res == FR_OK)
    {
        while (1)
        {
            char *pstr;
            res = f_readdir(dir, fileinfo); // 读取目录下的一个文件
            if (res != FR_OK || fileinfo->fname[0] == 0 || scanfile->file_num > FATFS_DIR_MAX_NUM)
            {
                break; // 错误了/到末尾了,退出
            }

            if (fileinfo->fattrib & AM_DIR)
                continue;                         // 跳过目录
            
            pstr = strrchr(fileinfo->fname, '.'); // 从右向左查找'.'的位置

            if (pstr == NULL)
                continue; // 无后缀名的跳过
            
            pstr++;       // 去掉'.'

            if (strcasecmp(pstr, file_suffix) == 0 || (file_suffix[0] == '*'))
            {
            #if (_USE_LFN != 0)
                uint8_t fname_len = 0;
                fname_len = strlen(fileinfo->fname);

                scanfile->file_name[scanfile->file_num] = (char *)AllocMemD1(fname_len + 1);
                strcpy(scanfile->file_name[scanfile->file_num], fileinfo->fname);
            #else
                scanfile->file_name[scanfile->file_num] = (char *)AllocMemD1(13);
                strcpy(scanfile->file_name[scanfile->file_num], fileinfo->fname);
            #endif
                scanfile->file_num++;
            }
        }
    }

    f_closedir(dir);

    FreeMemD1(dir);
    FreeMemD1(fileinfo);

    return res;
}

/**
 * @brief 获取输入路径下的文件夹名称
 * 
 * @param pDfn 输入路径
 * @return uint8_t* 返回0代表输入路径为根路径
 *                  其它 文件夹名字首地址
 */
uint8_t *FATFS_getSrcDirName(uint8_t *pDfn)
{
    uint16_t temp = 0;
    while(*pDfn != 0)
    {
        pDfn++;
        temp++;
    }

    // 路径长度小于卷标名长度，返回0代表卷标路径
    if (temp < 4)
        return 0;
    
    // 追溯到倒数第一个 "\" 或 "/" 处
    while ((*pDfn != 0x5C) && (*pDfn != 0x2F))
    {
        pDfn--;
    }

    return ++pDfn;
}

#define MAX_PATHNAME_DEPTH	512+1	//最大目标文件路径+文件名深度

/**
 * @brief 获取文件夹大小 注意不要超过4GB
 * 
 * @param pfdname 文件夹路径
 * @return uint32_t 0 - 文件夹大小为0 或者读取过程中出现错误
 */
uint32_t FATFS_getFolderSize(uint8_t *pfdname)
{
    uint8_t res = 0;
    DIR *fddir = 0;        // 目录
    FILINFO *finfo = 0;    // 文件信息
    uint8_t *pathname = 0; // 目标文件夹路径+文件名
    uint16_t pathlen = 0;  // 目标路径长度
    uint32_t fdsize = 0;

    fddir = (DIR *)AllocMemD1(sizeof(DIR)); // 申请内存
    finfo = (FILINFO *)AllocMemD1(sizeof(FILINFO));
    if (fddir == NULL || finfo == NULL)
        res = 100;

    if (0 == res)
    {
        pathname = AllocMemD1(MAX_PATHNAME_DEPTH);

        if (pathname == NULL)
            res = 101;

        if (0 == res)
        {
            pathname[0] = 0;
            strcat((char *)pathname, (const char *)pfdname); // 复制路径
            res = f_opendir(fddir, (const TCHAR *)pfdname);  // 打开源目录
            if (0 == res)                                    // 打开目录成功
            {
                while (0 == res) // 开始复制文件夹里面的东东
                {
                    res = f_readdir(fddir, finfo); // 读取目录下的一个文件

                    if (res != FR_OK || finfo->fname[0] == 0)
                        break; // 错误了/到末尾了,退出

                    if (finfo->fname[0] == '.')
                        continue; // 忽略上级目录

                    if (0 == strcmp(finfo->fname, "System Volume Information"))
                        continue; // 忽略系统默认目录

                    if (finfo->fattrib & 0X10) // 是子目录(文件属性,0X20,归档文件;0X10,子目录;)
                    {
                        pathlen = strlen((const char *)pathname);             // 得到当前路径的长度
                        strcat((char *)pathname, (const char *)"/");          // 加斜杠
                        strcat((char *)pathname, (const char *)finfo->fname); // 源路径加上子目录名字
                        // printf("\r\nsub folder:%s\r\n",pathname);	//打印子目录名
                        fdsize += FATFS_getFolderSize(pathname); // 得到子目录大小,递归调用
                        pathname[pathlen] = 0;                   // 加入结束符
                    }
                    else
                    {
                        fdsize += finfo->fsize; // 非目录,直接加上文件的大小
                    }
                }

                res = f_closedir(fddir);
            }
            FreeMemD1(pathname);
        }
    }
    FreeMemD1(fddir);
    FreeMemD1(finfo);

    if (res)
        return 0;
    else
        return fdsize;
}

/**
 * @brief 文件夹复制 注意文件夹大小不要超过4GB
 *        将pSrc文件夹，copy到pDst文件夹
 * 
 * @param pSrc 源文件夹
 * @param pDst 目标文件夹
 * @param totalsize 总大小(当为 0 时，仅为单个文件拷贝)
 * @param cpdsize 已复制大小
 * @param fw_mode 文件写入模式
 *                0 - 不覆盖原有文件
 *                1 - 覆盖原有文件
 * @param showmsg 显示拷贝时信息
 * @return uint8_t 0 - 成功, 其它 - error, 0xff - 强制退出
 */
uint8_t FATFS_folderCopy(uint8_t *pSrc, uint8_t *pDst, uint32_t *totalsize, uint32_t *cpdsize,
                         uint8_t fw_mode, fscpymsg showmsg)
{
    uint8_t res = 0;
    DIR *srcdir = 0;    // 源目录
    DIR *dstdir = 0;    // 源目录
    FILINFO *finfo = 0; // 文件信息
    uint8_t *fn = 0;    // 长文件名

    uint8_t *dstpathname = 0; // 目标文件夹路径+文件名
    uint8_t *srcpathname = 0; // 源文件夹路径+文件名

    uint16_t dstpathlen = 0; // 目标路径长度
    uint16_t srcpathlen = 0; // 源路径长度

    srcdir = (DIR *)AllocMemD1(sizeof(DIR)); // 申请内存
    dstdir = (DIR *)AllocMemD1(sizeof(DIR));
    finfo = (FILINFO *)AllocMemD1(sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)
        res = 100;

    if (res == 0)
    {
        dstpathname = AllocMemD1(MAX_PATHNAME_DEPTH);
        srcpathname = AllocMemD1(MAX_PATHNAME_DEPTH);
        if (dstpathname == NULL || srcpathname == NULL)
            res = 101;
        if (res == 0)
        {
            dstpathname[0] = 0;
            srcpathname[0] = 0;
            strcat((char *)srcpathname, (const char *)pSrc); // 复制原始源文件路径
            strcat((char *)dstpathname, (const char *)pDst); // 复制原始目标文件路径
            res = f_opendir(srcdir, (const TCHAR *)pSrc);    // 打开源目录
            if (res == 0)                                    // 打开目录成功
            {
                strcat((char *)dstpathname, (const char *)"/"); // 加入斜杠

                fn = FATFS_getSrcDirName(pSrc);

                if (fn == 0) // 卷标拷贝
                {
                    dstpathlen = strlen((const char *)dstpathname);
                    dstpathname[dstpathlen] = pSrc[0]; // 记录卷标
                    dstpathname[dstpathlen + 1] = 0;   // 结束符
                }
                else
                {
                    strcat((char *)dstpathname, (const char *)fn); // 加文件名
                }

                showmsg(fn, 0, 0X04); // 更新文件夹

                res = f_mkdir((const TCHAR *)dstpathname); // 如果文件夹已经存在,就不创建.如果不存在就创建新的文件夹.

                if (res == FR_EXIST)
                    res = 0;

                while (res == 0) // 开始复制文件夹里面的东东
                {
                    res = f_readdir(srcdir, finfo); // 读取目录下的一个文件

                    if (res != FR_OK || finfo->fname[0] == 0)
                        break; // 错误了/到末尾了,退出

                    if (finfo->fname[0] == '.')
                        continue; // 忽略上级目录

                    if (0 == strcmp(finfo->fname, "System Volume Information"))
                        continue; // 忽略系统默认目录

                    fn = (uint8_t *)finfo->fname;                   // 得到文件名
                    dstpathlen = strlen((const char *)dstpathname); // 得到当前目标路径的长度
                    srcpathlen = strlen((const char *)srcpathname); // 得到源路径长度

                    strcat((char *)srcpathname, (const char *)"/"); // 源路径加斜杠

                    if (finfo->fattrib & 0X10) // 是子目录(文件属性,0X20,归档文件;0X10,子目录;)
                    {
                        strcat((char *)srcpathname, (const char *)fn);                                          // 源路径加上子目录名字
                        res = FATFS_folderCopy(srcpathname, dstpathname, totalsize, cpdsize, fw_mode, showmsg); // 拷贝文件夹
                    }
                    else // 非目录
                    {
                        strcat((char *)dstpathname, (const char *)"/");                                           // 目标路径加斜杠
                        strcat((char *)dstpathname, (const char *)fn);                                            // 目标路径加文件名
                        strcat((char *)srcpathname, (const char *)fn);                                            // 源路径加文件名
                        showmsg(fn, 0, 0X01);                                                                     // 更新文件名
                        res = FATFS_fileCopy(srcpathname, dstpathname, *totalsize, *cpdsize, fw_mode, showmsg); // 复制文件
                        *cpdsize += finfo->fsize;                                                                 // 增加一个文件大小
                    }

                    srcpathname[srcpathlen] = 0; // 加入结束符
                    dstpathname[dstpathlen] = 0; // 加入结束符
                }
            }
            FreeMemD1(dstpathname);
            FreeMemD1(srcpathname);
        }
    }
    FreeMemD1(srcdir);
    FreeMemD1(dstdir);
    FreeMemD1(finfo);

    return res;
}

uint16_t FATFS_getPictureNum(uint8_t *path)
{
    uint8_t res;
    uint16_t retval = 0;
    DIR tdir;
    FILINFO *tfileinfo;

    tfileinfo = AllocMemD1(sizeof(FILINFO));

    if (tfileinfo == NULL)
        return 0;
    res = f_opendir(&tdir, (const TCHAR*)path);
    if (FR_OK == res)
    {
        while (1)
        {
            res = f_readdir(&tdir, tfileinfo); // 读取目录下的一个文件
            if (res != FR_OK || tfileinfo->fname[0] == 0)
                break; // 错误了/到末尾了,退出

            printf("getPictureNum: %s \r\n", tfileinfo->fname);
            res = FATFS_getFileType((uint8_t *)tfileinfo->fname);
            if ((res & 0xF0) == 0x50) // 取高四位,看看是不是图片文件
            {
                retval++; // 有效文件数增加1
            }
        }
    }
    
    FreeMemD1(tfileinfo);

    if (res != FR_OK)
        FATFS_ErrorReport("getPictureNum", res);

    return retval;
}



/*****************************END OF FILE****/
