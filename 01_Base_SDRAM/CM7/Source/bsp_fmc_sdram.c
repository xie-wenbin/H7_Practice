/*
*********************************************************************************************************
*
*   [module   ] : SDRAM (For STM32H7)
*   [file name] : bsp_fmc_sdram.c
*   [version  ] : V1.0
*   [brief    ] : fmc sdram module
*                 STM32H747I-DISCO sdram type:IS42S32800J-6BLI, 32-bit bandwidth, 32MB memory size, 6ns speed(166MHz)
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp_fmc_sdram.h"
#include "bsp.h"

#include "stm32h747i_discovery_errno.h"
#include "is42s32800j.h"


SDRAM_HandleTypeDef g_hsdram[SDRAM_INSTANCES_NUMBER];

void bsp_SDRAM_ControllerInit(SDRAM_HandleTypeDef *hSdram);
static void bsp_SDRAM_GPIOConfig(void);




void bsp_InitExternalSdram(uint32_t Instance)
{
    static IS42S32800J_Context_t pRegMode = {0};

    if(Instance >=SDRAM_INSTANCES_NUMBER)
    {
        Error_Handler(__FILE__, __LINE__);
    }
    else
    {
        /* SDRAM gpio configuration */
        bsp_SDRAM_GPIOConfig();

        /* SDRAM fmc controller configuration */
        bsp_SDRAM_ControllerInit(&g_hsdram[0]);
    
        /* External memory mode register configuration */
        pRegMode.TargetBank      = FMC_SDRAM_CMD_TARGET_BANK2;
        pRegMode.RefreshMode     = IS42S32800J_AUTOREFRESH_MODE_CMD;
        /*
            SDRAM refresh period / Number of rows) * SDRAM clock - 20
            = 64ms / 4096 * 100MHz - 20
            = 1542.5,  1543
        */
        pRegMode.RefreshRate     = REFRESH_COUNT;
        pRegMode.BurstLength     = IS42S32800J_BURST_LENGTH_1;
        pRegMode.BurstType       = IS42S32800J_BURST_TYPE_SEQUENTIAL;
        pRegMode.CASLatency      = IS42S32800J_CAS_LATENCY_3;
        pRegMode.OperationMode   = IS42S32800J_OPERATING_MODE_STANDARD;
        pRegMode.WriteBurstMode  = IS42S32800J_WRITEBURST_MODE_SINGLE;

        /* SDRAM initialization sequence */
        if(IS42S32800J_Init(&g_hsdram[0], &pRegMode) != IS42S32800J_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }
    }
}


void bsp_SDRAM_ControllerInit(SDRAM_HandleTypeDef *hSdram)
{
    FMC_SDRAM_TimingTypeDef sdram_timing = {0};
 
    /* SDRAM device configuration */
    hSdram->Instance = FMC_SDRAM_DEVICE;

    /* SDRAM handle configuration */
    hSdram->Init.SDBank             = FMC_SDRAM_BANK2; /*use BANK2, start address is 0xD0000000U*/
    hSdram->Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hSdram->Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
    hSdram->Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
    hSdram->Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hSdram->Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
    hSdram->Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hSdram->Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2; /* FMC clock 200MHz, 2 frequency division to SDRAM, that is 100MHz*/
    hSdram->Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    hSdram->Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

    /* 
        Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz) ,One SDRAM clock cycle is 10ns
        The following parameter units is 10ns
    */
    sdram_timing.LoadToActiveDelay    = 2; /*20ns, TMRD*/
    sdram_timing.ExitSelfRefreshDelay = 7; /*70ns, TXSR*/
    sdram_timing.SelfRefreshTime      = 4; /*50ns, TRAS*/
    sdram_timing.RowCycleDelay        = 7; /*70ns, TRC*/
    sdram_timing.WriteRecoveryTime    = 2; /*20ns, TWR*/
    sdram_timing.RPDelay              = 2; /*20ns, TRP*/
    sdram_timing.RCDDelay             = 2; /*20ns, TRCD*/

    /* SDRAM controller initialization */
    if(HAL_SDRAM_Init(hSdram, &sdram_timing) != HAL_OK)
    {
        /* Initialization Error */
		Error_Handler(__FILE__, __LINE__);
    }
}

static void bsp_SDRAM_GPIOConfig(void)
{
    GPIO_InitTypeDef gpio_init_structure;

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

/*-- SDRAM GPIO remap -----------------------------------------------------*/
/*
    +-------------------+--------------------+--------------------+--------------------+
    +                       SDRAM pins assignment                                      +
    +-------------------+--------------------+--------------------+--------------------+
    | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0 <-> FMC_A10    |
    | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1 <-> FMC_A11    |
    | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG4 <-> FMC_A14(BA0)|
    | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG5 <-> FMC_A15(BA1)|
    | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    | PG8 <-> FC_SDCLK   |
    | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    | PG15 <-> FMC_NCAS  |
    | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FC_NRAS   |--------------------+
    +-------------------| PE12 <-> FMC_D9    | PF12 <-> FMC_A6    | PG2  --- FMC_A12
                        | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |
                        | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |
                        | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |
    +-------------------+--------------------+--------------------+
    | PH7 <-> FMC_SDCKE1| PI4 <-> FMC_NBL2   |
    | PH6 <-> FMC_SDNE1 | PI5 <-> FMC_NBL3   |
    | PH5 <-> FMC_SDNWE |--------------------+
    +-------------------+
    +-------------------+------------------+
    +   32-bits Mode: D31-D16              +
    +-------------------+------------------+
    | PH8 <-> FMC_D16   | PI0 <-> FMC_D24  |
    | PH9 <-> FMC_D17   | PI1 <-> FMC_D25  |
    | PH10 <-> FMC_D18  | PI2 <-> FMC_D26  |
    | PH11 <-> FMC_D19  | PI3 <-> FMC_D27  |
    | PH12 <-> FMC_D20  | PI6 <-> FMC_D28  |
    | PH13 <-> FMC_D21  | PI7 <-> FMC_D29  |
    | PH14 <-> FMC_D22  | PI9 <-> FMC_D30  |
    | PH15 <-> FMC_D23  | PI10 <-> FMC_D31 |
    +------------------+-------------------+

    +-------------------+
    +  Pins remapping   +
    +-------------------+
    | PH5 <-> FMC_SDNWE |
    | PH6 <-> FMC_SDNE1 |
    | PH7 <-> FMC_SDCKE1|
    +-------------------+
*/
    /* Common GPIO configuration */
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = GPIO_AF12_FMC;

    /* GPIOD configuration */
    gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                                GPIO_PIN_14 | GPIO_PIN_15;

    HAL_GPIO_Init(GPIOD, &gpio_init_structure);

    /* GPIOE configuration */
    gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                                GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                GPIO_PIN_15;

    HAL_GPIO_Init(GPIOE, &gpio_init_structure);
    /* GPIOF configuration */
    gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                                GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                GPIO_PIN_15;

    HAL_GPIO_Init(GPIOF, &gpio_init_structure);
    /* GPIOG configuration */
    gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 /*| GPIO_PIN_3 */|\
                                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &gpio_init_structure);

    /* GPIOH configuration */
    gpio_init_structure.Pin   = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |\
                                GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                GPIO_PIN_15;

    HAL_GPIO_Init(GPIOH, &gpio_init_structure);

    /* GPIOI configuration */
    gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |\
                                GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;

    HAL_GPIO_Init(GPIOI, &gpio_init_structure);
}

