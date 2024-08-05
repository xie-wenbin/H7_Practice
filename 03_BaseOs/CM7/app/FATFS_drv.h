#ifndef FATFS_EXTENSION_DRV_H
#define FATFS_EXTENSION_DRV_H

#include "bsp.h"
#include "ff.h"				/* FatFS文件系统模块*/
#include "ff_gen_drv.h"

//定义指定目录下可扫描的最大文件数量
#define FATFS_DIR_MAX_NUM  (255)

//根据表FILE_TYPE_TBL获得.
#define T_BIN		0X00	//bin文件
#define T_LRC		0X10	//lrc文件

#define T_NES		0X20	//nes文件
#define T_SMS		0X21	//sms文件

#define T_TEXT		0X30	//.txt文件
#define T_C			0X31	//.c文件
#define T_H			0X32    //.h文件

#define T_WAV		0X40	//WAV文件
#define T_MP3		0X41	//MP3文件 
#define T_APE		0X42	//APE文件
#define T_FLAC		0X43	//FLAC文件

#define T_BMP		0X50	//bmp文件
#define T_JPG		0X51	//jpg文件
#define T_JPEG		0X52	//jpeg文件		 
#define T_GIF		0X53	//gif文件  
 
#define T_AVI		0X60	//avi文件  

typedef enum
{
    FATFS_DEV_MMCSD_0 = 0,
    FATFS_DEV_NOR_FLASH,
    FATFS_DEV_USBH,
    FATFS_DEV_NUM
} FATFS_DEVICE_E;

typedef struct
{
    char **file_name;
    uint32_t file_num;
} FileDirScan_T;

/**
 * @brief show file/folder copyed message
 * 
 * @param pname file name/file folder name
 * @param percent copyed percent
 * @param mode [0]:update file name
 *             [1]:update percent
 *             [2]:update file folder
 *             [3~7]:reserve
 */
typedef uint8_t (*fscpymsg) (uint8_t* pname, uint8_t percent, uint8_t mode);


void FATFS_ErrorReport(const char * api_name, FRESULT result);

void FATFS_diskInitialize(void);
FATFS *FATFS_getFsHandler(uint8_t devid);
uint8_t FATFS_getFree(uint8_t *drvPath, uint32_t *total, uint32_t *free);
uint8_t FATFS_getFileType(uint8_t *fname);
FRESULT FATFS_scanDirectoryList(const char *dir_path, const char *file_suffix, FileDirScan_T *scanfile);

uint32_t FATFS_getFolderSize(uint8_t *pfdname);
uint16_t FATFS_getPictureNum(uint8_t *path);

#endif /* FATFS_EXTENSION_DRV_H */
