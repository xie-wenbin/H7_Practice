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
#include "bsp_lcd_dev.h"

BSP_LCD_Ctx_t glcd_ctx;

void BSP_LCD_InitContext(void)
{
    /* Set the LCD Context default value*/
    glcd_ctx.ActiveLayer = 0;
    glcd_ctx.PixelFormat = LCD_PIXEL_FORMAT_ARGB8888;
    glcd_ctx.BppFactor = 4; /* 4 Bytes Per Pixel for ARGB8888 */  
    glcd_ctx.XSize = LCD_DEFAULT_WIDTH;  
    glcd_ctx.YSize = LCD_DEFAULT_HEIGHT;
    glcd_ctx.Orientation = LCD_ORIENTATION_LANDSCAPE;
}


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
static void LL_DMA2D_FillBuffer(uint32_t LayerIndex, uint32_t *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t Color)
{
    uint32_t output_color_mode, input_color = Color;

    switch (glcd_ctx.PixelFormat)
    {
    case LCD_PIXEL_FORMAT_RGB565:
        output_color_mode = DMA2D_OUTPUT_RGB565; /* RGB565 */
        input_color = CONVERTRGB5652ARGB8888(Color);
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
}


/**
 * @brief  Draw a horizontal line on LCD.
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

    uint32_t color, j;
    for (i = 0; i < Height; i++)
    {
        for (j = 0; j < Width; j++)
        {
            color = *pData | (*(pData + 1) << 8) | (*(pData + 2) << 16) | (*(pData + 3) << 24);
            BSP_LCD_WritePixel(Xpos + j, Ypos + i, color);
            pData += glcd_ctx.BppFactor;
        }
    }

    return BSP_ERROR_NONE;
}

#define ABS(X)                 ((X) > 0 ? (X) : -(X))

void BSP_LCD_DrawLine(uint32_t Xpos1, uint32_t Ypos1, uint32_t Xpos2, uint32_t Ypos2, uint32_t Color)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
  curpixel = 0;
  int32_t x_diff, y_diff;

  x_diff = Xpos2 - Xpos1;
  y_diff = Ypos2 - Ypos1;

  deltax = ABS(x_diff);         /* The absolute difference between the x's */
  deltay = ABS(y_diff);         /* The absolute difference between the y's */
  x = Xpos1;                       /* Start x off at the first pixel */
  y = Ypos1;                       /* Start y off at the first pixel */

  if (Xpos2 >= Xpos1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }

  if (Ypos2 >= Ypos1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }

  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }

  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    BSP_LCD_WritePixel(x, y, Color);   /* Draw the current pixel */
    num += numadd;                            /* Increase the numerator by the top of the fraction */
    if (num >= den)                           /* Check if numerator >= denominator */
    {
      num -= den;                             /* Calculate the new numerator value */
      x += xinc1;                             /* Change the x as appropriate */
      y += yinc1;                             /* Change the y as appropriate */
    }
    x += xinc2;                               /* Change the x as appropriate */
    y += yinc2;                               /* Change the y as appropriate */
  }
}

void BSP_LCD_DrawExampleTest(void)
{
    uint32_t iLoop, j;

    for (iLoop = 0; iLoop < 120; iLoop++)
    {
        for (j = 0; j < 120; j++)
        {
            BSP_LCD_WritePixel(240+j, 240+iLoop, 0xFF007FFFUL);
        }
    }

    BSP_LCD_DrawLine(120, 120, 240, 240, 0xFF7F7F4FUL);
}
