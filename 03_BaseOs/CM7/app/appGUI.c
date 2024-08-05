#include "appGUI.h"
#include "appPicBinary.h"
#include "bsp_lcd_hw.h"
#include "bsp_lcd_svc.h"
#include "stdio.h"
#include "stlogo.h"

extern const unsigned char stlogo[9174];

/* Private define ------------------------------------------------------------*/
#define  CIRCLE_RADIUS        40
#define  LINE_LENGHT          30
/* Private macro -------------------------------------------------------------*/
#define  CIRCLE_XPOS(i)       ((i * 800) / 5)
#define  CIRCLE_YPOS(i)       (480 - CIRCLE_RADIUS - 60)
#if (USE_TS_MULTI_TOUCH == 1)
static uint32_t touchscreen_color_idx = 0;
#endif /* USE_TS_MULTI_TOUCH */

/* Possible allowed indexes of touchscreen demo */
typedef enum
{
    TOUCHSCREEN_DEMO_1 = 1,
    TOUCHSCREEN_DEMO_2 = 2,
    TOUCHSCREEN_DEMO_3 = 3,
    TOUCHSCREEN_DEMO_MAX = TOUCHSCREEN_DEMO_3,

} TouchScreenDemoTypeDef;

/* Table for touchscreen event information display on LCD : table indexed on enum @ref TS_TouchEventTypeDef information */
char * ts_event_string_tab[4] = { "None",
"Press down",
"Lift up",
"Contact"};

/* Table for touchscreen gesture Id information display on LCD : table indexed on enum @ref TS_GestureIdTypeDef information */
char * ts_gesture_id_string_tab[7] = { "No Gesture",
"Move Up",
"Move Right",
"Move Down",
"Move Left",
"Zoom In",
"Zoom Out"};

/* Global variables ---------------------------------------------------------*/
uint32_t TouchScreenDemo = 0;
uint16_t x_new_pos = 0, x_previous_pos = 0;
uint16_t y_new_pos = 0, y_previous_pos = 0;
TS_State_t  TS_State = {0};
TS_MultiTouch_State_t  TS_MTState = {0};
uint32_t colors[24] = {LCD_UTIL_COLOR_BLUE, LCD_UTIL_COLOR_GREEN, LCD_UTIL_COLOR_RED, LCD_UTIL_COLOR_CYAN, LCD_UTIL_COLOR_MAGENTA, LCD_UTIL_COLOR_YELLOW,
LCD_UTIL_COLOR_LIGHTBLUE, LCD_UTIL_COLOR_LIGHTGREEN, LCD_UTIL_COLOR_LIGHTRED, LCD_UTIL_COLOR_WHITE, LCD_UTIL_COLOR_LIGHTMAGENTA,
LCD_UTIL_COLOR_LIGHTYELLOW, LCD_UTIL_COLOR_DARKBLUE, LCD_UTIL_COLOR_DARKGREEN, LCD_UTIL_COLOR_DARKRED, LCD_UTIL_COLOR_DARKCYAN,
LCD_UTIL_COLOR_DARKMAGENTA, LCD_UTIL_COLOR_DARKYELLOW, LCD_UTIL_COLOR_LIGHTGRAY, LCD_UTIL_COLOR_GRAY, LCD_UTIL_COLOR_DARKGRAY,
LCD_UTIL_COLOR_BLACK, LCD_UTIL_COLOR_BROWN, LCD_UTIL_COLOR_ORANGE };

static TS_Init_t *hTS;
static TS_Gesture_Config_t GestureConf;

/* Private function prototypes -----------------------------------------------*/
static void     Touchscreen_SetHint_Demo(TouchScreenDemoTypeDef demoIndex);
void Touchscreen_DrawBackground_Circles(uint8_t state);
#if (USE_TS_MULTI_TOUCH == 1)
static uint32_t Touchscreen_Handle_NewTouch(void);
static void TS_Update(void);
#endif /* USE_TS_MULTI_TOUCH */


void appGUI_InitializeDisplay(void)
{
    char desc[64];
    uint32_t x_size = 800;
    uint32_t y_size = 480;

    /* Clear the LCD */
    LCD_UTIL_SetBackColor(0xFFFFFFFFUL);
    LCD_UTIL_Clear(0xFFFFFFFFUL);

    /* Display LCD messages */
    LCD_UTIL_DisplayString(0, 10, FONTSIZE_24, 0xFF000080UL, (uint8_t *)"STM32H747I BSP", CENTER_MODE);
    LCD_UTIL_DisplayString(0, 35, FONTSIZE_24, 0xFF000080UL, (uint8_t *)"Drivers examples", CENTER_MODE);

    /* Draw Bitmap */
    LCD_UTIL_DrawBitmap((x_size - 80)/2, 65, (uint8_t *)stlogo);

    LCD_UTIL_DisplayString(0, y_size - 20, FONTSIZE_12, 0xFF000080UL, (uint8_t *)"Copyright (c) STMicroelectronics 2018", CENTER_MODE);

    LCD_UTIL_FillRect(0, y_size/2 + 15, x_size, 60, 0xFF0000FFUL);
    LCD_UTIL_SetBackColor(0xFF0000FFUL);
    LCD_UTIL_DisplayString(0, y_size / 2 + 30, FONTSIZE_16, 0xFFFFFFFFUL, (uint8_t *)"Press Wakeup button to start :", CENTER_MODE);
    sprintf(desc,"TOUCHSCREEN DEMO1 example");
    LCD_UTIL_DisplayString(0, y_size/2 + 45,FONTSIZE_16, 0xFFFFFFFFUL, (uint8_t *)desc, CENTER_MODE);

}

void TouchScreenDemo1_Init(void)
{
    /* Display touch screen demo description */
    Touchscreen_SetHint_Demo(TOUCHSCREEN_DEMO_1);
    Touchscreen_DrawBackground_Circles(0);
}

void TouchScreenDemo1(void)
{
    uint16_t x1, y1;
    uint8_t state = 0;
    // uint32_t ts_status = BSP_ERROR_NONE;

    /* Display touch screen demo description */
    // Touchscreen_SetHint_Demo(TOUCHSCREEN_DEMO_1);
    // Touchscreen_DrawBackground_Circles(state);

    // while (1)
    {
      /* Check in polling mode in touch screen the touch status and coordinates */
      /* of touches if touch occurred                                           */
      /* ts_status = */BSP_TS_GetState(&TS_State);
      if(TS_State.TouchDetected)
      {
        /* One or dual touch have been detected          */
        /* Only take into account the first touch so far */

        /* Get X and Y position of the first touch post calibrated */
        x1 = TS_State.TouchX;
        y1 = TS_State.TouchY;
        printf("touchscreen x: %d, y:%d\r\n", x1, y1);

        if ((y1 > (CIRCLE_YPOS(1) - CIRCLE_RADIUS)) &&
            (y1 < (CIRCLE_YPOS(1) + CIRCLE_RADIUS)))
        {
          if ((x1 > (CIRCLE_XPOS(1) - CIRCLE_RADIUS)) &&
              (x1 < (CIRCLE_XPOS(1) + CIRCLE_RADIUS)))
          {
            if ((state & 1) == 0)
            {
              Touchscreen_DrawBackground_Circles(state);
              LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);
              state = 1;
            }
          }
          if ((x1 > (CIRCLE_XPOS(2) - CIRCLE_RADIUS)) &&
              (x1 < (CIRCLE_XPOS(2) + CIRCLE_RADIUS)))
          {
            if ((state & 2) == 0)
            {
              Touchscreen_DrawBackground_Circles(state);
              LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, LCD_UTIL_COLOR_RED);
              state = 2;
            }
          }

          if ((x1 > (CIRCLE_XPOS(3) - CIRCLE_RADIUS)) &&
              (x1 < (CIRCLE_XPOS(3) + CIRCLE_RADIUS)))
          {
            if ((state & 4) == 0)
            {
              Touchscreen_DrawBackground_Circles(state);
              LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_YELLOW);
              state = 4;
            }
          }

          if ((x1 > (CIRCLE_XPOS(4) - CIRCLE_RADIUS)) &&
              (x1 < (CIRCLE_XPOS(4) + CIRCLE_RADIUS)))
          {
            if ((state & 8) == 0)
            {
              Touchscreen_DrawBackground_Circles(state);
              LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_GREEN);
              state = 8;
            }
          }
        }
      }
    //   if(CheckForUserInput() > 0)
    //   {

    //     BSP_TS_DeInit();

    //     // ButtonState = 0;
    //     return;
    //   }

      HAL_Delay(20);
    }
}

/**
  * @brief  Display TS Demo Hint for all touchscreen demos depending on passed
  *         demoIndex in parameter.
  * @param  demoIndex : parameter of type @ref TouchScreenDemoTypeDef
  * @retval None
  */
static void Touchscreen_SetHint_Demo(TouchScreenDemoTypeDef demoIndex)
{
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(&x_size);
    BSP_LCD_GetYSize(&y_size);

    if(demoIndex <= TOUCHSCREEN_DEMO_MAX)
    {
        /* Clear the LCD */
        LCD_UTIL_Clear(LCD_UTIL_COLOR_WHITE);

        /* Set Touchscreen Demo1 description */
        LCD_UTIL_FillRect(0, 0, x_size, 80, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_SetBackColor(LCD_UTIL_COLOR_BLUE);

        if(demoIndex == TOUCHSCREEN_DEMO_1)
        {
        LCD_UTIL_DisplayString(0, 0,  FONTSIZE_24, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Touchscreen basic polling", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 30, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Please use the Touchscreen to", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 45, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"activate the colored circle", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 60, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"inside the rectangle. Then press User button", CENTER_MODE);
        }
        else if (demoIndex == TOUCHSCREEN_DEMO_2)
        {
        LCD_UTIL_DisplayString(0, 0, FONTSIZE_24, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Touchscreen dual touch polling", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 30, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Please press the Touchscreen to", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 45, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"activate single and", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 60, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"dual touch", CENTER_MODE);

        }
        else /* demoIndex == TOUCHSCREEN_DEMO_3 */
        {
        LCD_UTIL_DisplayString(0, 0, FONTSIZE_24, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Touchscreen dual touch interrupt", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 30, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Please press the Touchscreen to", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 45, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"activate single and", CENTER_MODE);
        LCD_UTIL_DisplayString(0, 60, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"dual touch", CENTER_MODE);
        }

        LCD_UTIL_DrawRect(10, 90, x_size - 20, y_size - 100, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawRect(11, 91, x_size - 22, y_size - 102, LCD_UTIL_COLOR_BLUE);

    } /* of if(demoIndex <= TOUCHSCREEN_DEMO_MAX) */
}

/**
  * @brief  Draw Touchscreen Background
  * @param  state : touch zone state
  * @retval None
  */
void Touchscreen_DrawBackground_Circles(uint8_t state)
{
    uint16_t x, y;
    switch (state)
    {
    case 0:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);

        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, LCD_UTIL_COLOR_RED);

        LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_YELLOW);


        LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_GREEN);

        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        break;

    case 1:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        break;

    case 2:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, LCD_UTIL_COLOR_RED);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        break;

    case 4:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_YELLOW);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        break;

    case 8:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(4), CIRCLE_RADIUS, LCD_UTIL_COLOR_GREEN);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(4), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        break;

    case 16:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLUE);

        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);

        LCD_UTIL_DrawHLine(CIRCLE_XPOS(1)-LINE_LENGHT, CIRCLE_YPOS(1), 2*LINE_LENGHT, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawHLine(CIRCLE_XPOS(2)-LINE_LENGHT, CIRCLE_YPOS(2), 2*LINE_LENGHT, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawVLine(CIRCLE_XPOS(2), CIRCLE_YPOS(2)-LINE_LENGHT, 2*LINE_LENGHT, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawHLine(CIRCLE_XPOS(3)-LINE_LENGHT, CIRCLE_YPOS(3), 2*LINE_LENGHT, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawHLine(CIRCLE_XPOS(4)-LINE_LENGHT, CIRCLE_YPOS(4), 2*LINE_LENGHT, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawVLine(CIRCLE_XPOS(4), CIRCLE_YPOS(4)-LINE_LENGHT, 2*LINE_LENGHT, LCD_UTIL_COLOR_BLUE);

        LCD_UTIL_SetBackColor(LCD_UTIL_COLOR_WHITE);
        x = CIRCLE_XPOS(1);
        y = CIRCLE_YPOS(1) - CIRCLE_RADIUS - LCD_UTIL_GetFont()->Height;
        LCD_UTIL_DisplayString(x, y, FONTSIZE_24, LCD_UTIL_COLOR_BLUE, (uint8_t *)"Volume", CENTER_MODE);
        x = CIRCLE_XPOS(4);
        y = CIRCLE_YPOS(4) - CIRCLE_RADIUS - LCD_UTIL_GetFont()->Height;
        LCD_UTIL_DisplayString(x, y, FONTSIZE_24, LCD_UTIL_COLOR_BLUE, (uint8_t *)"Frequency", CENTER_MODE);

        break;

    case 32:
        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLACK);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS, LCD_UTIL_COLOR_BLACK);

        LCD_UTIL_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2, LCD_UTIL_COLOR_WHITE);


        LCD_UTIL_SetBackColor(LCD_UTIL_COLOR_WHITE);
        x = CIRCLE_XPOS(1) - 10;
        y = CIRCLE_YPOS(1) - (LCD_UTIL_GetFont()->Height)/2;
        LCD_UTIL_DisplayString(x, y, FONTSIZE_20, LCD_UTIL_COLOR_BLACK, (uint8_t *)"Up", LEFT_MODE);
        x = CIRCLE_XPOS(2) - 10;
        y = CIRCLE_YPOS(3)  - (LCD_UTIL_GetFont()->Height)/2;
        LCD_UTIL_DisplayString(x, y, FONTSIZE_20, LCD_UTIL_COLOR_BLACK, (uint8_t *)"Dw", LEFT_MODE);

        break;
    }
}


#if (USE_TS_MULTI_TOUCH == 1)

void TouchScreenDemo2_Init(void)
{
    uint32_t ts_status = BSP_ERROR_NONE;
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(&x_size);
    BSP_LCD_GetYSize(&y_size);

    hTS->Width = x_size;
    hTS->Height = y_size;
    hTS->Orientation = TS_SWAP_XY | TS_SWAP_Y;
    hTS->Accuracy = 5;

    GestureConf.Radian = 0x0A;
    GestureConf.OffsetLeftRight = 0x19;
    GestureConf.OffsetUpDown = 0x19;
    GestureConf.DistanceLeftRight = 0x19;
    GestureConf.DistanceUpDown = 0x19;
    GestureConf.DistanceZoom = 0x32;

    /* Touchscreen initialization */
    ts_status = BSP_TS_Init(hTS);
    ts_status = BSP_TS_EnableIT();
    ts_status = BSP_TS_GestureConfig(&GestureConf);

    if(ts_status == BSP_ERROR_NONE)
    {
        /* Display touch screen demo2 description */
        Touchscreen_SetHint_Demo(TOUCHSCREEN_DEMO_2);
    } /* of if(status == BSP_ERROR_NONE) */
}


/**
  * @brief  Touchscreen Demo2 : test touchscreen single and dual touch in polling mode
  * @param  None
  * @retval None
  */
void Touchscreen_demo2(void)
{
    Touchscreen_Handle_NewTouch();
}


void BSP_TS_Callback(void)
{
    // TS_Update();
}

static void TS_Update(void)
{
    uint16_t i, j;

    BSP_TS_GetState(&TS_State);
    if(TS_State.TouchDetected)
    {
        /* One or dual touch have been detected          */
        /* Only take into account the first touch so far */

        /* Get X and Y position of the first touch post calibrated */
        x_new_pos = TS_State.TouchX;
        y_new_pos = TS_State.TouchY;

        for(i = 0; i < hTS->Width/40; i++)
        {
        for(j = 0; j < hTS->Height/40; j++)
        {
            if(((x_new_pos > 40*i) && (x_new_pos < 40*(i+1))) && ((y_new_pos > 40*j) && (y_new_pos < 40*(j+1))))
            {
            LCD_UTIL_FillRect(x_previous_pos, y_previous_pos,40,40, LCD_UTIL_COLOR_WHITE);
            LCD_UTIL_DrawRect(x_previous_pos, y_previous_pos,40,40, LCD_UTIL_COLOR_BLACK);

            LCD_UTIL_FillRect(40*i, 40*j,40,40, colors[(touchscreen_color_idx++ % 24)]);

            x_previous_pos = 40*i;
            y_previous_pos = 40*j;
            break;
            }
        }
        }
    }
}

/**
  * @brief  Touchscreen_Handle_NewTouch: factorization of touch management
  * @param  None
  * @retval BSP_ERROR_NONE
  */
static uint32_t Touchscreen_Handle_NewTouch(void)
{
    uint32_t GestureId = GESTURE_ID_NO_GESTURE;
    #define TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS 15
    #define TOUCH_INFO_STRING_SIZE                70
    uint16_t x1 = 0;
    uint16_t y1 = 0;
    uint16_t x2 = 0;
    uint16_t y2 = 0;
    uint32_t drawTouch1 = 0; /* activate/deactivate draw of footprint of touch 1 */
    uint32_t drawTouch2 = 0; /* activate/deactivate draw of footprint of touch 2 */
    uint32_t colors[24] = {LCD_UTIL_COLOR_BLUE, LCD_UTIL_COLOR_GREEN, LCD_UTIL_COLOR_RED, LCD_UTIL_COLOR_CYAN, LCD_UTIL_COLOR_MAGENTA, LCD_UTIL_COLOR_YELLOW,
    LCD_UTIL_COLOR_LIGHTBLUE, LCD_UTIL_COLOR_LIGHTGREEN, LCD_UTIL_COLOR_LIGHTRED, LCD_UTIL_COLOR_LIGHTCYAN, LCD_UTIL_COLOR_LIGHTMAGENTA,
    LCD_UTIL_COLOR_LIGHTYELLOW, LCD_UTIL_COLOR_DARKBLUE, LCD_UTIL_COLOR_DARKGREEN, LCD_UTIL_COLOR_DARKRED, LCD_UTIL_COLOR_DARKCYAN,
    LCD_UTIL_COLOR_DARKMAGENTA, LCD_UTIL_COLOR_DARKYELLOW, LCD_UTIL_COLOR_LIGHTGRAY, LCD_UTIL_COLOR_GRAY, LCD_UTIL_COLOR_DARKGRAY,
    LCD_UTIL_COLOR_BLACK, LCD_UTIL_COLOR_BROWN, LCD_UTIL_COLOR_ORANGE };
    uint32_t ts_status = BSP_ERROR_NONE;
    uint8_t lcd_string[TOUCH_INFO_STRING_SIZE] = "";
    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(&x_size);
    BSP_LCD_GetYSize(&y_size);

    /* Check in polling mode in touch screen the touch status and coordinates */
    /* of touches if touch occurred                                           */
    ts_status = BSP_TS_Get_MultiTouchState(&TS_MTState);
    if(TS_MTState.TouchDetected)
    {
        /* One or dual touch have been detected  */

        /* Erase previous information on touchscreen play pad area */
        LCD_UTIL_FillRect(0, 80, x_size, y_size - 160, LCD_UTIL_COLOR_WHITE);

        /* Re-Draw touch screen play area on LCD */
        LCD_UTIL_DrawRect(10, 90, x_size - 20, y_size - 180, LCD_UTIL_COLOR_BLUE);
        LCD_UTIL_DrawRect(11, 91, x_size - 22, y_size - 182, LCD_UTIL_COLOR_BLUE);

        /* Erase previous information on bottom text bar */
        LCD_UTIL_FillRect(0, y_size - 80, x_size, 80, LCD_UTIL_COLOR_BLUE);

        /* Deactivate drawing footprint of touch 1 and touch 2 until validated against boundaries of touch pad values */
        drawTouch1 = drawTouch2 = 0;

        /* Get X and Y position of the first touch post calibrated */
        x1 = TS_MTState.TouchX[0];
        y1 = TS_MTState.TouchY[0];

        if((y1 > (90 + TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)) &&
        (y1 < (y_size - 90 - TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)))
        {
        drawTouch1 = 1;
        }

        /* If valid touch 1 position : inside the reserved area for the use case : draw the touch */
        if(drawTouch1 == 1)
        {
        /* Draw circle of first touch : turn on colors[] table */
        LCD_UTIL_FillCircle(x1, y1, TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS, colors[(touchscreen_color_idx++ % 24)]);

        //   LCD_UTIL_SetTextColor(LCD_UTIL_COLOR_WHITE);
        //   LCD_UTIL_SetFont(&Font16);
        LCD_UTIL_DisplayString(0, y_size - 70, FONTSIZE_16, LCD_UTIL_COLOR_WHITE, (uint8_t *)"TOUCH INFO : ", CENTER_MODE);

        //   LCD_UTIL_SetFont(&Font12);
        sprintf((char*)lcd_string, "x1 = %u, y1 = %u, Event = %s, Weight = %lu",
                x1,
                y1,
                ts_event_string_tab[TS_MTState.TouchEvent[0]],
                TS_MTState.TouchWeight[0]);
        LCD_UTIL_DisplayString(0, y_size - 45, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, lcd_string, CENTER_MODE);
        } /* of if(drawTouch1 == 1) */

        if(TS_MTState.TouchDetected > 1)
        {
        /* Get X and Y position of the second touch post calibrated */
        x2 = TS_MTState.TouchX[1];
        y2 = TS_MTState.TouchY[1];

        if((y2 > (90 + TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)) &&
            (y2 < (y_size - 90 - TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS)))
        {
            drawTouch2 = 1;
        }

        /* If valid touch 2 position : inside the reserved area for the use case : draw the touch */
        if(drawTouch2 == 1)
        {
            sprintf((char*)lcd_string, "x2 = %u, y2 = %u, Event = %s, Weight = %lu",
                    x2,
                    y2,
                    ts_event_string_tab[TS_MTState.TouchEvent[1]],
                    TS_MTState.TouchWeight[1]);
            LCD_UTIL_DisplayString(0, y_size - 35, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, lcd_string, CENTER_MODE);

            /* Draw circle of second touch : turn on color[] table */
            LCD_UTIL_FillCircle(x2, y2, TS_MULTITOUCH_FOOTPRINT_CIRCLE_RADIUS, colors[(touchscreen_color_idx++ % 24)]);
        } /* of if(drawTouch2 == 1) */

        } /* of if(TS_MTState.TouchDetected > 1) */

        if((drawTouch1 == 1) || (drawTouch2 == 1))
        {
        /* Get updated gesture Id in global variable 'TS_State' */
        ts_status = BSP_TS_GetGestureId(&GestureId);

        sprintf((char*)lcd_string, "Gesture Id = %s", ts_gesture_id_string_tab[GestureId]);
        }
        else
        {
        /* Invalid touch position */
        //   LCD_UTIL_SetTextColor(LCD_UTIL_COLOR_WHITE);
        LCD_UTIL_DisplayString(0, y_size - 70, FONTSIZE_12, LCD_UTIL_COLOR_WHITE, (uint8_t *)"Invalid touch position : use drawn touch area : ", CENTER_MODE);
        }
    } /* of if(TS_MTState.TouchDetected) */

    return(ts_status);
}

#endif /* USE_TS_MULTI_TOUCH == 1 */



void appGUI_MainProc(void)
{
    static uint8_t DemoInitFlag = 0;
    if (gucWukpKeyState == 0)
    {
        if ((0 == DemoInitFlag) || (2 == DemoInitFlag))
        {
            DemoInitFlag = 1;
            TouchScreenDemo1_Init();
        }
        
        TouchScreenDemo1();
    }
    else
    {
        if (1 == DemoInitFlag)
        {
            DemoInitFlag = 2;
            TouchScreenDemo2_Init();
        }
        
        Touchscreen_demo2();
    }
}









