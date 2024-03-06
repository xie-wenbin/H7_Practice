/*
*********************************************************************************************************
*
*   [module   ] : BSP (For STM32H7)
*	[file name] : bsp.h
*	[version  ] : V1.0
*	[brief    ] : This is the main file for the underlying drivers of the hardware. 
*                Each c file can #include "bsp.h" to include all peripheral driver modules.
*			     bsp = Borad surport packet
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H_

/* Macro to bsp module access control for dule core */
#ifdef CORE_CM7
#define USE_CORE_CM7_BSP
#endif

#ifdef CORE_CM4
#define USE_CORE_CM4_BSP
#endif

/* Macro to switch global interrupt */
#define ENABLE_INT()	__set_PRIMASK(0)	/* enable global interrupt */
#define DISABLE_INT()	__set_PRIMASK(1)	/* disable global interrupt */

#define ERROR_HANDLER()		Error_Handler(__FILE__, __LINE__);

#define  Enable_EventRecorder  0

#if Enable_EventRecorder == 1
    #include "EventRecorder.h"
#endif

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
    #define TRUE  1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#include "bsp_dwt.h"
#include "bsp_timer.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_uart.h"
#include "bsp_tim_pwm.h"
#include "bsp_fmc_sdram.h"

void bsp_Init(void);
void bsp_RCC_GPIO_Enable(GPIO_TypeDef* GPIOx);
void bsp_ConfigGpioOut(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX);
void Error_Handler(char *file, uint32_t line);
void bsp_RunPer10ms(void);
void bsp_RunPer1ms(void);

#endif /* _BSP_H_ */
