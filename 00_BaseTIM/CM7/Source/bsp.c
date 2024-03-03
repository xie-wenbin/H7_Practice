/*
*********************************************************************************************************
*
*   [module   ] : BSP (For STM32H7)
*	[file name] : bsp.c
*	[version  ] : V1.0
*	[brief    ] : This is the main file for the underlying drivers of the hardware. 
*                Each c file can #include "bsp.h" to include all peripheral driver modules.
*			     bsp = Borad surport packet
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp.h"


/*
*********************************************************************************************************
*	                                   function declaration
*********************************************************************************************************
*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config(void);


/**
 * @brief Enable GPIO clock
 * 
 * @param GPIOx GPIOA-GPIOK
 */
void bsp_RCC_GPIO_Enable(GPIO_TypeDef* GPIOx)
{
    if (GPIOx == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (GPIOx == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (GPIOx == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (GPIOx == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (GPIOx == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (GPIOx == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (GPIOx == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (GPIOx == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
    else if (GPIOx == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();
    else if (GPIOx == GPIOJ) __HAL_RCC_GPIOJ_CLK_ENABLE();
    else if (GPIOx == GPIOK) __HAL_RCC_GPIOK_CLK_ENABLE();
}

/**
 * @brief config GPIO Port Output
 * 
 * @param GPIOx 
 * @param GPIO_PinX 
 */
void bsp_ConfigGpioOut(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX)
{
    GPIO_InitTypeDef   GPIO_InitStruct;

    bsp_RCC_GPIO_Enable(GPIOx);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = GPIO_PinX;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
/**
 * @brief initialize all hardware devices. It only need to be invorked once
 * @param None
 * @retval None
 */
void bsp_Init(void)
{
#ifdef USE_CORE_CM7_BSP
    /* Configure the MPU attributes */
    MPU_Config();

    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    /* STM32H7xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
    HAL_Init();

    /* Configure the system clock to 400 MHz */
    SystemClock_Config();
#endif

#ifdef USE_CORE_CM4_BSP
    /* STM32H7xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
    HAL_Init();
#endif
    
#if Enable_EventRecorder == 1  
    EventRecorderInitialize(EventRecordAll, 1U);
    EventRecorderStart();
#endif

    /* add user bsp initial functions*/
    bsp_InitKey();
    bsp_InitTimer();
    bsp_InitAllLed();
    bsp_InitUart();

}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (Cortex-M7 CPU Clock)
  *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus matrix Clocks)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    HAL_StatusTypeDef ret = HAL_OK;
    
    /*!< Supply configuration update enable */
    HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

    /* The voltage scaling allows optimizing the power consumption when the device is
      clocked below the maximum system frequency, to update the voltage scaling value
      regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
    
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 160;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;  
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;

    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if(ret != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
  /* Select PLL as system clock source and configure  bus clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                  RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
    if(ret != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
}

/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU as Strongly ordered for not defined regions */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x00;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
* @brief  CPU L1-Cache enable.
* @param  None
* @retval None
*/
static void CPU_CACHE_Enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void Error_Handler(char *file, uint32_t line)
{
    /* 
      User can add his own implementation to report the file name and line number
      printf("Wrong parameters value: file %s on line %d\r\n", file, line) 
    */
   printf("Wrong parameters value: file %s on line %d\r\n", file, line);
    
    /* Infinite loop */
    if (line == 0)
    {
      return;
    }
    
    while(1)
    {
    }
}

void bsp_RunPer10ms(void)
{
    bsp_KeyScan10ms();
}


void bsp_RunPer1ms(void)
{
	
}
