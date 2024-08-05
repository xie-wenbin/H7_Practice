/*
*********************************************************************************************************
*
*   [module   ] : LCD (For STM32H7)
*   [file name] : bsp_lcd_service.c
*   [version  ] : V1.0
*   [brief    ] : lcd draw service interface module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/

#include "bsp_lcd_svc.h"
#include "bsp_lcd_hw.h"

static LCD_Drv_t *Lcd_Drv = NULL;
BSP_LCD_Ctx_t glcd_ctx;

/*********************************************************************************************/
/******                         Lcd Property Functions                                 *******/
/*********************************************************************************************/
/**
  * @brief  Set the LCD Active Layer.
  * @param  Instance    LCD Instance
  * @param  LayerIndex  LCD layer index
  * @retval BSP status
  */
int32_t BSP_LCD_SetActiveLayer(uint32_t LayerIndex)
{
    int32_t ret = BSP_ERROR_NONE;

    if (LayerIndex >= MAX_LAYER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        glcd_ctx.ActiveLayer = LayerIndex;
    }

    return ret;
}

/**
  * @brief  Gets the LCD Active LCD Pixel Format.
  * @param  Instance    LCD Instance
  * @param  PixelFormat Active LCD Pixel Format
  * @retval BSP status
  */
int32_t BSP_LCD_GetPixelFormat(uint32_t *PixelFormat)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Only RGB565 format is supported */
    *PixelFormat = glcd_ctx.PixelFormat;

    return ret;
}

/**
  * @brief  Gets the LCD X size.
  * @param  XSize     LCD width
  * @retval BSP status
  */
int32_t BSP_LCD_GetXSize(uint32_t *XSize)
{
    int32_t ret = BSP_ERROR_NONE;

    if(Lcd_Drv->GetXSize != NULL)
    {
        *XSize = glcd_ctx.XSize;
    }

    return ret;
}

/**
  * @brief  Gets the LCD Y size.
  * @param  YSize     LCD Height
  * @retval BSP status
  */
int32_t BSP_LCD_GetYSize(uint32_t *YSize)
{
    int32_t ret = BSP_ERROR_NONE;

    if(Lcd_Drv->GetYSize != NULL)
    {
        *YSize = glcd_ctx.YSize;
    }

    return ret;
}


/**
  * @brief  Switch On the display.
  * @param  Instance    LCD Instance
  * @retval BSP status
  */
int32_t BSP_LCD_DisplayOn(void)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Lcd_Drv->DisplayOn != NULL)
    {
        if (Lcd_Drv->DisplayOn(Lcd_CompObj) != BSP_ERROR_NONE)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_NONE;
        }
    }

    return ret;
}

/**
  * @brief  Switch Off the display.
  * @param  Instance    LCD Instance
  * @retval BSP status
  */
int32_t BSP_LCD_DisplayOff(void)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Lcd_Drv->DisplayOff != NULL)
    {
        if (Lcd_Drv->DisplayOff(Lcd_CompObj) != BSP_ERROR_NONE)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_NONE;
        }
    }

    return ret;
}

/**
  * @brief  Set the brightness value
  * @param  Instance    LCD Instance
  * @param  Brightness [00: Min (black), 100 Max]
  * @retval BSP status
  */
int32_t BSP_LCD_SetBrightness(uint32_t Brightness)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Lcd_Drv->DisplayOff != NULL)
    {
        if (Lcd_Drv->SetBrightness(Lcd_CompObj, Brightness) != BSP_ERROR_NONE)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_NONE;
        }
    }

    return ret;
}

/**
  * @brief  Set the brightness value
  * @param  Instance    LCD Instance
  * @param  Brightness [00: Min (black), 100 Max]
  * @retval BSP status
  */
int32_t BSP_LCD_GetBrightness(uint32_t *Brightness)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Lcd_Drv->GetBrightness != NULL)
    {
        if (Lcd_Drv->GetBrightness(Lcd_CompObj, Brightness) != BSP_ERROR_NONE)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
    }

    return ret;
}


/*********************************************************************************************/
/******                         Lcd Control Functions                                  *******/
/*********************************************************************************************/


/**
  * @brief  Control the LTDC reload
  * @param  Instance    LCD Instance
  * @param  ReloadType can be one of the following values
  *         - BSP_LCD_RELOAD_NONE
  *         - BSP_LCD_RELOAD_IMMEDIATE
  *         - BSP_LCD_RELOAD_VERTICAL_BLANKING
  * @retval BSP status
  */
int32_t BSP_LCD_Relaod(uint32_t ReloadType)
{
    int32_t ret = BSP_ERROR_NONE;

    if (ReloadType == BSP_LCD_RELOAD_NONE)
    {
        glcd_ctx.ReloadEnable = 0U;
    }
    else if (HAL_LTDC_Reload(&hlcd_ltdc, ReloadType) != HAL_OK)
    {
        ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
        glcd_ctx.ReloadEnable = 1U;
    }

    return ret;
}

/**
  * @brief  Sets an LCD Layer visible
  * @param  Instance    LCD Instance
  * @param  LayerIndex  Visible Layer
  * @param  State  New state of the specified layer
  *          This parameter can be one of the following values:
  *            @arg  ENABLE
  *            @arg  DISABLE
  * @retval BSP status
  */
int32_t BSP_LCD_SetLayerVisible(uint32_t LayerIndex, FunctionalState State)
{
    int32_t ret = BSP_ERROR_NONE;

    if (State == ENABLE)
    {
        __HAL_LTDC_LAYER_ENABLE(&hlcd_ltdc, LayerIndex);
    }
    else
    {
        __HAL_LTDC_LAYER_DISABLE(&hlcd_ltdc, LayerIndex);
    }

    if (glcd_ctx.ReloadEnable == 1U)
    {
        __HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hlcd_ltdc);
    }

    return ret;
}

/**
  * @brief  Configures the transparency.
  * @param  Instance      LCD Instance
  * @param  LayerIndex    Layer foreground or background.
  * @param  Transparency  Transparency
  *           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF
  * @retval BSP status
  */
int32_t BSP_LCD_SetTransparency(uint32_t LayerIndex, uint8_t Transparency)
{
    int32_t ret = BSP_ERROR_NONE;

    if (glcd_ctx.ReloadEnable == 1U)
    {
        (void)HAL_LTDC_SetAlpha(&hlcd_ltdc, Transparency, LayerIndex);
    }
    else
    {
        (void)HAL_LTDC_SetAlpha_NoReload(&hlcd_ltdc, Transparency, LayerIndex);
    }

    return ret;
}

/**
  * @brief  Sets an LCD layer frame buffer address.
  * @param  Instance    LCD Instance
  * @param  LayerIndex  Layer foreground or background
  * @param  Address     New LCD frame buffer value
  * @retval BSP status
  */
int32_t BSP_LCD_SetLayerAddress(uint32_t LayerIndex, uint32_t Address)
{
    int32_t ret = BSP_ERROR_NONE;

    if (glcd_ctx.ReloadEnable == 1U)
    {
        (void)HAL_LTDC_SetAddress(&hlcd_ltdc, Address, LayerIndex);
    }
    else
    {
        (void)HAL_LTDC_SetAddress_NoReload(&hlcd_ltdc, Address, LayerIndex);
    }

    return ret;
}

/**
  * @brief  Sets display window.
  * @param  Instance    LCD Instance
  * @param  LayerIndex  Layer index
  * @param  Xpos   LCD X position
  * @param  Ypos   LCD Y position
  * @param  Width  LCD window width
  * @param  Height LCD window height
  * @retval BSP status
  */
int32_t BSP_LCD_SetLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
    int32_t ret = BSP_ERROR_NONE;

    if (glcd_ctx.ReloadEnable == 1U)
    {
        /* Reconfigure the layer size  and position */
        (void)HAL_LTDC_SetWindowSize(&hlcd_ltdc, Width, Height, LayerIndex);
        (void)HAL_LTDC_SetWindowPosition(&hlcd_ltdc, Xpos, Ypos, LayerIndex);
    }
    else
    {
        /* Reconfigure the layer size and position */
        (void)HAL_LTDC_SetWindowSize_NoReload(&hlcd_ltdc, Width, Height, LayerIndex);
        (void)HAL_LTDC_SetWindowPosition_NoReload(&hlcd_ltdc, Xpos, Ypos, LayerIndex);
    }

    glcd_ctx.XSize = Width;
    glcd_ctx.YSize = Height;

    return ret;
}

/**
  * @brief  Configures and sets the color keying.
  * @param  Instance    LCD Instance
  * @param  LayerIndex  Layer foreground or background
  * @param  Color       Color reference
  * @retval BSP status
  */
int32_t BSP_LCD_SetColorKeying(uint32_t LayerIndex, uint32_t Color)
{
    int32_t ret = BSP_ERROR_NONE;

    if (glcd_ctx.ReloadEnable == 1U)
    {
        /* Configure and Enable the color Keying for LCD Layer */
        (void)HAL_LTDC_ConfigColorKeying(&hlcd_ltdc, Color, LayerIndex);
        (void)HAL_LTDC_EnableColorKeying(&hlcd_ltdc, LayerIndex);
    }
    else
    {
        /* Configure and Enable the color Keying for LCD Layer */
        (void)HAL_LTDC_ConfigColorKeying_NoReload(&hlcd_ltdc, Color, LayerIndex);
        (void)HAL_LTDC_EnableColorKeying_NoReload(&hlcd_ltdc, LayerIndex);
    }

    return ret;
}

/**
  * @brief  Disables the color keying.
  * @param  Instance    LCD Instance
  * @param  LayerIndex Layer foreground or background
  * @retval BSP status
  */
int32_t BSP_LCD_ResetColorKeying(uint32_t LayerIndex)
{
    int32_t ret = BSP_ERROR_NONE;

    if (glcd_ctx.ReloadEnable == 1U)
    {
        /* Disable the color Keying for LCD Layer */
        (void)HAL_LTDC_DisableColorKeying(&hlcd_ltdc, LayerIndex);
    }
    else
    {
        /* Disable the color Keying for LCD Layer */
        (void)HAL_LTDC_DisableColorKeying_NoReload(&hlcd_ltdc, LayerIndex);
    }

    return ret;
}

/*********************************************************************************************/
/******                         Draws Functions                                        *******/
/*********************************************************************************************/
/**
 * @brief  Draws a pixel on LCD.
 * @param  Xpos X position
 * @param  Ypos Y position
 * @param  Color Pixel color
 * @retval BSP status
 */
int32_t BSP_LCD_WritePixel(uint32_t Xpos, uint32_t Ypos, uint32_t Color)
{
    uint32_t lcdFrameOrigin = 0;

    // get current frame buffer start address
    lcdFrameOrigin = hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress;

    if ((hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888)
        || (hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB888))
    {
        if (glcd_ctx.Orientation == LCD_ORIENTATION_LANDSCAPE)
        {
            /* Write data value to SDRAM memory */
            *(__IO uint32_t *)(lcdFrameOrigin + (4U * (Ypos * glcd_ctx.XSize + Xpos))) = Color;
        }
        else
        {
            /* Write data value to SDRAM memory */
            *(__IO uint32_t *)(lcdFrameOrigin + (4U * ((glcd_ctx.YSize - Xpos - 1) * glcd_ctx.XSize + Ypos))) = Color;
        }
    }
    else
    {
        if (glcd_ctx.Orientation == LCD_ORIENTATION_LANDSCAPE)
        {
            /* Write data value to SDRAM memory */
            *(__IO uint16_t *)(lcdFrameOrigin + (2U * (Ypos * glcd_ctx.XSize + Xpos))) = Color;
        }
        else
        {
            /* Write data value to SDRAM memory */
            *(__IO uint16_t *)(lcdFrameOrigin + (2U * ((glcd_ctx.YSize - Xpos - 1) * glcd_ctx.XSize + Ypos))) = Color;
        }
    }

    return BSP_ERROR_NONE;
}

/**
 * @brief  Reads an LCD pixel.
 * @param  Xpos X position
 * @param  Ypos Y position
 * @param  Color RGB pixel color
 * @retval BSP status
 */
int32_t BSP_LCD_ReadPixel(uint32_t Xpos, uint32_t Ypos, uint32_t *Color)
{
    uint32_t lcdFrameOrigin = 0;

    // get current frame buffer start address
    lcdFrameOrigin = hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress;

    if (hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888)
    {
        if (glcd_ctx.Orientation == LCD_ORIENTATION_LANDSCAPE)
        {
            /* Read data value from SDRAM memory */
            *Color = *(__IO uint32_t *)(lcdFrameOrigin + (4U * (Ypos * glcd_ctx.XSize + Xpos)));
        }
        else
        {
            /* Read data value from SDRAM memory */
            *Color = *(__IO uint32_t *)(lcdFrameOrigin + (4U * ((glcd_ctx.YSize - Xpos - 1) * glcd_ctx.XSize + Ypos)));
        }
    }
    else /* if((hlcd_ltdc.LayerCfg[layer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) */
    {
        if (glcd_ctx.Orientation == LCD_ORIENTATION_LANDSCAPE)
        {
            /* Read data value from SDRAM memory */
            *Color = *(__IO uint16_t *)(lcdFrameOrigin + (2U * (Ypos * glcd_ctx.XSize + Xpos)));
        }
        else
        {
            /* Read data value from SDRAM memory */
            *Color = *(__IO uint16_t *)(lcdFrameOrigin + (2U * ((glcd_ctx.YSize - Xpos - 1) * glcd_ctx.XSize + Xpos)));
        }
    }

    return BSP_ERROR_NONE;
}


/**
 * @brief  Fills a buffer.
 * @param  Instance LCD Instance
 * @param  pDst Pointer to destination buffer
 * @param  xSize Buffer width
 * @param  ySize Buffer height
 * @param  OffLine Offset
 * @param  Color Color index
 */
static void LL_DMA2D_FillBuffer(uint32_t *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t Color)
{
    uint32_t output_color_mode, input_color = Color;

    switch (glcd_ctx.PixelFormat)
    {
    case LCD_PIXEL_FORMAT_RGB565:
        output_color_mode = DMA2D_OUTPUT_RGB565; /* RGB565 */
        input_color |= CONVERTARGB88882RGB565(Color);
        break;
    case LCD_PIXEL_FORMAT_RGB888:
    default:
        output_color_mode = DMA2D_OUTPUT_ARGB8888; /* ARGB8888 */
        break;
    }

    /* Register to memory mode with ARGB8888 as color Mode */
    hlcd_dma2d.Init.Mode = DMA2D_R2M;
    hlcd_dma2d.Init.ColorMode = output_color_mode;
    hlcd_dma2d.Init.OutputOffset = OffLine;

    hlcd_dma2d.Instance = DMA2D;
#if 0
    /* DMA2D Initialization */
    if (HAL_DMA2D_Init(&hlcd_dma2d) == HAL_OK)
    {
        if (HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) == HAL_OK)
        {
            if (HAL_DMA2D_Start(&hlcd_dma2d, input_color, (uint32_t)pDst, xSize, ySize) == HAL_OK)
            {
                /* Polling For DMA transfer */
                (void)HAL_DMA2D_PollForTransfer(&hlcd_dma2d, 25);
            }
        }
    }
#endif    
    DMA2D->CR = hlcd_dma2d.Init.Mode;
    DMA2D->OPFCCR = hlcd_dma2d.Init.ColorMode;
    DMA2D->OOR = hlcd_dma2d.Init.OutputOffset;
    //DMA2D->OPFCCR |= (hlcd_dma2d.Init.AlphaInverted << DMA2D_OPFCCR_AI_Pos) | (hlcd_dma2d.Init.RedBlueSwap << DMA2D_OPFCCR_RBS_Pos);
    
    DMA2D->NLR = ySize | (xSize << DMA2D_NLR_PL_Pos);
    DMA2D->OMAR = (uint32_t)pDst;
    DMA2D->OCOLR = input_color;

    DMA2D->CR     |= DMA2D_CR_START; 
    while (DMA2D->CR & DMA2D_CR_START) 
    {
    }
}

/**
 * @brief  Converts a line to an RGB pixel format.
 * @param  pSrc Pointer to source buffer
 * @param  pDst Output color
 * @param  xSize Buffer width
 * @param  ColorMode Input color mode
 */
static void LL_DMA2D_ConvertLineToRGB(uint32_t *pSrc, uint32_t *pDst, uint32_t xSize, uint32_t ColorMode)
{
    uint32_t output_color_mode;

    switch (glcd_ctx.PixelFormat)
    {
    case LCD_PIXEL_FORMAT_RGB565:
        output_color_mode = DMA2D_OUTPUT_RGB565; /* RGB565 */
        break;
    case LCD_PIXEL_FORMAT_RGB888:
    default:
        output_color_mode = DMA2D_OUTPUT_ARGB8888; /* ARGB8888 */
        break;
    }

    /* Configure the DMA2D Mode, Color Mode and output offset */
    hlcd_dma2d.Init.Mode = DMA2D_M2M;
    hlcd_dma2d.Init.ColorMode = output_color_mode;
    hlcd_dma2d.Init.OutputOffset = 0;

    /* Foreground Configuration */
    hlcd_dma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hlcd_dma2d.LayerCfg[1].InputAlpha = 0xFF;
    hlcd_dma2d.LayerCfg[1].InputColorMode = ColorMode;
    hlcd_dma2d.LayerCfg[1].InputOffset = 0;

    hlcd_dma2d.Instance = DMA2D;
#if 0
    /* DMA2D Initialization */
    if (HAL_DMA2D_Init(&hlcd_dma2d) == HAL_OK)
    {
        if (HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) == HAL_OK)
        {
            if (HAL_DMA2D_Start(&hlcd_dma2d, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == HAL_OK)
            {
                /* Polling For DMA transfer */
                (void)HAL_DMA2D_PollForTransfer(&hlcd_dma2d, 50);
            }
        }
    }
#else
    DMA2D->CR = hlcd_dma2d.Init.Mode;
    DMA2D->OPFCCR = 0;
    DMA2D->OOR = 0;
    //DMA2D->OPFCCR |= (hlcd_dma2d.Init.AlphaInverted << DMA2D_OPFCCR_AI_Pos) | (hlcd_dma2d.Init.RedBlueSwap << DMA2D_OPFCCR_RBS_Pos);
    
    DMA2D->FGPFCCR = ColorMode /* | (0xFF << DMA2D_BGPFCCR_ALPHA_Pos) | (DMA2D_NO_MODIF_ALPHA << DMA2D_BGPFCCR_AM_Pos) */;
    DMA2D->FGOR = 0;

    DMA2D->NLR = 1 | (xSize << DMA2D_NLR_PL_Pos);
    DMA2D->OMAR = (uint32_t)pDst;
    DMA2D->FGMAR = (uint32_t)pSrc;

    DMA2D->CR     |= DMA2D_CR_START; 
    while (DMA2D->CR & DMA2D_CR_START) 
    {
    }
#endif
}

/**
  * @brief  Copy a buffer.
  * @param  pSrc: Pointer to source buffer
  * @param  pDst: Output color
  * @param  xSize: Buffer width
  * @param  ColorMode: Input color mode   
  * @retval None
  */
void LL_DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
    uint32_t regValue;
    uint32_t destination = (uint32_t)*pDst;
    uint32_t source      = (uint32_t)pSrc;

    uint32_t output_color_mode;

    switch (glcd_ctx.PixelFormat)
    {
    case LCD_PIXEL_FORMAT_RGB565:
        output_color_mode = DMA2D_OUTPUT_RGB565; /* RGB565 */
        break;
    case LCD_PIXEL_FORMAT_RGB888:
    default:
        output_color_mode = DMA2D_OUTPUT_ARGB8888; /* ARGB8888 */
        break;
    }

    /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
    hlcd_dma2d.Init.Mode = DMA2D_M2M;
    hlcd_dma2d.Init.ColorMode = output_color_mode;
    hlcd_dma2d.Init.OutputOffset = glcd_ctx.XSize - xsize;
    hlcd_dma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA; /* No Output Alpha Inversion*/
    hlcd_dma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;      /* No Output Red & Blue swap */

    /*##-2- Foreground Configuration ###########################################*/
    hlcd_dma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hlcd_dma2d.LayerCfg[1].InputAlpha = 0xFF;
    hlcd_dma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
    hlcd_dma2d.LayerCfg[1].InputOffset = 0;
    hlcd_dma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;      /* No ForeGround Red/Blue swap */
    hlcd_dma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

    hlcd_dma2d.Instance = DMA2D;
#if 1
    DMA2D->CR = hlcd_dma2d.Init.Mode;
    DMA2D->OPFCCR = hlcd_dma2d.Init.ColorMode;
    DMA2D->OOR = hlcd_dma2d.Init.OutputOffset;
    DMA2D->OPFCCR |= (hlcd_dma2d.Init.AlphaInverted << DMA2D_OPFCCR_AI_Pos) | (hlcd_dma2d.Init.RedBlueSwap << DMA2D_OPFCCR_RBS_Pos);

    regValue = hlcd_dma2d.LayerCfg[1].InputColorMode | \
              (hlcd_dma2d.LayerCfg[1].AlphaMode << DMA2D_BGPFCCR_AM_Pos) | \
              (hlcd_dma2d.LayerCfg[1].AlphaInverted << DMA2D_BGPFCCR_AI_Pos) | \
              (hlcd_dma2d.LayerCfg[1].RedBlueSwap << DMA2D_BGPFCCR_RBS_Pos) | \
              (hlcd_dma2d.LayerCfg[1].InputAlpha << DMA2D_BGPFCCR_ALPHA_Pos);

    DMA2D->FGPFCCR = hlcd_dma2d.Init.ColorMode /* | (0xFF << DMA2D_BGPFCCR_ALPHA_Pos) | (DMA2D_NO_MODIF_ALPHA << DMA2D_BGPFCCR_AM_Pos) */;
    DMA2D->FGOR = hlcd_dma2d.LayerCfg[1].InputOffset;
    DMA2D->FGPFCCR |= regValue;

    DMA2D->NLR = ysize | (xsize << DMA2D_NLR_PL_Pos);
    DMA2D->OMAR = (uint32_t)destination;
    DMA2D->FGMAR = (uint32_t)source;

    DMA2D->CR     |= DMA2D_CR_START; 
    while (DMA2D->CR & DMA2D_CR_START) 
    {
    }
#else
    /* DMA2D Initialization */
    if (HAL_DMA2D_Init(&hlcd_dma2d) == HAL_OK)
    {
        if (HAL_DMA2D_ConfigLayer(&hlcd_dma2d, 1) == HAL_OK)
        {
            if (HAL_DMA2D_Start(&hlcd_dma2d, source, destination, xsize, ysize) == HAL_OK)
            {
                /* Polling For DMA transfer */
                HAL_DMA2D_PollForTransfer(&hlcd_dma2d, 50);
            }
        }
    }
#endif

}


/**
 * @brief  Draw a horizontal line on LCD with multibuffer color.
 * @param  Xpos X position.
 * @param  Ypos Y position.
 * @param  pData Pointer to RGB line data
 * @param  Width Rectangle width.
 * @param  Height Rectangle Height.
 * @retval BSP status.
 */
int32_t BSP_LCD_FillRGBRect(uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height)
{
    uint32_t i;
#if 0
    uint32_t  Xaddress;

    for (i = 0; i < Height; i++)
    {
        /* Get the line address */
        Xaddress = hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress + (glcd_ctx.BppFactor * ((glcd_ctx.XSize * (Ypos + i)) + Xpos));

//#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
        SCB_CleanDCache_by_Addr((uint32_t *)pData, glcd_ctx.BppFactor * glcd_ctx.XSize);
//#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */

        /* Write line */
        if (glcd_ctx.PixelFormat == LCD_PIXEL_FORMAT_RGB565)
        {
            LL_DMA2D_ConvertLineToRGB((uint32_t *)pData, (uint32_t *)Xaddress, Width, DMA2D_INPUT_RGB565);
        }
        else
        {
            LL_DMA2D_ConvertLineToRGB((uint32_t *)pData, (uint32_t *)Xaddress, Width, DMA2D_INPUT_ARGB8888);
        }
        pData += glcd_ctx.BppFactor * Width;
    }

#else
    uint32_t color, j;
    for (i = 0; i < Height; i++)
    {
        for (j = 0; j < Width; j++)
        {
            if (glcd_ctx.PixelFormat == LCD_PIXEL_FORMAT_RGB565)
            {
                color = *pData | (*(pData + 1) << 8);
            }
            else
            {
                color = *pData | (*(pData + 1) << 8) | (*(pData + 2) << 16) | (*(pData + 3) << 24);
            }
            
            BSP_LCD_WritePixel(Xpos + j, Ypos + i, color);
            pData += glcd_ctx.BppFactor;
        }
    }
#endif
    return BSP_ERROR_NONE;
}



/**
  * @brief  Draws a full rectangle in currently active layer with single color.
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Width Rectangle width
  * @param  Height Rectangle height
  * @param  Color Pixel color
  * @retval BSP status.
  */
int32_t BSP_LCD_FillRect(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color)
{
    uint32_t  Xaddress;

    /* Get the rectangle start address */
    Xaddress = (hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress) + (glcd_ctx.BppFactor*(glcd_ctx.XSize*Ypos + Xpos));

    /* Fill the rectangle */
    LL_DMA2D_FillBuffer((uint32_t *)Xaddress, Width, Height, (glcd_ctx.XSize - Width), Color);

    return BSP_ERROR_NONE;
}



/**
  * @brief  Draws an horizontal line in currently active layer.
  * @param  Xpos  X position
  * @param  Ypos  Y position
  * @param  Length  Line length
  * @param  Color Pixel color
  * @retval BSP status.
  */
int32_t BSP_LCD_DrawHLine(uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
    uint32_t  Xaddress;

    /* Get the line address */
    Xaddress = hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress + (glcd_ctx.BppFactor*((glcd_ctx.XSize*Ypos) + Xpos));

    /* Write line */
    if((Xpos + Length) > glcd_ctx.XSize)
    {
    Length = glcd_ctx.XSize - Xpos;
    }
    LL_DMA2D_FillBuffer((uint32_t *)Xaddress, Length, 1, 0, Color);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Draws a vertical line in currently active layer.
  * @param  Xpos  X position
  * @param  Ypos  Y position
  * @param  Length  Line length
  * @param  Color Pixel color
  * @retval BSP status.
  */
int32_t BSP_LCD_DrawVLine(uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
    uint32_t  Xaddress;

    /* Get the line address */
    Xaddress = (hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress) + (glcd_ctx.BppFactor*(glcd_ctx.XSize*Ypos + Xpos));

    /* Write line */
    if((Ypos + Length) > glcd_ctx.YSize)
    {
    Length = glcd_ctx.YSize - Ypos;
    }
    LL_DMA2D_FillBuffer((uint32_t *)Xaddress, 1, Length, (glcd_ctx.XSize - 1U), Color);

    return BSP_ERROR_NONE;
}



/**
 * @brief  Draws a bitmap picture loaded in the internal Flash in currently active layer.
 * @param  Xpos Bmp X position in the LCD
 * @param  Ypos Bmp Y position in the LCD
 * @param  pBmp Pointer to Bmp picture address in the internal Flash.
 * @retval BSP status
 */
int32_t BSP_LCD_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp)
{
    int32_t ret = BSP_ERROR_NONE;
    uint32_t index, width, height, bit_pixel;
    uint32_t Address;
    uint32_t input_color_mode;
    uint8_t *pbmp;

    /* Get bitmap data address offset */
    index = (uint32_t)pBmp[10] + ((uint32_t)pBmp[11] << 8) + ((uint32_t)pBmp[12] << 16) + ((uint32_t)pBmp[13] << 24);

    /* Read bitmap width */
    width = (uint32_t)pBmp[18] + ((uint32_t)pBmp[19] << 8) + ((uint32_t)pBmp[20] << 16) + ((uint32_t)pBmp[21] << 24);

    /* Read bitmap height */
    height = (uint32_t)pBmp[22] + ((uint32_t)pBmp[23] << 8) + ((uint32_t)pBmp[24] << 16) + ((uint32_t)pBmp[25] << 24);

    /* Read bit/pixel */
    bit_pixel = (uint32_t)pBmp[28] + ((uint32_t)pBmp[29] << 8);

    /* Set the address */
    Address = hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress + (((glcd_ctx.XSize * Ypos) + Xpos) * glcd_ctx.BppFactor);

    /* Get the layer pixel format */
    if ((bit_pixel / 8U) == 4U)
    {
        input_color_mode = DMA2D_INPUT_ARGB8888;
    }
    else if ((bit_pixel / 8U) == 2U)
    {
        input_color_mode = DMA2D_INPUT_RGB565;
    }
    else
    {
        input_color_mode = DMA2D_INPUT_RGB888;
    }

    /* Bypass the bitmap header */
    pbmp = pBmp + (index + (width * (height - 1U) * (bit_pixel / 8U)));

    /* Convert picture to ARGB8888 pixel format */
    for (index = 0; index < height; index++)
    {
        /* Pixel format conversion */
        LL_DMA2D_ConvertLineToRGB((uint32_t *)pbmp, (uint32_t *)Address, width, input_color_mode);

        /* Increment the source and destination buffers */
        Address += (glcd_ctx.XSize * glcd_ctx.BppFactor);
        pbmp -= width * (bit_pixel / 8U);
    }

    return ret;
}


void BSP_LCD_InitContext(void)
{
    /* Set the LCD Context default value*/
    glcd_ctx.ActiveLayer = 0;
    glcd_ctx.PixelFormat = LCD_PIXEL_FORMAT_RGB565;
    glcd_ctx.BppFactor = 2; /* 4 Bytes Per Pixel for ARGB8888 */  
    glcd_ctx.XSize = LCD_DEFAULT_WIDTH;  
    glcd_ctx.YSize = LCD_DEFAULT_HEIGHT;
    glcd_ctx.Orientation = LCD_ORIENTATION_LANDSCAPE;

    LCD_HwInit(LCD_ORIENTATION_LANDSCAPE);

    Lcd_Drv = (LCD_Drv_t *)(void *) &OTM8009A_LCD_Driver;
}

void BSP_LCD_ColorFill(uint16_t Xpos, uint16_t Ypos, uint16_t xSize, uint16_t ySize, uint32_t *color)
{
    uint32_t addr;

    /* Get the rectangle start address */
    addr = (hlcd_ltdc.LayerCfg[glcd_ctx.ActiveLayer].FBStartAdress) + (glcd_ctx.BppFactor*(glcd_ctx.XSize*Ypos + Xpos));

    LL_DMA2D_CopyBuffer((uint32_t*)color, &addr, Xpos, Ypos, xSize, ySize);
}
