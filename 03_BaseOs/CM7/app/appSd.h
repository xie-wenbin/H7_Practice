#ifndef APP_SD_H
#define APP_SD_H

#include "main.h"
#include "bsp.h"
#include "bsp_sdio_sd.h"

/* Exported macro ------------------------------------------------------------*/
#define APPSD_EVT_INITDONE      (0x01 << 0)
#define APPSD_EVT_SD_INSERT     (0x01 << 1)
#define APPSD_EVT_SD_REMOVE     (0x01 << 2)
#define APPSD_EVT_SHOW_ROOTDIR  (0x01 << 3)

// for test event
#define APPSD_EVT_NEWFILE      (0x01 << 4)
#define APPSD_EVT_READFILE     (0x01 << 5)
#define APPSD_EVT_NEWDIR       (0x01 << 6)
#define APPSD_EVT_DELETEALL    (0x01 << 7)
#define APPSD_EVT_SPEED_TEST   (0x01 << 8)


/* Exported variavles ------------------------------------------------------------*/
extern osThreadId_t ThreadIdSdMainProc;
extern osThreadId_t ThreadIdSdDetect;

extern osEventFlagsId_t gEventId_sdapp;
/* Exported functions --------------------------------------------------------*/
extern void appSd_CreateTask(void);


#endif /* APP_SD_H */
