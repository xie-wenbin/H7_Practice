/*
*********************************************************************************************************
*
*   [module   ] : LCD (For STM32H7)
*   [file name] : bsp_lcd_hw.c
*   [version  ] : V1.0
*   [brief    ] : dsi ltdc display module, device peripheral initialization
*********************************************************************************************************
*   LCD_TFT 同步时序配置（整理自官方做的一个截图，言简意赅）：
*       ----------------------------------------------------------------------------
*    
*                                                 Total Width
*                             <--------------------------------------------------->
*                       Hsync width HBP             Active Width                HFP
*                             <---><--><--------------------------------------><-->
*                         ____    ____|_______________________________________|____ 
*                             |___|   |                                       |    |
*                                     |                                       |    |
*                         __|         |                                       |    |
*            /|\    /|\  |            |                                       |    |
*             | VSYNC|   |            |                                       |    |
*             |Width\|/  |__          |                                       |    |
*             |     /|\     |         |                                       |    |
*             |  VBP |      |         |                                       |    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |         | / / / / / / / / / / / / / / / / / / / |    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Total    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Heigh    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Active|      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Heigh |      |         |/ / / / / / Active Display Area / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |                                                      |
*             |  VFP |      |                                                      |
*            \|/    \|/_____|______________________________________________________|
*            
*     
*     每个LCD设备都有自己的同步时序值：
*     Horizontal Synchronization (Hsync) 
*     Horizontal Back Porch (HBP)       
*     Active Width                      
*     Horizontal Front Porch (HFP)     
*   
*     Vertical Synchronization (Vsync)  
*     Vertical Back Porch (VBP)         
*     Active Heigh                       
*     Vertical Front Porch (VFP)         
*     
*     LCD_TFT 窗口水平和垂直的起始以及结束位置 :
*     ----------------------------------------------------------------
*   
*     HorizontalStart = (Offset_X + Hsync + HBP);
*     HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
*     VarticalStart   = (Offset_Y + Vsync + VBP);
*     VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);
*
*********************************************************************************************************
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "bsp_lcd_hw.h"

#include "bsp_lcd_svc.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/

#define VSYNC               2 
#define VBP                 20 
#define VFP                 20
#define VACT                LCD_DEFAULT_HEIGHT
#define HSYNC               5
#define HBP                 35
#define HFP                 35
#define HACT                LCD_DEFAULT_WIDTH

/* Variable declaration ------------------------------------------------------------*/
DSI_HandleTypeDef   hlcd_dsi;
DMA2D_HandleTypeDef hlcd_dma2d;
LTDC_HandleTypeDef  hlcd_ltdc;

LCD_DevColorMode_t  gLcdDevColor;

OTM8009A_Object_t  *Lcd_CompObj = NULL;


/* Function declaration ------------------------------------------------------------*/
void LCD_LTDC_Init(void);
static void LCD_LayerInit(uint16_t LayerIndex, uint32_t Address);
void LCD_DSI_Init(void);
void LCD_OTM8009A_IcDriverInit(uint8_t orientation);

int32_t LCD_GetTick(void);
static int32_t DSI_IO_Write(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size);
static int32_t DSI_IO_Read(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size);



void LCD_MspInit(void)
{
    /** @brief Enable the LTDC clock */
    __HAL_RCC_LTDC_CLK_ENABLE();

    /** @brief Toggle Sw reset of LTDC IP */
    __HAL_RCC_LTDC_FORCE_RESET();
    __HAL_RCC_LTDC_RELEASE_RESET();

    /** @brief Enable the DMA2D clock */
    __HAL_RCC_DMA2D_CLK_ENABLE();

    /** @brief Toggle Sw reset of DMA2D IP */
    __HAL_RCC_DMA2D_FORCE_RESET();
    __HAL_RCC_DMA2D_RELEASE_RESET();

    /** @brief Enable DSI Host and wrapper clocks */
    __HAL_RCC_DSI_CLK_ENABLE();

    /** @brief Soft Reset the DSI Host and wrapper */
    __HAL_RCC_DSI_FORCE_RESET();
    __HAL_RCC_DSI_RELEASE_RESET();

    /** @brief NVIC configuration for LTDC interrupt that is now enabled */
    HAL_NVIC_SetPriority(LTDC_IRQn, 9, 0xf);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);

    /** @brief NVIC configuration for DMA2D interrupt that is now enabled */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 9, 0xf);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);

    /** @brief NVIC configuration for DSI interrupt that is now enabled */
    HAL_NVIC_SetPriority(DSI_IRQn, 9, 0xf);
    HAL_NVIC_EnableIRQ(DSI_IRQn);
}

/**
 * @brief  BSP LCD Reset
 *         Hw reset the LCD DSI activating its XRES signal (active low for some time)
 *         and deactivating it later.
 * @param  Instance LCD Instance
 */
void BSP_LCD_Reset(uint32_t Instance)
{
    GPIO_InitTypeDef gpio_init_structure;

    LCD_RESET_GPIO_CLK_ENABLE();

    /* Configure the GPIO Reset pin */
    gpio_init_structure.Pin = LCD_RESET_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(LCD_RESET_GPIO_PORT, &gpio_init_structure);

    /* Activate XRES active low */
    HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT, LCD_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);                                                       /* wait 20 ms */
    HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT, LCD_RESET_PIN, GPIO_PIN_SET); /* Deactivate XRES */
    HAL_Delay(10);                                                       /* Wait for 10ms after releasing XRES before sending commands */
}


/**
 * @brief Pixel clock = lane rate x number of lanes / bit_per_pixel 
 *        In 24-bit color mode with two data lanes at 500 Mbit/s each, the maximum supported LTDC 
 *        pixel clock is 500 mbs x 2 / 24 = 41.7 MHz.
 * 
 */
void LCD_LTDC_Init(void)
{
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct = {0};

    /* LCD clock configuration */
    /* PLL3_VCO Input = HSE_VALUE/PLL3M = 5 Mhz */
    /* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 800 Mhz */
    /* PLLLCDCLK = PLL3_VCO Output/PLL3R = 800/19 = 42 Mhz */
    /* LTDC clock frequency = PLLLCDCLK = 42 Mhz */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLL3.PLL3M = 5;    
    PeriphClkInitStruct.PLL3.PLL3N = 160;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;  
    PeriphClkInitStruct.PLL3.PLL3R = 19;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    /* DeInit */
    hlcd_ltdc.Instance = LTDC;
    HAL_LTDC_DeInit(&hlcd_ltdc);

    /* LTDC Config */
    /* Timing and polarity */
    hlcd_ltdc.Init.HorizontalSync = HSYNC;
    hlcd_ltdc.Init.VerticalSync = VSYNC;
    hlcd_ltdc.Init.AccumulatedHBP = HSYNC+HBP;
    hlcd_ltdc.Init.AccumulatedVBP = VSYNC+VBP;
    hlcd_ltdc.Init.AccumulatedActiveH = VSYNC+VBP+VACT;
    hlcd_ltdc.Init.AccumulatedActiveW = HSYNC+HBP+HACT;
    hlcd_ltdc.Init.TotalHeigh = VSYNC+VBP+VACT+VFP;
    hlcd_ltdc.Init.TotalWidth = HSYNC+HBP+HACT+HFP;

    /* background value */
    hlcd_ltdc.Init.Backcolor.Blue = 20;
    hlcd_ltdc.Init.Backcolor.Green = 40;
    hlcd_ltdc.Init.Backcolor.Red = 60;

    /* Polarity */
    hlcd_ltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hlcd_ltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hlcd_ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hlcd_ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    hlcd_ltdc.Instance = LTDC;

    HAL_LTDC_Init(&hlcd_ltdc);
}

/**
 * @brief  Initializes the LCD layers.
 * @param  LayerIndex: Layer foreground or background
 * @param  FB_Address: Layer frame buffer
 * @retval None
 */
static void LCD_LayerInit(uint16_t LayerIndex, uint32_t Address)
{
    LTDC_LayerCfgTypeDef layercfg;

    /* Layer Init */
    layercfg.WindowX0 = 0;
    layercfg.WindowX1 = LCD_DEFAULT_WIDTH;
    layercfg.WindowY0 = 0;
    layercfg.WindowY1 = LCD_DEFAULT_HEIGHT;
    layercfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
    layercfg.FBStartAdress = Address;
    layercfg.Alpha = 255;
    layercfg.Alpha0 = 0;
    layercfg.Backcolor.Blue = 0;
    layercfg.Backcolor.Green = 0;
    layercfg.Backcolor.Red = 0;
    layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layercfg.ImageWidth = LCD_DEFAULT_WIDTH;
    layercfg.ImageHeight = LCD_DEFAULT_HEIGHT;

    HAL_LTDC_ConfigLayer(&hlcd_ltdc, &layercfg, LayerIndex);

    __HAL_LTDC_LAYER_ENABLE(&hlcd_ltdc, LayerIndex);
}

void LCD_DSI_Init(void)
{
    DSI_PLLInitTypeDef dsiPllInit = {0};
    DSI_CmdCfgTypeDef CmdCfg = {0};
    DSI_LPCmdTypeDef LPCmd = {0};
    DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
    DSI_PHY_TimerTypeDef  PhyTimings = {0};

    /* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
    hlcd_dsi.Instance = DSI;

    HAL_DSI_DeInit(&hlcd_dsi);

    /* The link rate in this example is 500 Mbit/s per lane, the lane_byte_clock is set to 62.5 MHz.*/
    dsiPllInit.PLLNDIV  = 20;
    dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV1;
    dsiPllInit.PLLODF  = DSI_PLL_OUT_DIV1;  

    hlcd_dsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
    hlcd_dsi.Init.TXEscapeCkdiv = 0x4;

    if (HAL_DSI_Init(&hlcd_dsi, &dsiPllInit) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    HostTimeouts.TimeoutCkdiv = 1;
    HostTimeouts.HighSpeedTransmissionTimeout = 0;
    HostTimeouts.LowPowerReceptionTimeout = 0;
    HostTimeouts.HighSpeedReadTimeout = 0;
    HostTimeouts.LowPowerReadTimeout = 0;
    HostTimeouts.HighSpeedWriteTimeout = 0;
    HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
    HostTimeouts.LowPowerWriteTimeout = 0;
    HostTimeouts.BTATimeout = 0;
    if (HAL_DSI_ConfigHostTimeouts(&hlcd_dsi, &HostTimeouts) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    
    /* Configure DSI PHY HS2LP and LP2HS timings */
    PhyTimings.ClockLaneHS2LPTime = 28; //35;
    PhyTimings.ClockLaneLP2HSTime = 33; //35;
    PhyTimings.DataLaneHS2LPTime = 15; //35;
    PhyTimings.DataLaneLP2HSTime = 25; //35;
    PhyTimings.DataLaneMaxReadTime = 0;
    PhyTimings.StopWaitTime = 0; //10;
    HAL_DSI_ConfigPhyTimer(&hlcd_dsi, &PhyTimings);

    if (HAL_DSI_ConfigFlowControl(&hlcd_dsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
    if (HAL_DSI_SetLowPowerRXFilter(&hlcd_dsi, 10000) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
    if (HAL_DSI_ConfigErrorMonitor(&hlcd_dsi, HAL_DSI_ERROR_NONE) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /*Command transmission configuration*/
    /*enable command transmission in LP mode. This is mandatory for some 
    displays during init phase. After display init LP transmission has to be 
    disabled for DCS Long Write commands*/
    LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_ENABLE;
    LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_ENABLE;
    LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_ENABLE;
    LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_ENABLE;
    LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_ENABLE;
    LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_ENABLE;
    LPCmd.LPGenLongWrite        = DSI_LP_GLW_ENABLE;
    LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_ENABLE;
    LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_ENABLE;
    LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_ENABLE;
    LPCmd.LPDcsLongWrite        = DSI_LP_DLW_ENABLE;
    LPCmd.LPMaxReadPacket       = DSI_LP_MRDP_ENABLE;
    LPCmd.AcknowledgeRequest    = DSI_ACKNOWLEDGE_DISABLE;
    if (HAL_DSI_ConfigCommand(&hlcd_dsi, &LPCmd) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /* Adapted command configuration */
    CmdCfg.VirtualChannelID      = 0;
    CmdCfg.ColorCoding           = DSI_RGB888; /*Select DSI host color format*/
    CmdCfg.CommandSize           = HACT;
    CmdCfg.TearingEffectSource   = DSI_TE_DSILINK;
    CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
    /* DSI host Signal polarity. Same polarity between LTDC and DSI except for 
    Data Enable which has opposite polarity*/
    CmdCfg.HSPolarity            = DSI_HSYNC_ACTIVE_HIGH;
    CmdCfg.VSPolarity            = DSI_VSYNC_ACTIVE_HIGH;
    CmdCfg.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
    CmdCfg.VSyncPol              = DSI_VSYNC_FALLING;
    CmdCfg.AutomaticRefresh      = DSI_AR_DISABLE;
    CmdCfg.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;
    if (HAL_DSI_ConfigAdaptedCommandMode(&hlcd_dsi, &CmdCfg) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }

    /* Start DSI */
    HAL_DSI_Start(&hlcd_dsi);

}

void LCD_OTM8009A_IcDriverInit(uint8_t orientation)
{
    OTM8009A_IO_t              IOCtx = {0};
    DSI_LPCmdTypeDef           LPCmd = {0};
    static OTM8009A_Object_t   OTM8009AObj;

    /* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver) */
    IOCtx.Address     = 0;
    IOCtx.GetTick     = LCD_GetTick;
    IOCtx.WriteReg    = DSI_IO_Write;
    IOCtx.ReadReg     = DSI_IO_Read;
    OTM8009A_RegisterBusIO(&OTM8009AObj, &IOCtx);
    Lcd_CompObj = (&OTM8009AObj);
    OTM8009A_Init(Lcd_CompObj, OTM8009A_COLMOD_RGB888, orientation);


    LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_DISABLE;
    LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_DISABLE;
    LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_DISABLE;
    LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_DISABLE;
    LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_DISABLE;
    LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_DISABLE;
    LPCmd.LPGenLongWrite        = DSI_LP_GLW_DISABLE;
    LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_DISABLE;
    LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_DISABLE;
    LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_DISABLE;
    LPCmd.LPDcsLongWrite        = DSI_LP_DLW_DISABLE;
    HAL_DSI_ConfigCommand(&hlcd_dsi, &LPCmd);

    // HAL_DSI_ConfigFlowControl(&hlcd_dsi, DSI_FLOW_CONTROL_BTA);
    // HAL_DSI_ForceRXLowPower(&hlcd_dsi, ENABLE);  
}

int32_t LCD_GetTick(void)
{
    return (int32_t)HAL_GetTick();
}

/**
 * @brief  DCS or Generic short/long write command
 * @param  ChannelNbr Virtual channel ID
 * @param  Reg Register to be written
 * @param  pData pointer to a buffer of data to be write
 * @param  Size To precise command to be used (short or long)
 * @retval BSP status
 */
static int32_t DSI_IO_Write(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Size <= 1U)
    {
        if (HAL_DSI_ShortWrite(&hlcd_dsi, ChannelNbr, DSI_DCS_SHORT_PKT_WRITE_P1, Reg, (uint32_t)pData[Size]) != HAL_OK)
        {
            ret = BSP_ERROR_BUS_FAILURE;
        }
    }
    else
    {
        if (HAL_DSI_LongWrite(&hlcd_dsi, ChannelNbr, DSI_DCS_LONG_PKT_WRITE, Size, (uint32_t)Reg, pData) != HAL_OK)
        {
            ret = BSP_ERROR_BUS_FAILURE;
        }
    }

    return ret;
}

/**
 * @brief  DCS or Generic read command
 * @param  ChannelNbr Virtual channel ID
 * @param  Reg Register to be read
 * @param  pData pointer to a buffer to store the payload of a read back operation.
 * @param  Size  Data size to be read (in byte).
 * @retval BSP status
 */
static int32_t DSI_IO_Read(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
    int32_t ret = BSP_ERROR_NONE;

    if (HAL_DSI_Read(&hlcd_dsi, ChannelNbr, pData, Size, DSI_DCS_SHORT_PKT_READ, Reg, pData) != HAL_OK)
    {
        ret = BSP_ERROR_BUS_FAILURE;
    }

    return ret;
}

void LCD_InitDMA2D(void)
{
    hlcd_dma2d.Init.Mode = DMA2D_R2M;
    hlcd_dma2d.Init.ColorMode = DMA2D_INPUT_ARGB8888;
    hlcd_dma2d.Init.OutputOffset = 0x0;

    hlcd_dma2d.Instance = DMA2D;

    if (HAL_DMA2D_Init(&hlcd_dma2d) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  This function handles DSI Handler.
 * @param  None
 * @retval None
 */
void DSI_IRQHandler(void)
{
    HAL_DSI_IRQHandler(&hlcd_dsi);
}

void LTDC_IRQHandler(void)
{
    HAL_LTDC_IRQHandler(&hlcd_ltdc);
}

void DMA2D_IRQHandler(void)
{
    HAL_DMA2D_IRQHandler(&hlcd_dma2d);
}


/**
 * @brief  Initializes the DSI LCD.
 * The ititialization is done as below:
 *     - DSI PLL ititialization
 *     - DSI ititialization
 *     - LTDC ititialization
 *     - OTM8009A LCD Display IC Driver ititialization
 * @param  None
 * @retval LCD state
 */
void LCD_HwInit(uint8_t orientation)
{
    //LCD_SetLcdContext();

    /* Toggle Hardware Reset of the DSI LCD using
        its XRES signal (active low) */
    BSP_LCD_Reset(0);

    /* Call first MSP Initialize only in case of first initialization
    * This will set IP blocks LTDC, DSI and DMA2D
    * - out of reset
    * - clocked
    * - NVIC IRQ related to IP blocks enabled
    */
    LCD_MspInit();

    LCD_LTDC_Init();
    LCD_DSI_Init();
    LCD_OTM8009A_IcDriverInit(orientation);
    LCD_InitDMA2D();

    /* Disable DSI Wrapper in order to access and configure the LTDC */
    __HAL_DSI_WRAPPER_DISABLE(&hlcd_dsi);

    LCD_LayerInit(LTDC_LAYER_1, LCD_FRAME_BUFFER1);

    /* Enable DSI Wrapper so DSI IP will drive the LTDC */
    __HAL_DSI_WRAPPER_ENABLE(&hlcd_dsi);

    /*Refresh the LCD display*/
    HAL_DSI_Refresh(&hlcd_dsi);  
    
}


/**
  * @brief  Tearing Effect DSI callback.
  * @param  hdsi  pointer to a DSI_HandleTypeDef structure that contains
  *               the configuration information for the DSI.
  * @retval None
  */
void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef *hdsi)
{
  /* Prevent unused argument(s) compilation warning */

  HAL_DSI_Refresh(hdsi);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DSI_TearingEffectCallback could be implemented in the user file
   */
}

/**
  * @brief  End of Refresh DSI callback.
  * @param  hdsi  pointer to a DSI_HandleTypeDef structure that contains
  *               the configuration information for the DSI.
  * @retval None
  */
void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
{
  /* Prevent unused argument(s) compilation warning */

  HAL_DSI_Refresh(hdsi);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DSI_EndOfRefreshCallback could be implemented in the user file
   */
}

/**
  * @brief  Operation Error DSI callback.
  * @param  hdsi  pointer to a DSI_HandleTypeDef structure that contains
  *               the configuration information for the DSI.
  * @retval None
  */
void HAL_DSI_ErrorCallback(DSI_HandleTypeDef *hdsi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdsi);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DSI_ErrorCallback could be implemented in the user file
   */
}

