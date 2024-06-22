/*
*********************************************************************************************************
*
*   [module   ] : LCD (For STM32H7)
*   [file name] : bsp_lcd_hw.h
*   [version  ] : V1.0
*   [brief    ] : dsi ltdc display module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_LCD_DEVICE_H_
#define BSP_LCD_DEVICE_H_

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery_errno.h"
#include "otm8009a.h"

#define LCD_ORIENTATION_PORTRAIT         0x00U /* Portrait orientation choice of LCD screen               */
#define LCD_ORIENTATION_LANDSCAPE        0x01U /* Landscape orientation choice of LCD screen              */

#define LCD_DEFAULT_WIDTH                OTM8009A_800X480_WIDTH
#define LCD_DEFAULT_HEIGHT               OTM8009A_800X480_HEIGHT

/**
  * @brief LCD special pins
  */
/* LCD reset pin */
#define LCD_RESET_PIN                    GPIO_PIN_3
#define LCD_RESET_PULL                   GPIO_NOPULL
#define LCD_RESET_GPIO_PORT              GPIOG
#define LCD_RESET_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define LCD_RESET_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOG_CLK_DISABLE()

/* LCD tearing effect pin */
#define LCD_TE_PIN                       GPIO_PIN_2
#define LCD_TE_GPIO_PORT                 GPIOJ
#define LCD_TE_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LCD_TE_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOJ_CLK_DISABLE()

/* Back-light control pin */
#define LCD_BL_CTRL_PIN                  GPIO_PIN_12
#define LCD_BL_CTRL_GPIO_PORT            GPIOJ
#define LCD_BL_CTRL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LCD_BL_CTRL_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOJ_CLK_DISABLE()

typedef struct
{
    uint32_t DsiHost_ColorMode;
    uint32_t DsiChirp_ColorMode;
    uint32_t LTDC_ColorMode;
    uint32_t DMA2D_ColorMode;
}LCD_DevColorMode_t;

extern DSI_HandleTypeDef   hlcd_dsi;
extern DMA2D_HandleTypeDef hlcd_dma2d;
extern LTDC_HandleTypeDef  hlcd_ltdc;
extern OTM8009A_Object_t  *Lcd_CompObj;

void LCD_HwInit(uint8_t orientation);

#endif /* BSP_LCD_DEVICE_H_ */
