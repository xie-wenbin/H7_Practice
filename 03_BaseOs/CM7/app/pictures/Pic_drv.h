#ifndef PIC_DRIVER_H
#define PIC_DRIVER_H

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>

#include "stm32h7xx_hal.h"
#include "bsp.h"
#include "bsp_lcd.h"
#include "FATFS_drv.h"
#include "tjpgd.h"

#define PIC_FORMAT_ERR     0x27     // 格式错误
#define PIC_SIZE_ERR       0x28     // 图片尺寸错误
#define PIC_WINDOW_ERR     0x29     // 窗口设定错误
#define PIC_MEM_ERR        0x11     // 内存错误

// 图片显示驱动接口
typedef struct
{
    uint32_t (*read_point)(uint16_t x, uint16_t y);                                        // 读点函数
    void (*draw_point)(uint16_t x, uint16_t y, uint32_t color);                            // 画点函数
    void (*fill_rect)(uint16_t x, uint16_t y, uint16_t wd, uint16_t he, uint32_t color);   // 单色填充函数
    void (*draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);              // 画水平线函数
    void (*fill_color)(uint16_t x, uint16_t y, uint16_t wd, uint16_t he, uint32_t *color); // 颜色填充
} Pic_Driver_Stru;


// 图片信息
typedef struct
{
    uint16_t lcdwidth;
    uint16_t lcdheight;
    uint32_t imgwidth; // 图像的实际宽高
    uint32_t imgheight;

    uint32_t div_fac; // 缩放系数

    uint32_t set_height; // 设定的高度及宽度
    uint32_t set_width;

    uint32_t set_xoff; // x y 的偏移量
    uint32_t set_yoff;

    uint32_t cur_x; // 当前 x y 坐标
    uint32_t cur_y;
} Pic_Info_Stru;

extern Pic_Driver_Stru gPic_Drv;
extern Pic_Info_Stru gPicInfo;


void Pic_DrvInit(void);
void Pic_drawInit(void);
uint8_t Pic_isElementValid(uint16_t x, uint16_t y, uint8_t can_show);
uint8_t Pic_loadPictureFile(uint8_t *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t fast);

void *pic_memalloc(uint32_t size);
void pic_memfree(void *mf);

#endif /* PIC_DRIVER_H */

/*****************************END OF FILE****/
