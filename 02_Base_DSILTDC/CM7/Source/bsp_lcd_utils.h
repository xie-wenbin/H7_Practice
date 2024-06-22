/*
*********************************************************************************************************
*
*   [module   ] : LCD (For STM32H7)
*   [file name] : bsp_lcd_utils.h
*   [version  ] : V1.0
*   [brief    ] : lcd draw tool utils interface module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_LCD_UTILS_H_
#define BSP_LCD_UTILS_H_

#include "fonts.h"
#include <stddef.h>

/**
  * @brief  LCD Utility color definitions values
  */
#define LCD_UTIL_COLOR_BLUE          0xFF0000FFUL
#define LCD_UTIL_COLOR_GREEN         0xFF00FF00UL
#define LCD_UTIL_COLOR_RED           0xFFFF0000UL
#define LCD_UTIL_COLOR_CYAN          0xFF00FFFFUL
#define LCD_UTIL_COLOR_MAGENTA       0xFFFF00FFUL
#define LCD_UTIL_COLOR_YELLOW        0xFFFFFF00UL
#define LCD_UTIL_COLOR_LIGHTBLUE     0xFF8080FFUL
#define LCD_UTIL_COLOR_LIGHTGREEN    0xFF80FF80UL
#define LCD_UTIL_COLOR_LIGHTRED      0xFFFF8080UL
#define LCD_UTIL_COLOR_LIGHTCYAN     0xFF80FFFFUL
#define LCD_UTIL_COLOR_LIGHTMAGENTA  0xFFFF80FFUL
#define LCD_UTIL_COLOR_LIGHTYELLOW   0xFFFFFF80UL
#define LCD_UTIL_COLOR_DARKBLUE      0xFF000080UL
#define LCD_UTIL_COLOR_DARKGREEN     0xFF008000UL
#define LCD_UTIL_COLOR_DARKRED       0xFF800000UL
#define LCD_UTIL_COLOR_DARKCYAN      0xFF008080UL
#define LCD_UTIL_COLOR_DARKMAGENTA   0xFF800080UL
#define LCD_UTIL_COLOR_DARKYELLOW    0xFF808000UL
#define LCD_UTIL_COLOR_WHITE         0xFFFFFFFFUL
#define LCD_UTIL_COLOR_LIGHTGRAY     0xFFD3D3D3UL
#define LCD_UTIL_COLOR_GRAY          0xFF808080UL
#define LCD_UTIL_COLOR_DARKGRAY      0xFF404040UL
#define LCD_UTIL_COLOR_BLACK         0xFF000000UL
#define LCD_UTIL_COLOR_BROWN         0xFFA52A2AUL
#define LCD_UTIL_COLOR_ORANGE        0xFFFFA500UL

/* Definition of Official ST Colors */
#define LCD_UTIL_COLOR_ST_BLUE_DARK   0xFF002052UL
#define LCD_UTIL_COLOR_ST_BLUE        0xFF39A9DCUL
#define LCD_UTIL_COLOR_ST_BLUE_LIGHT  0xFFD1E4F3UL
#define LCD_UTIL_COLOR_ST_GREEN_LIGHT 0xFFBBCC01UL
#define LCD_UTIL_COLOR_ST_GREEN_DARK  0xFF003D14UL
#define LCD_UTIL_COLOR_ST_YELLOW      0xFFFFD300UL
#define LCD_UTIL_COLOR_ST_BROWN       0xFF5C0915UL
#define LCD_UTIL_COLOR_ST_PINK        0xFFD4007AUL
#define LCD_UTIL_COLOR_ST_PURPLE      0xFF590D58UL
#define LCD_UTIL_COLOR_ST_GRAY_DARK   0xFF4F5251UL
#define LCD_UTIL_COLOR_ST_GRAY        0xFF90989EUL
#define LCD_UTIL_COLOR_ST_GRAY_LIGHT  0xFFB9C4CAUL


typedef enum
{
  CENTER_MODE             = 0x01,    /*!< Center mode */
  RIGHT_MODE              = 0x02,    /*!< Right mode  */
  LEFT_MODE               = 0x03     /*!< Left mode   */
} Text_AlignModeTypdef;

typedef enum
{
    FONTSIZE_8  = 0,
    FONTSIZE_12,
    FONTSIZE_16,
    FONTSIZE_20,
    FONTSIZE_24,

    FONTSIZE_MAXNUM
} Text_FontSize;

typedef struct
{
  uint32_t  TextColor; /*!< Specifies the color of text */
  uint32_t  BackColor; /*!< Specifies the background color below the text */
  sFONT    *pFont;     /*!< Specifies the font used for the text */
  uint32_t  LcdLayer;
  uint32_t  LcdDevice;
  uint32_t  LcdXsize;
  uint32_t  LcdYsize;
  uint32_t  LcdPixelFormat;
} UTIL_LCD_Ctx_t;

/**
  * @brief  LCD Utility Drawing point (pixel) geometric definition
  */
typedef struct
{
  int16_t X; /*!< geometric X position of drawing */
  int16_t Y; /*!< geometric Y position of drawing */
} LcdDrawPoint;

/**
  * @brief  Pointer on LCD Utility Drawing point (pixel) geometric definition
  */
typedef LcdDrawPoint * pLcdDrawPoint;

typedef struct
{
  int32_t ( *DrawBitmap      ) (uint32_t, uint32_t, uint8_t *);
  int32_t ( *FillRGBRect     ) (uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t);
  int32_t ( *DrawHLine       ) (uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *DrawVLine       ) (uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *FillRect        ) (uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *GetPixel        ) (uint32_t, uint32_t, uint32_t*);
  int32_t ( *SetPixel        ) (uint32_t, uint32_t, uint32_t);
  int32_t ( *GetXSize        ) (uint32_t *);
  int32_t ( *GetYSize        ) (uint32_t *);
  int32_t ( *SetLayer        ) (uint32_t);
  int32_t ( *GetFormat       ) (uint32_t *);
} LCD_UTILS_Drv_t;



void LCD_UTIL_SetFuncDriver(const LCD_UTILS_Drv_t *pDrv);

void     LCD_UTIL_SetBackColor(uint32_t Color);
uint32_t LCD_UTIL_GetBackColor(void);
sFONT* LCD_UTIL_GetFont(void);

void LCD_UTIL_Clear(uint32_t Color);
void LCD_UTIL_GetPixel(uint16_t Xpos, uint16_t Ypos, uint32_t *Color);
void LCD_UTIL_SetPixel(uint16_t Xpos, uint16_t Ypos, uint32_t Color);
void LCD_UTIL_DrawHLine(uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
void LCD_UTIL_DrawVLine(uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
void LCD_UTIL_DisplayStringAtLine(uint32_t Line, Text_FontSize fontSize, uint32_t fontColor, uint8_t *ptr);
void LCD_UTIL_DisplayString(uint32_t Xpos, uint32_t Ypos, Text_FontSize fontSize, uint32_t fontColor, uint8_t *Text, Text_AlignModeTypdef Mode);
void LCD_UTIL_ClearStringLine(uint32_t Line);
void LCD_UTIL_DrawLine(uint32_t Xpos1, uint32_t Ypos1, uint32_t Xpos2, uint32_t Ypos2, uint32_t Color);
void LCD_UTIL_DrawRect(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
void LCD_UTIL_DrawCircle(uint32_t Xpos, uint32_t Ypos, uint32_t Radius, uint32_t Color);
void LCD_UTIL_DrawPolygon(pLcdDrawPoint Points, uint32_t PointCount, uint32_t Color);
void LCD_UTIL_DrawEllipse(int Xpos, int Ypos, int XRadius, int YRadius, uint32_t Color);
void LCD_UTIL_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pData);

void LCD_UTIL_FillRect(uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
void LCD_UTIL_FillCircle(uint32_t Xpos, uint32_t Ypos, uint32_t Radius, uint32_t Color);
void LCD_UTIL_FillPolygon(pLcdDrawPoint Points, uint32_t PointCount, uint32_t Color);
void LCD_UTIL_FillEllipse(int Xpos, int Ypos, int XRadius, int YRadius, uint32_t Color);

#endif /*BSP_LCD_UTILS_H_*/
