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
#include "bsp.h"
#include "bsp_timer.h"

/* These two global variables are used in the bsp DelayMS() function */
static volatile uint32_t s_gulDelayCount = 0;
static volatile uint8_t s_gucTimeOutFlag = 0;

static SOFT_TMR s_tTmr[SOFTWARE_TMR_COUNT] = {0};

static volatile uint8_t s_gucEnableSystickISR = 0;

/**
 * @brief configure the systick interrupt, initialize the software timer variable
 * 
 */
void bsp_InitTimer(void)
{
    uint8_t iLoop = 0;

    for (iLoop = 0; iLoop < SOFTWARE_TMR_COUNT; iLoop++)
    {
        s_tTmr[iLoop].Count = 0;
        s_tTmr[iLoop].PreLoad = 0;
        s_tTmr[iLoop].Flag = 0;
        s_tTmr[iLoop].Mode = TMR_ONCE_MODE;
    }

    /* 
        Set the systic interrupt period to 1ms and enable the systick interrupt.

        SysTick_Config() function parameters:
            -- SystemCoreClock / 1000  Indicates the timing frequency is 1000Hz, timing period  1ms
            -- SystemCoreClock / 500   Indicates the timing frequency is 500Hz， timing period  2ms
            -- SystemCoreClock / 2000  Indicates the timing frequency is 2000Hz，timing period  500us
    */
    /* HAL_InitTick() has set systick period to 1ms , so we no need to set it */

    //SysTick_Config(SystemCoreClock / 1000);

    s_gucEnableSystickISR = 1;

}

/**
 * @brief imprecise ms delay, delay accuracy is plus or minus 1ms
 * 
 * @param ms :delay time, should greater than 2
 */
void bsp_DelayMS(uint32_t ms)
{
    if (ms == 0)
    {
        return;
    }
    else if (ms == 1)
    {
        ms = 2;
    }

    DISABLE_INT(); 

    s_gulDelayCount = ms;
    s_gucTimeOutFlag = 0;

    ENABLE_INT(); 

    while (1)
    {
        if (s_gucTimeOutFlag == 1)
        {
            break;
        }
    }
}

/**
 * @brief class us delay, must be called after the systick timer has started.
 * 
 * @param us 
 */
void bsp_DelayUS(uint32_t us)
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;
       
	reload = SysTick->LOAD;                
    ticks = us * (SystemCoreClock / 1000000);	 /* us trans to systicks */  
    
    tcnt = 0;
    told = SysTick->VAL;             /* previous systicks */

    while (1)
    {
        tnow = SysTick->VAL;    
        if (tnow != told)
        {    
            /* SYSTICK is a decrement counter */    
            if (tnow < told)
            {
                tcnt += told - tnow;    
            }
            /* reload systicks counter */
            else
            {
                tcnt += reload - tnow + told;    
            }        
            told = tnow;

            if (tcnt >= ticks)
            {
            	break;
            }
        }  
    }
}

/**
 * @brief start a one shot software timer, setting a period of this timer
 * 
 * @param timerId :timer id
 * @param period :timer period, 1ms timing cycle
 */
void bsp_StartTimer(uint8_t timerId, uint32_t period)
{
    if (timerId >= SOFTWARE_TMR_COUNT)
    {
        return ;
    }

    DISABLE_INT();

    s_tTmr[timerId].Count = period;
    s_tTmr[timerId].PreLoad = period;
    s_tTmr[timerId].Flag = 0;
    s_tTmr[timerId].Mode = TMR_ONCE_MODE;

    ENABLE_INT();
}

/**
 * @brief start a automatically restart software timer,
 * 
 * @param timerId :timer id
 * @param period :timer period, 1ms timing cycle
 */
void bsp_StartAutoTimer(uint8_t timerId, uint32_t period)
{
    if (timerId >= SOFTWARE_TMR_COUNT)
    {
        return ;
    }

    DISABLE_INT();

    s_tTmr[timerId].Count = period;
    s_tTmr[timerId].PreLoad = period;
    s_tTmr[timerId].Flag = 0;
    s_tTmr[timerId].Mode = TMR_AUTO_MODE;

    ENABLE_INT();
}

/**
 * @brief stop a soft timer
 * 
 * @param timerId 
 */
void bsp_StopTimer(uint8_t timerId)
{
    if (timerId >= SOFTWARE_TMR_COUNT)
    {
        return ;
    }

    DISABLE_INT();

    s_tTmr[timerId].Count = 0;
    s_tTmr[timerId].Flag = 0;
    s_tTmr[timerId].Mode = TMR_ONCE_MODE;

    ENABLE_INT();
}

/**
 * @brief check a soft timer whether expires
 * 
 * @param timerId 
 * @return uint8_t : 0 - timeing not expires, 1 - timer expires
 */
uint8_t bsp_CheckTimer(uint8_t timerId)
{
	if (timerId >= SOFTWARE_TMR_COUNT)
	{
		return 0;
	}

	if (s_tTmr[timerId].Flag == 1)
	{
		s_tTmr[timerId].Flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief must be called periodically by SysTick ISR
 * 
 * @param pTimerPtr :soft timer pointer
 */
static void bsp_SoftTimerDec(SOFT_TMR *pTimerPtr)
{
    if (pTimerPtr == NULL)
    {
        return ;
    }

    if (pTimerPtr->Count > 0)
    {
        if (--pTimerPtr->Count == 0)
        {
            pTimerPtr->Flag = 1;

            if(pTimerPtr->Mode == TMR_AUTO_MODE)
            {
                pTimerPtr->Count = pTimerPtr->PreLoad;
            }
        }
    }
}

/**
 * @brief systick interrupt service function
 * 
 */
void SysTick_ISR(void)
{
    static uint8_t s_count = 0;
    uint8_t i = 0;

    /* if systick ISR is not prepared*/
    if (s_gucEnableSystickISR != 1)
    {
        return ;
    }

    if (s_gulDelayCount > 0)
    {
        if (--s_gulDelayCount == 0)
        {
            s_gucTimeOutFlag = 1;
        }
    }

    for (i = 0; i < SOFTWARE_TMR_COUNT; i++)
    {
        bsp_SoftTimerDec(&s_tTmr[i]);
    }

    bsp_RunPer1ms();

    if (++s_count >= 10)
    {
        s_count = 0;

        bsp_RunPer10ms();
    }
}


