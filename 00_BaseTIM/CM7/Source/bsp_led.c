/*
*********************************************************************************************************
*
*   [module   ] : LED (For STM32H7)
*   [file name] : bsp_led.c
*   [version  ] : V1.0
*   [brief    ] : LED module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "bsp_led.h"

static GPIO_TypeDef* s_led_port[BSP_LED_NUM] = { BSP_LED1_GPIO_PORT,
                                                   BSP_LED2_GPIO_PORT,
                                                   BSP_LED3_GPIO_PORT,
                                                   BSP_LED4_GPIO_PORT};

static const uint32_t s_led_pin[BSP_LED_NUM] = { BSP_LED1_PIN,
                                                   BSP_LED2_PIN,
                                                   BSP_LED3_PIN,
                                                   BSP_LED4_PIN};

#define BSP_LED_CLK_ENABLE() { \
        __HAL_RCC_GPIOI_CLK_ENABLE();   \
    };

/**
  * @brief  Configures LED on GPIO.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  BSP_LED1
  *            @arg  BSP_LED2
  *            @arg  BSP_LED3
  *            @arg  BSP_LED4
  * @retval BSP status
  */
int32_t  bsp_InitLed(BspLed_TypeEnum LedId)
{
    int32_t ret = 0;
    GPIO_InitTypeDef  GPIO_InitStruct;
    if (LedId >= BSP_LED_NUM)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    /* Enable the GPIO_LED clock */
    BSP_LED_CLK_ENABLE();

    /* Configure the GPIO_LED pin */
    GPIO_InitStruct.Pin = s_led_pin[LedId];
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(s_led_port[LedId], &GPIO_InitStruct);

    /* By default, turn off LED */
    HAL_GPIO_WritePin(s_led_port[LedId], s_led_pin[LedId], GPIO_PIN_SET);
    return ret;
}


/**
  * @brief  DeInit LEDs.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t  bsp_DeInitLed(BspLed_TypeEnum LedId)
{
    int32_t ret = 0;
    GPIO_InitTypeDef  gpio_init_structure;

    if (LedId >= BSP_LED_NUM)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    /* DeInit the GPIO_LED pin */
    gpio_init_structure.Pin = s_led_pin[LedId];
    /* Turn off LED */
    HAL_GPIO_WritePin (s_led_port[LedId], (uint16_t)s_led_pin[LedId], GPIO_PIN_SET);
    HAL_GPIO_DeInit (s_led_port[LedId], gpio_init_structure.Pin);
    return ret;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led LED to be set on
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t  bsp_LedOn(BspLed_TypeEnum LedId)
{
    int32_t ret = 0;
    if (LedId < BSP_LED_NUM)
    {
    HAL_GPIO_WritePin (s_led_port[LedId], (uint16_t)s_led_pin[LedId], GPIO_PIN_RESET);
    }
    return ret;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led LED to be set off
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t  bsp_LedOff(BspLed_TypeEnum LedId)
{
    int32_t ret = 0;
    if (LedId < BSP_LED_NUM)
    {
    HAL_GPIO_WritePin (s_led_port[LedId], (uint16_t)s_led_pin[LedId], GPIO_PIN_SET);
    }
    return ret;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led LED to be toggled
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t  bsp_LedToggle(BspLed_TypeEnum LedId)
{
    int32_t ret = 0;
    if (LedId < BSP_LED_NUM)
    {
    HAL_GPIO_TogglePin(s_led_port[LedId], (uint16_t)s_led_pin[LedId]);
    }
    
    return ret;
  }
/**
  * @brief  Get the selected LED state.
  * @param  Led LED to be get its state
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval LED status
  */
int32_t bsp_LedGetState (BspLed_TypeEnum LedId)
{
    int32_t ret = 0;
    if (LedId < BSP_LED_NUM)
    {
    ret = (int32_t)HAL_GPIO_ReadPin (s_led_port[LedId], (uint16_t)s_led_pin[LedId]);
    }
    return ret;
}


void bsp_InitAllLed(void)
{
    bsp_InitLed(BSP_LED1);
    bsp_InitLed(BSP_LED2);
    bsp_InitLed(BSP_LED3);
    bsp_InitLed(BSP_LED4);
}
