#include "Pic_drv.h"
#include "bsp_lcd_svc.h"
#include "FATFS_drv.h"

Pic_Driver_Stru gPic_Drv;
Pic_Info_Stru gPicInfo;

uint32_t PicDrv_read_point(uint16_t x, uint16_t y)
{
    uint32_t color = 0;

    LCD_UTIL_GetPixel(x, y, &color);

    return color;
}

void PicDrv_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    LCD_UTIL_SetPixel(x, y, color);
}

void PicDrv_fill_rect(uint16_t x, uint16_t y, uint16_t wd, uint16_t he, uint32_t color)
{
    LCD_UTIL_FillRect(x, y, wd, he, color);
}

void PicDrv_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color)
{
    LCD_UTIL_DrawHLine(x, y, len, color);
}

void PicDrv_fill_color(uint16_t x, uint16_t y, uint16_t wd, uint16_t he, uint32_t *color)
{
    BSP_LCD_ColorFill(x, y, wd, he, (uint32_t*)color);
}


void Pic_DrvInit(void)
{
    uint32_t lcdwidth, lcdheight;
    gPic_Drv.read_point = PicDrv_read_point;
    gPic_Drv.draw_point = PicDrv_draw_point;
    gPic_Drv.draw_hline = PicDrv_draw_hline;
    gPic_Drv.fill_rect  = PicDrv_fill_rect;
    gPic_Drv.fill_color = PicDrv_fill_color; //PicDrv_fill_color;

    BSP_LCD_GetXSize(&lcdwidth);
    BSP_LCD_GetYSize(&lcdheight);

    memset(&gPicInfo, 0, sizeof(Pic_Info_Stru));

    gPicInfo.lcdwidth = lcdwidth;
    gPicInfo.lcdheight = lcdheight;
}

void Pic_drawInit(void)
{
    float temp, temp1;
    
    temp = (float)gPicInfo.set_width / gPicInfo.imgwidth;
    temp1 = (float)gPicInfo.set_height / gPicInfo.imgheight;
    if (temp < temp1)
        temp1 = temp;
    
    if (temp1 > 1) temp1 = 1;

    gPicInfo.set_xoff += (gPicInfo.set_width - (gPicInfo.imgwidth * temp1)) / 2;
    gPicInfo.set_yoff += (gPicInfo.set_height - (gPicInfo.imgheight * temp1)) / 2;
    temp1 *= 8192;
    gPicInfo.div_fac = temp1;
    gPicInfo.cur_x = 0xFFFF;
    gPicInfo.cur_y = 0xFFFF;
}

/**
 * @brief 判断这个像素是否可以显示
 * 
 * @param x 
 * @param y 
 * @param can_show 功能变量
 * @return uint8_t 0-不需要显示 1-需要显示
 */
uint8_t Pic_isElementValid(uint16_t x, uint16_t y, uint8_t can_show)
{
    if ((x != gPicInfo.cur_x) || (y != gPicInfo.cur_y))
    {
        if (1 == can_show)
        {
            gPicInfo.cur_x = x;
            gPicInfo.cur_y = y;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 读取图片文件进行画图
 * 
 * @param filename 要显示的图片文件名 格式-BMP/JPG/JPEG/GIF
 * @param x 开始的x坐标
 * @param y 开始的y坐标
 * @param width 显示的宽度
 * @param height 显示的长度
 * @param fast 使能jpeg/jpg小图片(图片尺寸小于等于液晶屏分辨率)快速解码，0-disable 1-enable
 * @return uint8_t 
 */
uint8_t Pic_loadPictureFile(uint8_t *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t fast)
{
    uint8_t res; // 返回值
    uint8_t temp;

    if ((x + width) > gPicInfo.lcdwidth)
        return PIC_WINDOW_ERR; // x坐标超范围了.

    if ((y + height) > gPicInfo.lcdheight)
        return PIC_WINDOW_ERR; // y坐标超范围了.

    // 得到显示方框大小
    if (width == 0 || height == 0)
        return PIC_WINDOW_ERR; // 窗口设定错误

    gPicInfo.set_height = height;
    gPicInfo.set_width = width;

    // 显示区域无效
    if (gPicInfo.set_height == 0 || gPicInfo.set_width == 0)
    {
        uint32_t lcdwidth, lcdheight;
        BSP_LCD_GetXSize(&lcdwidth);
        BSP_LCD_GetYSize(&lcdheight);

        gPicInfo.set_height = lcdheight;
        gPicInfo.set_width = lcdwidth;
        return FALSE;
    }

    if (gPic_Drv.fill_color == NULL)
        fast = 0; // 颜色填充函数未实现,不能快速显示

    // 显示的开始坐标点
    gPicInfo.set_yoff = y;
    gPicInfo.set_xoff = x;

    // 文件名传递
    temp = FATFS_getFileType((uint8_t *)filename); // 得到文件的类型
    switch (temp)
    {
        case T_BMP:
            // res = stdbmp_decode(filename); // 解码bmp
            break;
        case T_JPG:
        case T_JPEG:
            res = jpg_decode(filename, fast); // 解码JPG/JPEG
            break;
        case T_GIF:
            // res = gif_decode(filename, x, y, width, height); // 解码gif
            break;
        default:
            res = PIC_FORMAT_ERR; // 非图片格式!!!
            break;
    }

    return res;
}

// 动态分配内存
void *pic_memalloc(uint32_t size)
{
    return (void *)AllocMemZone(MEM_ZONE_D1_AXIM, size);
}
// 释放内存
void pic_memfree(void *mf)
{
    FreeMemZone(MEM_ZONE_D1_AXIM, mf);
}

/*****************************END OF FILE****/
