/*
*********************************************************************************************************
*
*   [module   ] : timer (For STM32H7)
*   [file name] : bsp_timer.c
*   [version  ] : V1.0
*   [brief    ] : timer module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_TIMER_H_
#define BSP_TIMER_H_

#include "stm32h7xx_hal.h"

#define SOFTWARE_TMR_COUNT   4


typedef enum
{
	TMR_ONCE_MODE = 0,    /* Timer will not automatically restart when it expires */
	TMR_AUTO_MODE = 1     /* Timer will     automatically restart when it expires */
}TMR_MODE_E;


typedef struct
{
	volatile uint8_t Mode;		/* timer counter mode */
	volatile uint8_t Flag;		/* timer counter arrival flag */
	volatile uint32_t Count;	/* timer counter */
	volatile uint32_t PreLoad;	/* timer counter perload value */
}SOFT_TMR;

void bsp_InitTimer(void);
void bsp_DelayMS(uint32_t ms);
void bsp_DelayUS(uint32_t us);
void bsp_StartTimer(uint8_t timerId, uint32_t period);
void bsp_StartAutoTimer(uint8_t timerId, uint32_t period);
void bsp_StopTimer(uint8_t timerId);
uint8_t bsp_CheckTimer(uint8_t timerId);
void SysTick_ISR(void);
uint32_t bsp_GetRunTime(void);

#endif /* BSP_TIMER_H_ */
