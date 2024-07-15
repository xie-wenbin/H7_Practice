/*
*********************************************************************************************************
*
*   [module   ] : QSPI FLASH(For STM32H7)
*   [file name] : bsp_qspi_flash.c
*   [version  ] : V1.0
*   [brief    ] : QSPI FLASH module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "bsp_qspi_flash.h"

/* Private variables ---------------------------------------------------------*/

QSPI_HandleTypeDef hqspi;
BSP_QSPI_Ctx_t     gqspi_ctx;

/* Private functions ---------------------------------------------------------*/
static void QSPI_MspInit(QSPI_HandleTypeDef *hQspi);
static void QSPI_MspDeInit(QSPI_HandleTypeDef *hQspi);
static int32_t QSPI_ResetMemoryChip(void);
static int32_t QSPI_DummyCyclesCfg(void);
int32_t BSP_QSPI_ConfigFlash(BSP_QSPI_Interface_e Mode, BSP_QSPI_Transfer_e Rate);

/**
  * @}
  */
/**
  * @brief  Initializes the QSPI interface.
  *         Instance management :
  *         Instance can running at Indirect or MMP mode.
  *         Every indirect instance can switch between.
  *         Once MMP instance active then Instance switch function disabled.
  *         Only 1 MMP instance can run.
  *         Instance switch function enabled after MMP instance exit to Indirect mode.
  *
  *         MT25TL01G action :
  *         QE(Quad Enable, Non-volatile) bit of Status Register
  *         QE = 0; WP# & RESET# pin active
  *                 Accept 1-1-1, 1-1-2, 1-2-2 commands
  *         QE = 1; WP# become SIO2 pin, RESET# become SIO3 pin
  *                 Accept 1-1-1, 1-1-2, 1-2-2, 1-1-4, 1-4-4 commands
  *         Enter QPI mode by issue EQIO(0x35) command from 1-1-1 mode
  *                 Accept 4-4-4 commands
  *         Exit QPI mode by issue RSTQIO(0xF5) command from 4-4-4 mode
  *                 Accept commands, dependent QE bit status
  *         Memory Read commands support STR(Single Transfer Rate) & DTR(Double Transfer Rate) mode
  *
  *         Force QE = 1, Enter 4-Byte address mode
  *         Configure Dummy cycle & ODS(Out Driver Strength)
  *         STR/DTR effect read commands only
  * @param  Init       QSPI Init structure
  * @retval None
  */
void BSP_QSPI_Init(BSP_QSPI_Init_t *Init)
{
    BSP_QSPI_Info_t pInfo = {0};
    uint32_t SampleShifting = 0;
    /* Table to handle clock prescalers:
    1: For STR mode to reach max 108Mhz
    3: For DTR mode to reach max 54Mhz
    */
    static const uint32_t qspiPrescalerTab[2] = {1, 3};

    /* Check if instance is already initialized */
    if (gqspi_ctx.IsInitialized == QSPI_ACCESS_NONE)
    {
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
        /* Register the QSPI MSP Callbacks */
        if (gqspi_ctx.IsMspCallbacksValid == 0UL)
        {
            if (BSP_QSPI_RegisterDefaultMspCallbacks() != BSP_ERROR_NONE)
            {
                ret = BSP_ERROR_PERIPH_FAILURE;
            }
        }
#else
        if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }

        /* Msp QSPI initialization */
        QSPI_MspInit(&hqspi);
#endif /* USE_HAL_QSPI_REGISTER_CALLBACKS */

        /* STM32 QSPI interface initialization */
        (void)MT25TL01G_GetFlashInfo(&pInfo);

        /* QSPI initialization */
        /*
        QUADSPI在FLASH驱动信号后过半个CLK周期才�?�FLASH驱动的数�?采样�?
        在�?�部信号延迟时，这有利于推迟数据采样�?
        */
        SampleShifting = (Init->TransferRate == BSP_QSPI_STR_TRANSFER) ? 
                        QSPI_SAMPLE_SHIFTING_HALFCYCLE : QSPI_SAMPLE_SHIFTING_NONE;

        /* QSPI Freq Input = HCLK3 = SYSCLK / AHBCLKDivider Mhz = 200MHz */
        /* QSPI clock = QSPI Freq /(1 + ClockPrescaler) Mhz = 200MHz / (ClockPrescaler+1) = 100MHz or 50MHz */

        hqspi.Instance                = QUADSPI;
        hqspi.Init.ClockPrescaler     = qspiPrescalerTab[Init->TransferRate];
        hqspi.Init.FifoThreshold      = 32; /* 设置FIFO阀值，范围1 - 32 */
        hqspi.Init.SampleShifting     = SampleShifting;
        hqspi.Init.FlashSize          = (uint32_t)POSITION_VAL((uint32_t)pInfo.FlashSize) - 1U; /*Flash大小�?2^(FlashSize + 1) */
        /* 命令之间的CS片选至少保�?1�?时钟周期的高电平 */
        hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE; /* Min 50ns for nonRead */
        hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_0;
        hqspi.Init.FlashID            = QSPI_FLASH_ID_1; /* QSPI有两个BANK，这里使用的BANK1 */
        hqspi.Init.DualFlash          = QSPI_DUALFLASH_ENABLE;

        if (HAL_QSPI_Init(&hqspi) != HAL_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }

        /* QSPI flash memory chip reset */
        if (QSPI_ResetMemoryChip() != BSP_ERROR_NONE)
        {
            Error_Handler(__FILE__, __LINE__);
        } /* Force Flash enter 4 Byte address mode */
        else if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }
        else if (MT25TL01G_Enter4BytesAddressMode(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        } /* Configuration of the dummy cycles on QSPI memory side */
        else if (QSPI_DummyCyclesCfg() != BSP_ERROR_NONE)
        {
            Error_Handler(__FILE__, __LINE__);
        }
        else
        {
            /* Configure Flash to desired mode */
            if (BSP_QSPI_ConfigFlash(Init->InterfaceMode, Init->TransferRate) != BSP_ERROR_NONE)
            {
                Error_Handler(__FILE__, __LINE__);
            }
        }
    }
}


/**
  * @brief  De-Initializes the QSPI interface.
  * @param  None
  * @retval BSP status
  */
int32_t BSP_QSPI_DeInit(void)
{
    int32_t ret = BSP_ERROR_NONE;

    if (gqspi_ctx.IsInitialized == QSPI_ACCESS_MMP)
    {
        if (BSP_QSPI_DisableMemoryMappedMode() != BSP_ERROR_NONE)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }

    if (ret == BSP_ERROR_NONE)
    {
        /* Set default QSPI_Ctx values */
        gqspi_ctx.IsInitialized = QSPI_ACCESS_NONE;
        gqspi_ctx.InterfaceMode = BSP_QSPI_SPI_MODE;
        gqspi_ctx.TransferRate = BSP_QSPI_STR_TRANSFER;
        gqspi_ctx.DualFlashMode = QSPI_DUALFLASH_ENABLE;

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 0)
        QSPI_MspDeInit(&hqspi);
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 0) */

        /* Call the DeInit function to reset the driver */
        if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }

    /* Return BSP status */
    return ret;
}


/**
  * @}
  */
/***************************************************************/
/* Private function implementations                            */
/***************************************************************/

/**
  * @brief QSPI MSP Initialization
  * @param hQspi : QSPI handle
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - NVIC configuration for QSPI interrupt
  * @retval None
  */
static void QSPI_MspInit(QSPI_HandleTypeDef *hQspi)
{
    GPIO_InitTypeDef gpio_init_structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable the QuadSPI memory interface clock */
    QSPI_CLK_ENABLE();
    /* Reset the QuadSPI memory interface */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();
    /* Enable GPIO clocks */
    QSPI_CLK_GPIO_CLK_ENABLE();
    QSPI_BK1_CS_GPIO_CLK_ENABLE();
    QSPI_BK1_D0_GPIO_CLK_ENABLE();
    QSPI_BK1_D1_GPIO_CLK_ENABLE();
    QSPI_BK1_D2_GPIO_CLK_ENABLE();
    QSPI_BK1_D3_GPIO_CLK_ENABLE();

    QSPI_BK2_CS_GPIO_CLK_ENABLE();
    QSPI_BK2_D0_GPIO_CLK_ENABLE();
    QSPI_BK2_D1_GPIO_CLK_ENABLE();
    QSPI_BK2_D2_GPIO_CLK_ENABLE();
    QSPI_BK2_D3_GPIO_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* QSPI CLK GPIO pin configuration  */
    gpio_init_structure.Pin = QSPI_CLK_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);

    /* QSPI CS GPIO pin configuration  */
    gpio_init_structure.Pin = QSPI_BK1_CS_PIN;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_CS_GPIO_PORT, &gpio_init_structure);

    /* QSPI D0 GPIO pin configuration  */
    gpio_init_structure.Pin = QSPI_BK1_D0_PIN;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = QSPI_BK2_D0_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK2_D0_GPIO_PORT, &gpio_init_structure);

    /* QSPI D1 GPIO pin configuration  */
    gpio_init_structure.Pin = QSPI_BK1_D1_PIN;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = QSPI_BK2_D1_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK2_D1_GPIO_PORT, &gpio_init_structure);

    /* QSPI D2 GPIO pin configuration  */
    gpio_init_structure.Pin = QSPI_BK1_D2_PIN;
    gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = QSPI_BK2_D2_PIN;
    HAL_GPIO_Init(QSPI_BK2_D2_GPIO_PORT, &gpio_init_structure);

    /* QSPI D3 GPIO pin configuration  */
    gpio_init_structure.Pin = QSPI_BK1_D3_PIN;
    HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = QSPI_BK2_D3_PIN;
    HAL_GPIO_Init(QSPI_BK2_D3_GPIO_PORT, &gpio_init_structure);

    /*##-3- Configure the NVIC for QSPI #########################################*/
    /* NVIC configuration for QSPI interrupt */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}

/**
  * @brief QSPI MSP De-Initialization
  * @param hQspi  QSPI handle
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @retval None
  */
static void QSPI_MspDeInit(QSPI_HandleTypeDef *hQspi)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hQspi);

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* De-Configure QSPI pins */
    HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_CS_GPIO_PORT, QSPI_BK1_CS_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

    HAL_GPIO_DeInit(QSPI_BK2_CS_GPIO_PORT, QSPI_BK2_CS_PIN);
    HAL_GPIO_DeInit(QSPI_BK2_D0_GPIO_PORT, QSPI_BK2_D0_PIN);
    HAL_GPIO_DeInit(QSPI_BK2_D1_GPIO_PORT, QSPI_BK2_D1_PIN);
    HAL_GPIO_DeInit(QSPI_BK2_D2_GPIO_PORT, QSPI_BK2_D2_PIN);
    HAL_GPIO_DeInit(QSPI_BK2_D3_GPIO_PORT, QSPI_BK2_D3_PIN);

    /*##-3- Reset peripherals ##################################################*/
    /* Reset the QuadSPI memory interface */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();

    /* Disable the QuadSPI memory interface clock */
    QSPI_CLK_DISABLE();
}

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
/**
  * @brief Default BSP QSPI Msp Callbacks
  * @param None 
  * @retval BSP status
  */
int32_t BSP_QSPI_RegisterDefaultMspCallbacks (void)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPINIT_CB_ID, QSPI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPDEINIT_CB_ID, QSPI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      gqspi_ctx.IsMspCallbacksValid = 1U;
    }

    /* Return BSP status */
    return ret;
}

/**
  * @brief BSP QSPI Msp Callback registering
  * @param CallBacks    pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_QSPI_RegisterMspCallbacks (BSP_QSPI_Cb_t *CallBacks)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPINIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPDEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      gqspi_ctx[Instance].IsMspCallbacksValid = 1U;
    }

    /* Return BSP status */
    return ret;
}
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Set Flash to desired Interface mode. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function isn't work.
  *         Indirect -> Indirect
  * @param  Mode      QSPI mode
  * @param  Rate      QSPI transfer rate
  * @retval BSP status
  */
int32_t BSP_QSPI_ConfigFlash(BSP_QSPI_Interface_e Mode, BSP_QSPI_Transfer_e Rate)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if MMP mode locked ************************************************/
    if (gqspi_ctx.IsInitialized == QSPI_ACCESS_MMP)
    {
        ret = BSP_ERROR_QSPI_MMP_LOCK_FAILURE;
    }
    else
    {
        /* Setup Flash interface ***************************************************/
        switch (gqspi_ctx.InterfaceMode)
        {
        case BSP_QSPI_QPI_MODE: /* 4-4-4 commands */
            if (Mode != MT25TL01G_QPI_MODE)
            {
                if (MT25TL01G_ExitQPIMode(&hqspi) != MT25TL01G_OK)
                {
                    ret = BSP_ERROR_COMPONENT_FAILURE;
                }
            }
            break;

        case BSP_QSPI_SPI_MODE:     /* 1-1-1 commands, Power on H/W default setting */
        case BSP_QSPI_SPI_2IO_MODE: /* 1-2-2 read commands */
        case BSP_QSPI_SPI_4IO_MODE: /* 1-4-4 read commands */
        default:
            if (Mode == MT25TL01G_QPI_MODE)
            {
                if (MT25TL01G_EnterQPIMode(&hqspi) != MT25TL01G_OK)
                {
                    ret = BSP_ERROR_COMPONENT_FAILURE;
                }
            }
            break;
        }

        /* Update QSPI context if all operations are well done */
        if (ret == BSP_ERROR_NONE)
        {
            /* Update current status parameter *****************************************/
            gqspi_ctx.IsInitialized = QSPI_ACCESS_INDIRECT;
            gqspi_ctx.InterfaceMode = Mode;
            gqspi_ctx.TransferRate = Rate;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
  * @brief  This function reset the QSPI Flash memory chip.
  *         Fore QPI+SPI reset to avoid system come from unknown status.
  *         Flash accept 1-1-1, 1-1-2, 1-2-2 commands after reset.
  * @param  None
  * @retval BSP status
  */
static int32_t QSPI_ResetMemoryChip(void)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Send RESET ENABLE command in QPI mode (QUAD I/Os, 4-4-4) */
    if (MT25TL01G_ResetEnable(&hqspi, MT25TL01G_QPI_MODE) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Send RESET memory command in QPI mode (QUAD I/Os, 4-4-4) */
    else if (MT25TL01G_ResetMemory(&hqspi, MT25TL01G_QPI_MODE) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Wait Flash ready */
    else if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Send RESET ENABLE command in SPI mode (1-1-1) */
    else if (MT25TL01G_ResetEnable(&hqspi, BSP_QSPI_SPI_MODE) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Send RESET memory command in SPI mode (1-1-1) */
    else if (MT25TL01G_ResetMemory(&hqspi, BSP_QSPI_SPI_MODE) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        gqspi_ctx.IsInitialized = QSPI_ACCESS_INDIRECT; /* After reset S/W setting to indirect access   */
        gqspi_ctx.InterfaceMode = BSP_QSPI_SPI_MODE;    /* After reset H/W back to SPI mode by default  */
        gqspi_ctx.TransferRate = BSP_QSPI_STR_TRANSFER; /* After reset S/W setting to STR mode          */
    }

    /* Return BSP status */
    return ret;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  *         Dummy cycle bit locate in Configuration Register[7:6]
  * @param  None
  * @retval BSP status
  */
static int32_t QSPI_DummyCyclesCfg(void)
{
    int32_t ret = BSP_ERROR_NONE;
    QSPI_CommandTypeDef s_command;
    uint16_t reg = 0;

    /* Initialize the read volatile configuration register command */
    s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction = MT25TL01G_READ_VOL_CFG_REG_CMD;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_4_LINES;
    s_command.DummyCycles = 0;
    s_command.NbData = 2;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Reception of the data */
    if (HAL_QSPI_Receive(&hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Enable write operations */
    if (MT25TL01G_WriteEnable(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Update volatile configuration register (with new dummy cycles) */
    s_command.Instruction = MT25TL01G_WRITE_VOL_CFG_REG_CMD;
    MODIFY_REG(reg, 0xF0F0, ((MT25TL01G_DUMMY_CYCLES_READ_QUAD << 4) | (MT25TL01G_DUMMY_CYCLES_READ_QUAD << 12)));

    /* Configure the write volatile configuration register command */
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Transmission of the data */
    if (HAL_QSPI_Transmit(&hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Return BSP status */
    return ret;
}

/**
  * @}
  */
/***************************************************************/
/* Exported Functions implementations                          */
/***************************************************************/

/**
  * @brief  Reads current status of the QSPI memory.
  *         If WIP != 0 then return busy.
  * @param  None
  * @retval QSPI memory status: whether busy or not
  */
int32_t BSP_QSPI_GetStatus(void)
{
    int32_t ret = BSP_ERROR_NONE;
    uint8_t reg;

    if(MT25TL01G_ReadStatusRegister(&hqspi, gqspi_ctx.InterfaceMode, &reg) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        /* Check the value of the register */
        if ((reg & MT25TL01G_SR_WIP) != 0U)
        {
            ret = BSP_ERROR_BUSY;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo     pointer on the configuration structure
  * @retval None
  */
void BSP_QSPI_GetChipInfo(BSP_QSPI_Info_t *pInfo)
{
    (void)MT25TL01G_GetFlashInfo(pInfo);
}

/**
  * @brief  Get flash ID, 3 Byte
  *         Manufacturer ID, Memory type, Memory density
  * @param  Id QSPI Identifier
  * @retval BSP status
  */
int32_t BSP_QSPI_ReadChipID(uint8_t *Id)
{
    int32_t ret = BSP_ERROR_NONE;

    if(MT25TL01G_ReadID(&hqspi, gqspi_ctx.InterfaceMode, Id) != MT25TL01G_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Return BSP status */
    return ret;
}


/**
  * @brief  Configure the QSPI in memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @param  Instance  QSPI instance
  * @retval BSP status
  */
int32_t BSP_QSPI_EnableMemoryMappedMode(void)
{
    int32_t ret = BSP_ERROR_NONE;

    if (gqspi_ctx.TransferRate == BSP_QSPI_STR_TRANSFER)
    {
        if (MT25TL01G_EnableMemoryMappedModeSTR(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else /* Update QSPI context if all operations are well done */
        {
            gqspi_ctx.IsInitialized = QSPI_ACCESS_MMP;
        }
    }
    else
    {
        if (MT25TL01G_EnableMemoryMappedModeDTR(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else /* Update QSPI context if all operations are well done */
        {
            gqspi_ctx.IsInitialized = QSPI_ACCESS_MMP;
        }
    }

    /* Return BSP status */
    return ret;
}


/**
  * @brief  Exit form memory-mapped mode
  *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
  * @param  None
  * @retval BSP status
  */
int32_t BSP_QSPI_DisableMemoryMappedMode(void)
{
    uint8_t Dummy;
    int32_t ret = BSP_ERROR_NONE;

    if (gqspi_ctx.IsInitialized != QSPI_ACCESS_MMP)
    {
        ret = BSP_ERROR_QSPI_MMP_UNLOCK_FAILURE;
    } /* Abort MMP back to indirect mode */
    else if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
        /* Force QSPI interface Sampling Shift to half cycle */
        hqspi.Init.SampleShifting = (gqspi_ctx.TransferRate == BSP_QSPI_STR_TRANSFER) ? 
                        QSPI_SAMPLE_SHIFTING_HALFCYCLE : QSPI_SAMPLE_SHIFTING_NONE;//QSPI_SAMPLE_SHIFTING_HALFCYCLE;

        if (HAL_QSPI_Init(&hqspi) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        /* Dummy read for exit from Performance Enhance mode */
        else if (MT25TL01G_ReadSTR(&hqspi, gqspi_ctx.InterfaceMode, &Dummy, 0, 1) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else /* Update QSPI context if all operations are well done */
        {
            gqspi_ctx.IsInitialized = QSPI_ACCESS_INDIRECT;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @param  None 
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseChip(void)
{
    int32_t ret = BSP_ERROR_NONE;

    if (QSPI_ACCESS_MMP == gqspi_ctx.IsInitialized)
    {
        BSP_QSPI_DisableMemoryMappedMode();
    }

    /* Check Flash busy ? */
    if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Enable write operations */
    else if (MT25TL01G_WriteEnable(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        /* Issue Chip erase command */
        if (MT25TL01G_ChipErase(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }

    /* Return BSP status */
    return ret;
}


/**
  * @brief  Erases the specified block of the QSPI memory.
  *         MT25TL01G support 4K, 32K, 64K size block erase commands for each Die.
  *           i.e 8K, 64K, 128K at BSP level (see BSP_QSPI_Erase_t type definition)
  * @param  BlockAddress Block address to erase
  * @param  BlockSize    Erase Block size
  * @retval BSP status
  */
int32_t BSP_QSPI_EraseBlock(uint32_t BlockAddress, BSP_QSPI_Erase_e BlockSize)
{
    int32_t ret = BSP_ERROR_NONE;

    if (QSPI_ACCESS_MMP == gqspi_ctx.IsInitialized)
    {
        BSP_QSPI_DisableMemoryMappedMode();
    }

    /* Check Flash busy ? */
    if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Enable write operations */
    else if (MT25TL01G_WriteEnable(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        /* Issue Block Erase command */
        if (MT25TL01G_BlockErase(&hqspi, gqspi_ctx.InterfaceMode, BlockAddress, (MT25TL01G_Erase_enu)BlockSize) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData     Pointer to data to be read
  * @param  ReadAddr  Read start address
  * @param  Size      Size of data to read
  * @retval BSP status
  */
int32_t BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
    int32_t ret = BSP_ERROR_NONE;

    if (gqspi_ctx.TransferRate == BSP_QSPI_STR_TRANSFER)
    {
        if (MT25TL01G_ReadSTR(&hqspi, gqspi_ctx.InterfaceMode, pData, ReadAddr, Size) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }
    else
    {
        if (MT25TL01G_ReadDTR(&hqspi, gqspi_ctx.InterfaceMode, pData, ReadAddr, Size) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }

    /* Return BSP status */
    return ret;
}


/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t BSP_QSPI_SyncWrite(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
    int32_t ret = BSP_ERROR_NONE;
    uint32_t end_addr, current_size, current_addr;
    uint8_t *write_data;

    /* Calculation of the size between the write address and the end of the page */
    current_size = MT25TL01G_PAGE_SIZE - (WriteAddr % MT25TL01G_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > Size)
    {
        current_size = Size;
    }

    /* Initialize the address variables */
    current_addr = WriteAddr;
    end_addr = WriteAddr + Size;
    write_data = pData;

    if (QSPI_ACCESS_MMP == gqspi_ctx.IsInitialized)
    {
        BSP_QSPI_DisableMemoryMappedMode();
    }

    /* Perform the write page by page */
    do
    {
        /* Check if Flash busy ? */
        if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Enable write operations */
        else if (MT25TL01G_WriteEnable(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Issue page program command */
        else if (MT25TL01G_PageProgram(&hqspi, gqspi_ctx.InterfaceMode, write_data, current_addr, current_size) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Configure automatic polling mode to wait for end of program */
        else if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            /* Update the address and size variables for next page programming */
            current_addr += current_size;
            write_data += current_size;
            current_size = ((current_addr + MT25TL01G_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MT25TL01G_PAGE_SIZE;
        }
    } while ((current_addr < end_addr) && (ret == BSP_ERROR_NONE));

    /* Return BSP status */
    return ret;
}


/**
  * @brief  Writes data to the flash page.
  * @param  pData      Pointer to data to be written
  * @param  WriteAddr  Write start address
  * @param  Size       Size of data to write
  * @retval BSP status
  */
int32_t BSP_QSPI_PageWrite(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
    int32_t ret = BSP_ERROR_NONE;
    uint32_t current_size;

    // /* Calculation of the size between the write address and the end of the page */
    // current_size = MT25TL01G_PAGE_SIZE - (WriteAddr % MT25TL01G_PAGE_SIZE);

    // if (current_size > 0)
    // {
    //     return BSP_ERROR_COMPONENT_FAILURE;
    // }

    /* Check if the size of the data is less than the remaining place in the page */
    if (MT25TL01G_PAGE_SIZE > Size)
    {
        current_size = Size;
    }
    else
    {
        current_size = MT25TL01G_PAGE_SIZE;
    }

    if (QSPI_ACCESS_MMP == gqspi_ctx.IsInitialized)
    {
        BSP_QSPI_DisableMemoryMappedMode();
    }

    /* Check if Flash busy ? */
    if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Enable write operations */
    else if (MT25TL01G_WriteEnable(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Issue page program command */
    else if (MT25TL01G_PageProgram(&hqspi, gqspi_ctx.InterfaceMode, pData, WriteAddr, current_size) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Configure automatic polling mode to wait for end of program */
    else if (MT25TL01G_AutoPollingMemReady(&hqspi, gqspi_ctx.InterfaceMode) != MT25TL01G_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Return BSP status */
    return ret;
}



void bsp_InitQspiFlash_MT25(void)
{
    BSP_QSPI_Init_t init;

    init.InterfaceMode = MT25TL01G_QPI_MODE;
    init.TransferRate  = MT25TL01G_DTR_TRANSFER;
    init.DualFlashMode = MT25TL01G_DUALFLASH_ENABLE;

    BSP_QSPI_Init(&init);
}
