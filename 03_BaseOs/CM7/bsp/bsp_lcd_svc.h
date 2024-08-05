/*
*********************************************************************************************************
*
*   [module   ] : LCD (For STM32H7)
*   [file name] : bsp_lcd_service.c
*   [version  ] : V1.0
*   [brief    ] : lcd draw service interface module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_LCD_SERVICE_H_
#define BSP_LCD_SERVICE_H_

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery_errno.h"

#if 0
#define CONVERTARGB88882RGB565(argb) \
    ((uint16_t)((((uint16_t)(((uint8_t)((argb) >> 16)) & 0xFF) >> 3) & 0x1F)) | \
     ((((uint16_t)(((uint8_t)((argb) >> 8)) & 0xFF) & 0xFC) << 3)) | \
     (((uint16_t)(((uint8_t)((argb)) & 0xFF)) & 0xF8) << 8))

#else
#define CONVERTARGB88882RGB565(Color)((((Color & 0xFFU) >> 3) & 0x1FU) |\
                                     (((((Color & 0xFF00U) >> 8) >>2) & 0x3FU) << 5) |\
                                     (((((Color & 0xFF0000U) >> 16) >>3) & 0x1FU) << 11))
#endif
#define CONVERTRGB5652ARGB8888(Color)(((((((Color >> 11) & 0x1FU) * 527) + 23) >> 6) << 16) |\
                                     ((((((Color >> 5) & 0x3FU) * 259) + 33) >> 6) << 8) |\
                                     ((((Color & 0x1FU) * 527) + 23) >> 6) | 0xFF000000)


#define BSP_LCD_RELOAD_NONE              0U                            /* No reload executed       */
#define BSP_LCD_RELOAD_IMMEDIATE         LTDC_RELOAD_IMMEDIATE         /* Immediate Reload         */
#define BSP_LCD_RELOAD_VERTICAL_BLANKING LTDC_RELOAD_VERTICAL_BLANKING /* Vertical Blanking Reload */

/** @defgroup LCD_Exported_Constants LCD Exported Constants
  * @{ refer to LTDC_PIXEL_FORMAT
  */
#define LCD_PIXEL_FORMAT_ARGB8888        0x00000000U   /*!< ARGB8888 LTDC pixel format */
#define LCD_PIXEL_FORMAT_RGB888          0x00000001U   /*!< RGB888 LTDC pixel format   */
#define LCD_PIXEL_FORMAT_RGB565          0x00000002U   /*!< RGB565 LTDC pixel format   */
#define LCD_PIXEL_FORMAT_ARGB1555        0x00000003U   /*!< ARGB1555 LTDC pixel format */
#define LCD_PIXEL_FORMAT_ARGB4444        0x00000004U   /*!< ARGB4444 LTDC pixel format */
#define LCD_PIXEL_FORMAT_L8              0x00000005U   /*!< L8 LTDC pixel format       */
#define LCD_PIXEL_FORMAT_AL44            0x00000006U   /*!< AL44 LTDC pixel format     */
#define LCD_PIXEL_FORMAT_AL88            0x00000007U   /*!< AL88 LTDC pixel format     */

typedef struct
{
    uint32_t XSize;
    uint32_t YSize;
    uint32_t Orientation;
    uint32_t ActiveLayer;
    uint32_t PixelFormat;
    uint32_t BppFactor;
    uint32_t IsMspCallbacksValid;
    uint32_t ReloadEnable;
} BSP_LCD_Ctx_t;

typedef struct
{
  /* Control functions */
  int32_t (*Init             )(void*, uint32_t, uint32_t);
  int32_t (*DeInit           )(void*);
  int32_t (*ReadID           )(void*, uint32_t*);
  int32_t (*DisplayOn        )(void*);
  int32_t (*DisplayOff       )(void*);
  int32_t (*SetBrightness    )(void*, uint32_t);
  int32_t (*GetBrightness    )(void*, uint32_t*);
  int32_t (*SetOrientation   )(void*, uint32_t);
  int32_t (*GetOrientation   )(void*, uint32_t*);

  /* Drawing functions*/
  int32_t ( *SetCursor       ) (void*, uint32_t, uint32_t);
  int32_t ( *DrawBitmap      ) (void*, uint32_t, uint32_t, uint8_t *);
  int32_t ( *FillRGBRect     ) (void*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t);
  int32_t ( *DrawHLine       ) (void*, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *DrawVLine       ) (void*, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *FillRect        ) (void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *GetPixel        ) (void*, uint32_t, uint32_t, uint32_t*);
  int32_t ( *SetPixel        ) (void*, uint32_t, uint32_t, uint32_t);
  int32_t ( *GetXSize        ) (void*, uint32_t *);
  int32_t ( *GetYSize        ) (void*, uint32_t *);
}LCD_Drv_t;

void BSP_LCD_InitContext(void);
void BSP_LCD_DrawExampleTest(void);

int32_t BSP_LCD_SetActiveLayer(uint32_t LayerIndex);
int32_t BSP_LCD_GetPixelFormat(uint32_t *PixelFormat);
int32_t BSP_LCD_GetXSize(uint32_t *XSize);
int32_t BSP_LCD_GetYSize(uint32_t *YSize);
int32_t BSP_LCD_WritePixel(uint32_t Xpos, uint32_t Ypos, uint32_t Color);
int32_t BSP_LCD_ReadPixel(uint32_t Xpos, uint32_t Ypos, uint32_t *Color);
int32_t BSP_LCD_FillRGBRect(uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height);
int32_t BSP_LCD_FillRect(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
int32_t BSP_LCD_DrawHLine(uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t BSP_LCD_DrawVLine(uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t BSP_LCD_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);


void BSP_LCD_ColorFill(uint16_t Xpos, uint16_t Ypos, uint16_t xSize, uint16_t ySize, uint32_t *color);

#endif /* BSP_LCD_SERVICE_H_ */
