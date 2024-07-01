#include "bsp.h"
#include "bsp_lcd.h"


const LCD_UTILS_Drv_t LCD_Driver =
{
    BSP_LCD_DrawBitmap,
    BSP_LCD_FillRGBRect,
    BSP_LCD_DrawHLine,
    BSP_LCD_DrawVLine,
    BSP_LCD_FillRect,
    BSP_LCD_ReadPixel,
    BSP_LCD_WritePixel,
    BSP_LCD_GetXSize,
    BSP_LCD_GetYSize,
    BSP_LCD_SetActiveLayer,
    BSP_LCD_GetPixelFormat
};

TS_Init_t *hTS;


void bsp_LcdTsInit(void)
{
    uint32_t ts_status = BSP_ERROR_NONE;
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(&x_size);
    BSP_LCD_GetYSize(&y_size);

    hTS->Width = x_size;
    hTS->Height = y_size;
    hTS->Orientation = TS_SWAP_XY | TS_SWAP_Y;
    hTS->Accuracy = 0;

    /* Touchscreen initialization */
    ts_status = BSP_TS_Init(hTS);

    printf("LCD TS Init Done %d\r\n", ts_status);
}


void bsp_LcdInit(void)
{
    BSP_LCD_InitContext();

    LCD_UTIL_SetFuncDriver(&LCD_Driver);

    bsp_LcdTsInit();
}


