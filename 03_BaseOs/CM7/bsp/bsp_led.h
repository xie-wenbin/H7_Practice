/*
*********************************************************************************************************
*
*   [module   ] : LED (For STM32H7)
*   [file name] : bsp_led.h
*   [version  ] : V1.0
*   [brief    ] : LED module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/

#ifndef BSP_LED_H_
#define BSP_LED_H_

#include "stm32h7xx_hal.h"

typedef enum
{
  BSP_LED1 = 0U,
  BSP_LED_GREEN = BSP_LED1,
  BSP_LED2 = 1U,
  BSP_LED_ORANGE = BSP_LED2,
  BSP_LED3 = 2U,
  BSP_LED_RED = BSP_LED3,
  BSP_LED4 = 3U,
  BSP_LED_BLUE = BSP_LED4,
  BSP_LED_NUM
} BspLed_TypeEnum;

#define BSP_LED1_GPIO_PORT                   GPIOI
#define BSP_LED1_PIN                         GPIO_PIN_12

#define BSP_LED2_GPIO_PORT                   GPIOI
#define BSP_LED2_PIN                         GPIO_PIN_13

#define BSP_LED3_GPIO_PORT                   GPIOI
#define BSP_LED3_PIN                         GPIO_PIN_14

#define BSP_LED4_GPIO_PORT                   GPIOI
#define BSP_LED4_PIN                         GPIO_PIN_15

#define BSP_LEDx_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define BSP_LEDx_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOI_CLK_DISABLE()


int32_t  bsp_InitLed(BspLed_TypeEnum LedId);
int32_t  bsp_DeInitLed(BspLed_TypeEnum LedId);
int32_t  bsp_LedOn(BspLed_TypeEnum LedId);
int32_t  bsp_LedOff(BspLed_TypeEnum LedId);
int32_t  bsp_LedToggle(BspLed_TypeEnum LedId);
int32_t  bsp_LedGetState (BspLed_TypeEnum LedId);
void bsp_InitAllLed(void);

#endif /* BSP_LED_H_ */
