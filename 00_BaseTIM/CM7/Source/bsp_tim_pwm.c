/*
*********************************************************************************************************
*
*   [module   ] : TIM PWM (For STM32H7)
*   [file name] : bsp_tim_pwm.c
*   [version  ] : V1.0
*   [brief    ] : timer module
*
*	Copyright (C), 2023-2030
*
*Note that the STM32H7 has TIM1-TIM17 (without TIM9, TIM10, and TIM11) for a total of 14 timers.
*
*TIM channels that can be exported to GPIO:
    TIM1_CH1,  PA8   PE9   PK1
    TIM1_CH2,  PA9   PE11
    TIM1_CH3,  PA10  PE13  PJ9
    TIM1_CH4,  PA11  PE14  PJ11

    TIM2_CH1,  PA0   PA5   PA15
    TIM2_CH2,  PA1   PB3  
    TIM2_CH3,  PA2
    TIM2_CH4,  PA3   PB11

    TIM3_CH1,  PA6   PC6  PB4
    TIM3_CH2,  PA7   PC7  PB5   
    TIM3_CH3,  PB0   PC8  
    TIM3_CH4,  PB1   PC9  

    TIM4_CH1,  PB6   PD12
    TIM4_CH2,  PB7   PD13
    TIM4_CH3,  PB8   PD14
    TIM4_CH4,  PB9   PD15

    TIM5_CH1,  PA0   PH10
    TIM5_CH2,  PA1   PH11
    TIM5_CH3,  PA2   PH12
    TIM5_CH4,  PA3   PI0

    TIM8_CH1,  PC6   PI5  PJ8
    TIM8_CH2,  PC7   PI6  PJ10
    TIM8_CH3,  PC8   PI7  PK0
    TIM8_CH4,  PC9 

    TIM12_CH1,  PB14  PH6
    TIM12_CH2,  PB15  PH9

    TIM13_CH1,  PF8

    TIM14_CH1,  PF9

    TIM15_CH1,  PE5 
    TIM15_CH2,  PE6

    TIM16_CH1,  PB8   PF6
    TIM16_CH2,  PF7

    TIM17_CH1,  PB9
*********************************************************************************************************
*/

#include "bsp.h"
#include "bsp_tim_pwm.h"

/**
 * @brief Enable TIM RCC clock
 * 
 * @param TIMx TIM1 - TIM17
 */
void bsp_RCC_TIM_Enable(TIM_TypeDef* TIMx)
{
    if (TIMx == TIM1) __HAL_RCC_TIM1_CLK_ENABLE();
    else if (TIMx == TIM2) __HAL_RCC_TIM2_CLK_ENABLE();
    else if (TIMx == TIM3) __HAL_RCC_TIM3_CLK_ENABLE();
    else if (TIMx == TIM4) __HAL_RCC_TIM4_CLK_ENABLE();
    else if (TIMx == TIM5) __HAL_RCC_TIM5_CLK_ENABLE();
    else if (TIMx == TIM6) __HAL_RCC_TIM6_CLK_ENABLE();
    else if (TIMx == TIM7) __HAL_RCC_TIM7_CLK_ENABLE();
    else if (TIMx == TIM8) __HAL_RCC_TIM8_CLK_ENABLE();
    else if (TIMx == TIM12) __HAL_RCC_TIM12_CLK_ENABLE();
    else if (TIMx == TIM13) __HAL_RCC_TIM13_CLK_ENABLE();
    else if (TIMx == TIM14) __HAL_RCC_TIM14_CLK_ENABLE();
    else if (TIMx == TIM15) __HAL_RCC_TIM15_CLK_ENABLE();
    else if (TIMx == TIM16) __HAL_RCC_TIM16_CLK_ENABLE();
    else if (TIMx == TIM17) __HAL_RCC_TIM17_CLK_ENABLE();	
    else
    {
        Error_Handler(__FILE__, __LINE__);
    }	
}

/**
 * @brief disable TIM RCC clock
 * 
 * @param TIMx TIM1 - TIM17
 */
void bsp_RCC_TIM_Disable(TIM_TypeDef* TIMx)
{
    if (TIMx == TIM1) __HAL_RCC_TIM3_CLK_DISABLE();
    else if (TIMx == TIM2) __HAL_RCC_TIM2_CLK_DISABLE();
    else if (TIMx == TIM3) __HAL_RCC_TIM3_CLK_DISABLE();
    else if (TIMx == TIM4) __HAL_RCC_TIM4_CLK_DISABLE();
    else if (TIMx == TIM5) __HAL_RCC_TIM5_CLK_DISABLE();
    else if (TIMx == TIM6) __HAL_RCC_TIM6_CLK_DISABLE();
    else if (TIMx == TIM7) __HAL_RCC_TIM7_CLK_DISABLE();
    else if (TIMx == TIM8) __HAL_RCC_TIM8_CLK_DISABLE();
    else if (TIMx == TIM12) __HAL_RCC_TIM12_CLK_DISABLE();
    else if (TIMx == TIM13) __HAL_RCC_TIM13_CLK_DISABLE();
    else if (TIMx == TIM14) __HAL_RCC_TIM14_CLK_DISABLE();
    else if (TIMx == TIM15) __HAL_RCC_TIM15_CLK_DISABLE();
    else if (TIMx == TIM16) __HAL_RCC_TIM16_CLK_DISABLE();
    else if (TIMx == TIM17) __HAL_RCC_TIM17_CLK_DISABLE();
    else
    {
        Error_Handler(__FILE__, __LINE__);
    }
}
/**
 * @brief get TIM port AF config selection
 * 
 * @param TIMx 
 * @return uint8_t 
 */
uint8_t bsp_GetPinAFofTIM(TIM_TypeDef* TIMx)
{
	uint8_t ret = 0;

	if (TIMx == TIM1) ret = GPIO_AF1_TIM1;
	else if (TIMx == TIM2) ret = GPIO_AF1_TIM2;
	else if (TIMx == TIM3) ret = GPIO_AF2_TIM3;
	else if (TIMx == TIM4) ret = GPIO_AF2_TIM4;
	else if (TIMx == TIM5) ret = GPIO_AF2_TIM5;
	else if (TIMx == TIM8) ret = GPIO_AF3_TIM8;
	else if (TIMx == TIM12) ret = GPIO_AF2_TIM12;
	else if (TIMx == TIM13) ret = GPIO_AF9_TIM13;
	else if (TIMx == TIM14) ret = GPIO_AF9_TIM14;
	else if (TIMx == TIM15) ret = GPIO_AF4_TIM15;
	else if (TIMx == TIM16) ret = GPIO_AF1_TIM16;
	else if (TIMx == TIM17) ret = GPIO_AF1_TIM17;
	else
	{
		Error_Handler(__FILE__, __LINE__);
	}
	
	return ret;
}

/**
 * @brief config TIM pin Alternate Function mapping
 * 
 * @param GPIOx GPIOA - GPIOK
 * @param GPIO_PinX GPIO_PIN_0 - GPIO__PIN_15
 * @param TIMx TIM1 - TIM17
 */
void bsp_TimPinRemap(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX, TIM_TypeDef* TIMx)
{
    GPIO_InitTypeDef   GPIO_InitStruct;

    bsp_RCC_TIM_Enable(TIMx);

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = bsp_GetPinAFofTIM(TIMx);
    GPIO_InitStruct.Pin = GPIO_PinX;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

TIM_HandleTypeDef  gTimHandle = {0};
uint16_t gusPeriod = 0;
/**
 * @brief Set the frequency and duty cycle of the PWM signal output by the pin.
 * When the frequency is 0 and the duty cycle is 0,GPI0 outputs 0;
 * When the frequency is 0 and the duty cycle is 100%, GPIO outputs 1.
 * @param GPIOx GPIOA - GPIOK
 * @param GPIO_Pin GPIO_PIN_0 - GPIO__PIN_15
 * @param TIMx TIM1 - TIM17
 * @param _ucChannel timechannel to use, 1-4
 * @param _ulFreq PWM frequency,uint Hz(in actual test cna reach 100MHz), 0
 * @param _ulDutyCycle PWM duty cycle, uint 1/10000.(exp:5000, represent 50.00%)
 */
void bsp_SetTIMOutPWM(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, TIM_TypeDef *TIMx, uint8_t _ucChannel,
                      uint32_t _ulFreq, uint32_t _ulDutyCycle)
{
    TIM_HandleTypeDef  TimHandle = {0};
    TIM_OC_InitTypeDef sConfig = {0};
    uint16_t usPeriod = 0;
    uint16_t usPrescaler = 0;
    uint32_t pulse = 0;
    uint32_t uiTIMxCLK = 0;
    const uint16_t TimChannel[6+1] = {0, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4, TIM_CHANNEL_5, TIM_CHANNEL_6};

    if ((_ucChannel > 6) || (_ulDutyCycle > 10000))
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /* GPIO Pin config */

    if (_ulDutyCycle == 0)
    {
        bsp_ConfigGpioOut(GPIOx, GPIO_Pin);
        GPIOx->BSRR |= (GPIO_Pin << 16); /* PWM = 0, low levle*/
        return;
    }
    else if (_ulDutyCycle == 10000)
    {
        bsp_ConfigGpioOut(GPIOx, GPIO_Pin);	
        GPIOx->BSRR |= GPIO_Pin; /* PWM = 1*/
        return;
    }

    bsp_TimPinRemap(GPIOx, GPIO_Pin, TIMx);

    /* TIM PWM config */
    bsp_RCC_TIM_Enable(TIMx);

    /*-----------------------------------------------------------------------
        The clock configuration of the void SystemClock_Config(void) function in the bsp.c file is as follows:

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        APB1 TIMxCLK = APB1 x 2 = 200MHz;
        APB2 TIMxCLK = APB2 x 2 = 200MHz;
        APB4 TIMCLK 100MHz;

        APB1 TIMER: TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14 LPTIM1
        APB2 TIMER: TIM1, TIM8, TIM15, TIM16 TIM17

        APB4 TIMER: LPTIM2 LPTIM3 LPTIM4 LPTIM5
    ----------------------------------------------------------------------- */
    if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM15) || (TIMx == TIM16) || (TIMx == TIM17))
    {
        /* APB2 TIMER clock = 200M */
        uiTIMxCLK = SystemCoreClock / 2;
    }
    else	
    {
        /* APB1 TIMER clock = 200M */
        uiTIMxCLK = SystemCoreClock / 2;
    }

    if (_ulFreq < 100)
    {
        usPrescaler = 10000 - 1;                      /* Prescaler = 10000 */
        usPeriod = (uiTIMxCLK / 10000) / _ulFreq - 1; /* reload value */
    }
    else if (_ulFreq < 3000)
    {
        usPrescaler = 100 - 1;                      /* Prescaler = 100 */
        usPeriod = (uiTIMxCLK / 100) / _ulFreq - 1; /* reload value */
    }
    else /* frequencies greater than 4K, no frequency division is required */
    {
        usPrescaler = 0;                    /* Prescaler = 1 */
        usPeriod = uiTIMxCLK / _ulFreq - 1; /* reload value */
    }

    pulse = (_ulDutyCycle * usPeriod) / 10000;
    

    HAL_TIM_PWM_DeInit(&TimHandle);

    TimHandle.Instance = TIMx;
	TimHandle.Init.Prescaler         = usPrescaler;
	TimHandle.Init.Period            = usPeriod;
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;
	TimHandle.Init.AutoReloadPreload = 0;
	if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

    // for test
    gusPeriod = usPeriod;
    gTimHandle = TimHandle;

    /*configure the timer PWM output channel*/
    sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfig.Pulse = pulse; /*Duty Cycle*/

    if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TimChannel[_ucChannel]) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

    if (HAL_TIM_PWM_Start(&TimHandle, TimChannel[_ucChannel]) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
}

void bsp_SetTimPwmDuty(uint32_t _ulDutyCycle)
{
    uint32_t pulse = 0;
    pulse = (_ulDutyCycle * gusPeriod) / 10000;
    __HAL_TIM_SET_COMPARE(&gTimHandle, TIM_CHANNEL_1, pulse);
}

/**
 * @brief Configure TIM and NVIC for simple timing interrupts and enable timing interrupts
 *        note that the interrupt service function needs to be implemented by the user application
 * 
 * @param TIMx TIM1 - TIM17
 * @param _ulFreq timing frequency(hz)  0:close
 * @param _PreemptionPriority NVIC preemption priority
 * @param _SubPriority NVIC Sub priority
 */
void bsp_SetTIMforInt(TIM_TypeDef* TIMx, uint32_t _ulFreq, uint8_t _PreemptionPriority, uint8_t _SubPriority)
{
    TIM_HandleTypeDef   TimHandle = {0};
    uint16_t usPeriod;
    uint16_t usPrescaler;
    uint32_t uiTIMxCLK;

    bsp_RCC_TIM_Enable(TIMx);

    /*-----------------------------------------------------------------------
        The clock configuration of the void SystemClock_Config(void) function in the bsp.c file is as follows:

        System Clock source       = PLL (HSE)
        SYSCLK(Hz)                = 400000000 (CPU Clock)
        HCLK(Hz)                  = 200000000 (AXI and AHBs Clock)
        AHB Prescaler             = 2
        D1 APB3 Prescaler         = 2 (APB3 Clock  100MHz)
        D2 APB1 Prescaler         = 2 (APB1 Clock  100MHz)
        D2 APB2 Prescaler         = 2 (APB2 Clock  100MHz)
        D3 APB4 Prescaler         = 2 (APB4 Clock  100MHz)

        APB1 TIMxCLK = APB1 x 2 = 200MHz;
        APB2 TIMxCLK = APB2 x 2 = 200MHz;
        APB4 TIMCLK 100MHz;

        APB1 TIMER: TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14 LPTIM1
        APB2 TIMER: TIM1, TIM8, TIM15, TIM16 TIM17

        APB4 TIMER: LPTIM2 LPTIM3 LPTIM4 LPTIM5
    ----------------------------------------------------------------------- */
    if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM15) || (TIMx == TIM16) || (TIMx == TIM17))
    {
        /* APB2 TIMER clock = 200M */
        uiTIMxCLK = SystemCoreClock / 2;
    }
    else	
    {
        /* APB1 TIMER clock = 200M */
        uiTIMxCLK = SystemCoreClock / 2;
    }

    if (_ulFreq < 100)
    {
        usPrescaler = 10000 - 1;                      /* Prescaler = 10000 */
        usPeriod = (uiTIMxCLK / 10000) / _ulFreq - 1; /* reload value */
    }
    else if (_ulFreq < 3000)
    {
        usPrescaler = 100 - 1;                      /* Prescaler = 100 */
        usPeriod = (uiTIMxCLK / 100) / _ulFreq - 1; /* reload value */
    }
    else /* frequencies greater than 4K, no frequency division is required */
    {
        usPrescaler = 0;                    /* Prescaler = 1 */
        usPeriod = uiTIMxCLK / _ulFreq - 1; /* reload value */
    }

    /* 
        timer interrupt update cycle = TIMxCLK / (usPrescaler + 1) / (usPeriod + 1)
    */
    TimHandle.Instance = TIMx;
    TimHandle.Init.Prescaler         = usPrescaler;
    TimHandle.Init.Period            = usPeriod;	
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /* enable timer interrupt  */
    __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);


    /* configure TIM update cycle interrupt (Update) */
    {
        uint8_t irq = 0;

        if (TIMx == TIM1) irq = TIM1_UP_IRQn;
        else if (TIMx == TIM2) irq = TIM2_IRQn;
        else if (TIMx == TIM3) irq = TIM3_IRQn;
        else if (TIMx == TIM4) irq = TIM4_IRQn;
        else if (TIMx == TIM5) irq = TIM5_IRQn;
        else if (TIMx == TIM6) irq = TIM6_DAC_IRQn;
        else if (TIMx == TIM7) irq = TIM7_IRQn;
        else if (TIMx == TIM8) irq = TIM8_UP_TIM13_IRQn;
        else if (TIMx == TIM12) irq = TIM8_BRK_TIM12_IRQn;
        else if (TIMx == TIM13) irq = TIM8_UP_TIM13_IRQn;
        else if (TIMx == TIM14) irq = TIM8_TRG_COM_TIM14_IRQn;
        else if (TIMx == TIM15) irq = TIM15_IRQn;
        else if (TIMx == TIM16) irq = TIM16_IRQn;
        else if (TIMx == TIM17) irq = TIM17_IRQn;
        else
        {
            Error_Handler(__FILE__, __LINE__);
        }	
        HAL_NVIC_SetPriority((IRQn_Type)irq, _PreemptionPriority, _SubPriority);
        HAL_NVIC_EnableIRQ((IRQn_Type)irq);		
    }

    HAL_TIM_Base_Start(&TimHandle);
}

