/*
*********************************************************************************************************
*
*   [module   ] : TIM PWM (For STM32H7)
*   [file name] : bsp_tim_pwm.c
*   [version  ] : V1.0
*   [brief    ] : timer pwm module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_TIM_PWM_H_
#define BSP_TIM_PWM_H_

#include "stm32h7xx_hal.h"

void bsp_SetTIMforInt(TIM_TypeDef* TIMx, uint32_t _ulFreq, uint8_t _PreemptionPriority, uint8_t _SubPriority);
void bsp_SetTIMOutPWM(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, TIM_TypeDef *TIMx, uint8_t _ucChannel,
                      uint32_t _ulFreq, uint32_t _ulDutyCycle);
void bsp_SetTimPwmDuty(uint32_t _ulDutyCycle);
#endif /* BSP_TIM_PWM_H_ */
