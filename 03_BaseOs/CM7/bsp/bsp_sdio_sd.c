/*
*********************************************************************************************************
*
*   [module   ] : SDIO SD(For STM32H7)
*   [file name] : bsp_sdio_sd.c
*   [version  ] : V1.0
*   [brief    ] : SDIO SD module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "bsp_sdio_sd.h"


/* Private variables ---------------------------------------------------------*/
SD_HandleTypeDef hsd_sdmmc;
EXTI_HandleTypeDef hsd_exti;

static uint32_t s_gPinDetect  = SD_DETECT_PIN;

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
/* Is Msp Callbacks registered   */
static uint32_t   IsMspCallbacksValid = 0;
#endif
typedef void (* BSP_EXTI_LineCallback) (void);

/* Private functions ---------------------------------------------------------*/
static void SD_MspInit(SD_HandleTypeDef *hsd);
static void SD_MspDeInit(SD_HandleTypeDef *hsd);

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
static void SD_AbortCallback(SD_HandleTypeDef *hsd);
static void SD_TxCpltCallback(SD_HandleTypeDef *hsd);
static void SD_RxCpltCallback(SD_HandleTypeDef *hsd);
#if (USE_SD_TRANSCEIVER > 0U)
static void SD_DriveTransceiver_1_8V_Callback(FlagStatus status);
#endif
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1)   */

static void SD_EXTI_Callback(void);


/** @defgroup
  * @}
  */
/***************************************************************/
/* Private function implementations                            */
/***************************************************************/

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd  SD handle
  * @retval None
  */
static void SD_MspInit(SD_HandleTypeDef *hsd)
{
    GPIO_InitTypeDef gpio_init_structure;

    if (hsd == &hsd_sdmmc)
    {
        /* Enable SDIO clock */
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        /* Enable GPIOs clock */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        /* Common GPIO configuration */
        gpio_init_structure.Mode = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_structure.Alternate = GPIO_AF12_SDIO1;

#if (USE_SD_BUS_WIDE_4B > 0)
        /* SDMMC GPIO CLKIN PB8, D0 PC8, D1 PC9, D2 PC10, D3 PC11, CK PC12, CMD PD2 */
        /* GPIOC configuration */
        gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
#else
        /* SDMMC GPIO CLKIN PB8, D0 PC8, CK PC12, CMD PD2 */
        /* GPIOC configuration */
        gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_12;
#endif

        HAL_GPIO_Init(GPIOC, &gpio_init_structure);

        /* GPIOD configuration */
        gpio_init_structure.Pin = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOD, &gpio_init_structure);

        /* NVIC configuration for SDIO interrupts */
        HAL_NVIC_SetPriority(SDMMC1_IRQn, 14, 0);
        HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    }
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd  SD handle
  * @retval None
  */
static void SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    GPIO_InitTypeDef gpio_init_structure;

    if (hsd == &hsd_sdmmc)
    {
        HAL_NVIC_DisableIRQ(SDMMC1_IRQn);

        /* DeInit GPIO pins can be done in the application
        (by surcharging this __weak function) */

        /* Disable SDMMC1 clock */
        __HAL_RCC_SDMMC1_CLK_DISABLE();

#if (USE_SD_BUS_WIDE_4B > 0)
        /* GPIOJ configuration */
        gpio_init_structure.Pin = GPIO_PIN_14;
        HAL_GPIO_DeInit(GPIOJ, gpio_init_structure.Pin);

        /* GPIOC configuration */
        gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
#else
        gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_12;
#endif
        HAL_GPIO_DeInit(GPIOC, gpio_init_structure.Pin);

        /* GPIOD configuration */
        gpio_init_structure.Pin = GPIO_PIN_2;
        HAL_GPIO_DeInit(GPIOD, gpio_init_structure.Pin);
    }
}



/**
  * @brief  SD EXTI line detection callbacks.
  * @retval None
  */
static void SD_EXTI_Callback(void)
{
    uint32_t sd_status = SD_PRESENT;

    sd_status = BSP_SD_IsDetected();
    
    BSP_SD_DetectCallback(sd_status);
}

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
/**
  * @brief SD Abort callbacks
  * @param hsd  SD handle
  * @retval None
  */
static void SD_AbortCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_AbortCallback();
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
static void SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_WriteCpltCallback();
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
static void SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_ReadCpltCallback();
}
#endif


#if !defined (USE_HAL_SD_REGISTER_CALLBACKS) || (USE_HAL_SD_REGISTER_CALLBACKS == 0)
/**
  * @brief SD Abort callbacks
  * @param hsd  SD handle
  * @retval None
  */
void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_AbortCallback();
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_WriteCpltCallback();
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd  SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_ReadCpltCallback();
}

/**
  * @brief Error callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    BSP_SD_ErrorCallback();
}

#if (USE_SD_TRANSCEIVER != 0U)
/**
  * @brief  Enable the SD Transceiver 1.8V Mode Callback.
  */
void HAL_SD_DriveTransciver_1_8V_Callback(FlagStatus status)
{
    BSP_SD_DriveTransciver_1_8V_Callback(status);
}
#endif
#endif /* !defined (USE_HAL_SD_REGISTER_CALLBACKS) || (USE_HAL_SD_REGISTER_CALLBACKS == 0)   */


/** @defgroup
  * @}
  */
/***************************************************************/
/* Exported Functions implementations                          */
/***************************************************************/

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP SD Msp Callbacks
  * @param None
  * @retval BSP status
  */
int32_t BSP_SD_RegisterDefaultMspCallbacks()
{
    int32_t ret = BSP_ERROR_NONE;

    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_MSP_INIT_CB_ID, SD_MspInit) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_MSP_DEINIT_CB_ID, SD_MspDeInit) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
        IsMspCallbacksValid = 1U;
    }
    /* Return BSP status */
    return ret;
}

/**
  * @brief BSP SD Msp Callback registering
  * @param Instance     SD Instance
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_SD_RegisterMspCallbacks(BSP_SD_Cb_t *CallBacks)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_MSP_INIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_MSP_DEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
        IsMspCallbacksValid = 1U;
    }

    /* Return BSP status */
    return ret;
}
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1) */


/*******************************************************************************************************/
/*                     BSP Functions implementations                                                   */
/*******************************************************************************************************/

/**
  * @brief  Initializes the SDMMC1 peripheral.
  * @param  hsd SD handle
  * @retval HAL status
  */
HAL_StatusTypeDef BSP_SD_InitInstance(SD_HandleTypeDef *hsd)
{
    HAL_StatusTypeDef ret = HAL_OK;
    /* uSD device interface configuration */
    hsd->Instance            = SDMMC1;
    hsd->Init.ClockEdge      = SDMMC_CLOCK_EDGE_RISING;
    hsd->Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
#if (USE_SD_HIGH_PERFORMANCE > 0)
    hsd->Init.ClockDiv       = SDMMC_HSpeed_CLK_DIV;
#else
    hsd->Init.ClockDiv       = SDMMC_NSpeed_CLK_DIV;
#endif /*USE_SD_HIGH_PERFORMANCE*/

#if (USE_SD_BUS_WIDE_4B > 0)
    hsd->Init.BusWide        = SDMMC_BUS_WIDE_4B;
#else
    hsd->Init.BusWide        = SDMMC_BUS_WIDE_1B;
#endif

#if (USE_SD_TRANSCEIVER > 0)
    hsd->Init.TranceiverPresent   = SDMMC_TRANSCEIVER_PRESENT;
#endif /*USE_SD_TRANSCEIVER*/

    hsd->Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;

    /* HAL SD initialization   */
    if (HAL_SD_Init(hsd) != HAL_OK)
    {
        ret = HAL_ERROR;
    }

    return ret;
}

/**
  * @brief  Initializes the SD card device.
  * @param  None
  * @retval BSP status
  */
int32_t BSP_SD_Init(void)
{
    int32_t ret = BSP_ERROR_NONE;
    GPIO_InitTypeDef gpio_init_structure;

    /* Configure Input mode for SD detection pin */
    SD_DETECT_GPIO_CLK_ENABLE();
    gpio_init_structure.Pin = SD_DETECT_PIN;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(SD_DETECT_GPIO_PORT, &gpio_init_structure);

    if (BSP_SD_IsDetected() != SD_PRESENT)
    {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
        /* Register the SD MSP Callbacks */
        if (IsMspCallbacksValid == 0UL)
        {
            if (BSP_SD_RegisterDefaultMspCallbacks() != BSP_ERROR_NONE)
            {
                ret = BSP_ERROR_PERIPH_FAILURE;
            }
        }
#else
        HAL_SD_DeInit(&hsd_sdmmc);
        /* Msp SD initialization */
        SD_MspInit(&hsd_sdmmc);
#endif /* USE_HAL_SD_REGISTER_CALLBACKS   */

        if (ret == BSP_ERROR_NONE)
        {
            /* HAL SD initialization and Enable wide operation   */
            if (BSP_SD_InitInstance(&hsd_sdmmc) != HAL_OK)
            {
                ret = BSP_ERROR_PERIPH_FAILURE;
            }
#if (USE_SD_BUS_WIDE_4B > 0)
            else if (HAL_SD_ConfigWideBusOperation(&hsd_sdmmc, SDMMC_BUS_WIDE_4B) != HAL_OK)
            {
                ret = BSP_ERROR_PERIPH_FAILURE;
            }
#endif
            else
            {
                /* Switch to High Speed mode if the card support this mode */
                (void)HAL_SD_ConfigSpeedBusOperation(&hsd_sdmmc, SDMMC_SPEED_MODE_HIGH);

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
                /* Register SD TC, HT and Abort callbacks */
                if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_TX_CPLT_CB_ID, SD_TxCpltCallback) != HAL_OK)
                {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_RX_CPLT_CB_ID, SD_RxCpltCallback) != HAL_OK)
                {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else if (HAL_SD_RegisterCallback(&hsd_sdmmc, HAL_SD_ABORT_CB_ID, SD_AbortCallback) != HAL_OK)
                {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else
                {
#if (USE_SD_TRANSCEIVER != 0U)
                    if (HAL_SD_RegisterTransceiverCallback(&hsd_sdmmc, SD_DriveTransceiver_1_8V_Callback) != HAL_OK)
                    {
                        ret = BSP_ERROR_PERIPH_FAILURE;
                    }
#endif
                }
#endif /* USE_HAL_SD_REGISTER_CALLBACKS */
            }
        }
    }
    return ret;
}

/**
  * @brief  DeInitializes the SD card device.
  * @param None
  * @retval SD status
  */
int32_t BSP_SD_DeInit(void)
{
    int32_t ret = BSP_ERROR_NONE;
    GPIO_InitTypeDef gpio_init_structure;

    if (HAL_SD_DeInit(&hsd_sdmmc) != HAL_OK) /* HAL SD de-initialization */
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
        /* SD detection pin configuration */
        gpio_init_structure.Pin = SD_DETECT_PIN;
        HAL_GPIO_DeInit(SD_DETECT_GPIO_PORT, gpio_init_structure.Pin);
        /* Msp SD de-initialization */
#if (USE_HAL_SD_REGISTER_CALLBACKS == 0)
        SD_MspDeInit(&hsd_sdmmc);
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 0) */
    }

    return ret;
}


/**
  * @brief  Configures Interrupt mode for SD detection pin.
  * @param  None
  * @retval BSP status
  */
int32_t BSP_SD_RegisterDetectIntr(void)
{
    int32_t ret;
    GPIO_InitTypeDef gpio_init_structure;
    const uint32_t SD_EXTI_LINE = SD_DETECT_EXTI_LINE;
    static BSP_EXTI_LineCallback SdCallback = SD_EXTI_Callback;

    gpio_init_structure.Pin = s_gPinDetect;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Mode = GPIO_MODE_IT_RISING_FALLING;
    HAL_GPIO_Init(SD_DETECT_GPIO_PORT, &gpio_init_structure);

    /* Enable and set SD detect EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((IRQn_Type)(SD_DETECT_EXTI_IRQn), 0x0F, 0x00);
    HAL_NVIC_EnableIRQ((IRQn_Type)(SD_DETECT_EXTI_IRQn));
    HAL_EXTI_GetHandle(&hsd_exti, SD_EXTI_LINE);

    if (HAL_EXTI_RegisterCallback(&hsd_exti, HAL_EXTI_COMMON_CB_ID, SdCallback) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
        ret = BSP_ERROR_NONE;
    }

    /* Return BSP status */
    return ret;
}


/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
  * @param None
 * @retval Returns if SD is detected or not
 */
int32_t BSP_SD_IsDetected(void)
{
    int32_t ret = BSP_ERROR_UNKNOWN_FAILURE;

    ret = (uint32_t)HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, GPIO_PIN_8); /*PinDetect[Instance]*/
    /* Check SD card detect pin */
    if (ret != GPIO_PIN_RESET)
    {
        ret = (int32_t)SD_NOT_PRESENT;
    }
    else
    {
        ret = (int32_t)SD_PRESENT;
    }
    return ret;
}


/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockAddr   Block address from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @param  Timeout    Timeout for read operation
  * @retval BSP status
  */
int32_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr, uint32_t Timeout)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_ReadBlocks(&hsd_sdmmc, (uint8_t *)pData, BlockAddr, BlocksNbr, Timeout) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockAddr   Block address from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval BSP status
  */
int32_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr, uint32_t Timeout)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_WriteBlocks(&hsd_sdmmc, (uint8_t *)pData, BlockAddr, BlocksNbr, Timeout) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockAddr   Block address from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval BSP status
  */
int32_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_ReadBlocks_DMA(&hsd_sdmmc, (uint8_t *)pData, BlockAddr, BlocksNbr) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockAddr   Block address from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval BSP status
  */
int32_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_WriteBlocks_DMA(&hsd_sdmmc, (uint8_t *)pData, BlockAddr, BlocksNbr) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockAddr   Block address from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval SD status
  */
int32_t BSP_SD_ReadBlocks_IT(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_ReadBlocks_IT(&hsd_sdmmc, (uint8_t *)pData, BlockAddr, BlocksNbr) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockAddr   Block address from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval SD status
  */
int32_t BSP_SD_WriteBlocks_IT(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_WriteBlocks_IT(&hsd_sdmmc, (uint8_t *)pData, BlockAddr, BlocksNbr) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}


/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  Instance   SD Instance
  * @param  BlockIdx   Block address from where data is to be
  * @param  BlocksNbr  Number of SD blocks to erase
  * @retval SD status
  */
int32_t BSP_SD_Erase(uint32_t BlockAddr, uint32_t BlocksNbr)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_Erase(&hsd_sdmmc, BlockAddr, BlockAddr + BlocksNbr) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status   */
    return ret;
}

/**
  * @brief  Gets the current SD card data status.
  * @param  None
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
int32_t BSP_SD_GetCardState(void)
{
  return (int32_t)((HAL_SD_GetCardState(&hsd_sdmmc) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo  Pointer to HAL_SD_CardInfoTypedef structure
  * @retval BSP status
  */
int32_t BSP_SD_GetCardInfo(BSP_SD_CardInfo *CardInfo)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_SD_GetCardInfo(&hsd_sdmmc, CardInfo) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Return BSP status */
    return ret;
}


/*$$*****************************************************************************************************$$*/
/*$$                     user Functions to be implementations                                            $$*/
/*$$*****************************************************************************************************$$*/
/**
  * @brief  BSP SD Callback.
  * @param  Status   Pin status
  * @retval None.
  */
__weak void BSP_SD_DetectCallback(uint32_t Status)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Status);

  /* This function should be implemented by the user application. */
}

/**
  * @brief BSP SD Abort callbacks
  * @param  Instance     SD Instance
  * @retval None
  */
__weak void BSP_SD_AbortCallback(void)
{
    /* This function should be implemented by the user application. */

}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param  Instance     SD Instance
  * @retval None
  */
__weak void BSP_SD_WriteCpltCallback(void)
{
    /* This function should be implemented by the user application. */

}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @param  Instance     SD Instance
  * @retval None
  */
__weak void BSP_SD_ReadCpltCallback(void)
{
    /* This function should be implemented by the user application. */

}

/**
  * @brief BSP Error callbacks
  * @retval None
  */
__weak void BSP_SD_ErrorCallback(void)
{

}

/**
  * @brief  BSP SD Transceiver 1.8V Mode Callback.
  */
__weak void BSP_SD_DriveTransciver_1_8V_Callback(FlagStatus status)
{
#if (USE_BSP_IO_CLASS > 0U)
    if (status == SET)
    {
        BSP_IO_WritePin(0, SD_LDO_SEL_PIN, IO_PIN_SET);
    }
    else
    {
        BSP_IO_WritePin(0, SD_LDO_SEL_PIN, IO_PIN_RESET);
    }
#endif
}


/**
  * @brief  This function handles pin detection interrupt request.
  * @param  None
  * @retval None
  */
void BSP_SD_DETECT_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&hsd_exti);
}

/**
  * @brief  This function handles SDMMC interrupt requests.
  * @param  None
  * @retval None
  */
void BSP_SD_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hsd_sdmmc);
}

