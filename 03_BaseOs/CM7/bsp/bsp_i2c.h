/*
*********************************************************************************************************
*
*   [module   ] : I2C (For STM32H7)
*   [file name] : bsp_i2c.h
*   [version  ] : V1.0
*   [brief    ] : i2c hardware bus module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery_errno.h"

/**
  * @}
  */
/** @defgroup STM32H747I_DISCO_BUS_Exported_Constants Exported Constants
  * @{
  */
/* Definition for I2C4 clock resources */
#define BUS_I2C4                              I2C4
#define BUS_I2C4_CLK_ENABLE()                  __HAL_RCC_I2C4_CLK_ENABLE()
#define BUS_I2C4_CLK_DISABLE()                 __HAL_RCC_I2C4_CLK_DISABLE()
#define BUS_I2C4_SCL_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOD_CLK_ENABLE()
#define BUS_I2C4_SCL_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOD_CLK_DISABLE()
#define BUS_I2C4_SDA_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOD_CLK_ENABLE()
#define BUS_I2C4_SDA_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOD_CLK_DISABLE()

#define BUS_I2C4_FORCE_RESET()                 __HAL_RCC_I2C4_FORCE_RESET()
#define BUS_I2C4_RELEASE_RESET()               __HAL_RCC_I2C4_RELEASE_RESET()

/* Definition for I2C4 Pins */
#define BUS_I2C4_SCL_PIN                       GPIO_PIN_12
#define BUS_I2C4_SDA_PIN                       GPIO_PIN_13
#define BUS_I2C4_SCL_GPIO_PORT                 GPIOD
#define BUS_I2C4_SDA_GPIO_PORT                 GPIOD
#define BUS_I2C4_SCL_AF                        GPIO_AF4_I2C4
#define BUS_I2C4_SDA_AF                        GPIO_AF4_I2C4

#ifndef BUS_I2C4_FREQUENCY
   #define BUS_I2C4_FREQUENCY  100000U /* Frequency of I2Cn = 100 KHz*/
#endif

extern I2C_HandleTypeDef hbus_i2c4;

int32_t BSP_I2C4_Init(void);
int32_t BSP_I2C4_DeInit(void);
int32_t BSP_I2C4_IsReady(uint16_t DevAddr, uint32_t Trials);
int32_t BSP_I2C4_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C4_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
int32_t BSP_I2C4_RegisterDefaultMspCallbacks (void);
int32_t BSP_I2C4_RegisterMspCallbacks (BSP_I2C_Cb_t *Callback);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */

HAL_StatusTypeDef MX_I2C4_Init(I2C_HandleTypeDef *phi2c, uint32_t timing);


#endif /* BSP_I2C_H */
