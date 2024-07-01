/*
*********************************************************************************************************
*
*   [module   ] : LCD (For STM32H7)
*   [file name] : bsp_lcd_ts.c
*   [version  ] : V1.0
*   [brief    ] : touchscreen module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp_lcd_ts.h"
#include "bsp_i2c.h"
#include "bsp.h"

/*************************** Macros Defines ***************************/
typedef void (* BSP_EXTI_LineCallback) (void);

#define TS_MIN(a,b) ((a > b) ? b : a)

/*************************** Privates Functions ***************************/
static void TS_EXTI_Callback(void);

/*************************** Privates Variables ***************************/
static TS_Drv_t           *Ts_Drv = NULL;
static FT6X06_Object_t    FT6X06Obj;

/** @brief EXTI INT Line */
static EXTI_HandleTypeDef hts_exti = {0};
static const uint32_t TS_EXTI_LINE = {TS_INT_LINE};
static BSP_EXTI_LineCallback TsCallback = {TS_EXTI_Callback}; 

/*************************** Exported Variables ***************************/
TS_Ctx_t           Ts_Ctx = {0};
void               *Ts_CompObj = {0};



/*************************** Local Functions ***************************/

/*********************************************************************************************/
/******                         touch sensor hardware Functions                         *******/
/*********************************************************************************************/
/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval BSP status
  */
static int32_t FT6X06_Probe(void)
{
    int32_t ret;
    FT6X06_IO_t IOCtx;
    FT6X06_Capabilities_t Cap;
    uint32_t id;

    /* Configure the touch screen driver */
    IOCtx.Init      = BSP_I2C4_Init;
    IOCtx.DeInit    = BSP_I2C4_DeInit;
    IOCtx.ReadReg   = BSP_I2C4_ReadReg;
    IOCtx.WriteReg  = BSP_I2C4_WriteReg;
    IOCtx.GetTick   = BSP_GetTick;
    IOCtx.Address   = 0x70U;//TS_I2C_ADDRESS;

    if (FT6X06_RegisterBusIO(&FT6X06Obj, &IOCtx) != FT6X06_OK)
    {
        ret = BSP_ERROR_BUS_FAILURE;
    }
    else if (FT6X06_ReadID(&FT6X06Obj, &id) != FT6X06_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if ((id != FT6X36_ID) && (id != FT6X06_ID))
    {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
        printf("touch sensor id: 0x%x\r\n", id);
        (void)FT6X06_GetCapabilities(&FT6X06Obj, &Cap);
        Ts_CompObj = &FT6X06Obj;
        Ts_Drv = (TS_Drv_t *)&FT6X06_TS_Driver;

        if (Ts_Drv->Init(Ts_CompObj) != FT6X06_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_NONE;
        }
    }

    return ret;
}

/**
  * @brief  Initializes and configures the touch screen functionalities and
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..).
  * @param  TS_Init  TS Init structure
  * @retval BSP status
  */
int32_t BSP_TS_Init(TS_Init_t *TS_Init)
{
    int32_t ret = BSP_ERROR_NONE;

    if ((TS_Init->Width == 0U) || (TS_Init->Width > TS_MAX_WIDTH) ||
        (TS_Init->Height == 0U) || (TS_Init->Height > TS_MAX_HEIGHT) ||
        (TS_Init->Accuracy > TS_MIN((TS_Init->Width), (TS_Init->Height))))
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (FT6X06_Probe() != BSP_ERROR_NONE)
        {
            ret = BSP_ERROR_NO_INIT;
        }
        else
        {
            TS_Capabilities_t Capabilities;
            Ts_Ctx.Width = TS_Init->Width;
            Ts_Ctx.Height = TS_Init->Height;
            Ts_Ctx.Orientation = TS_Init->Orientation;
            Ts_Ctx.Accuracy = TS_Init->Accuracy;

            if (Ts_Drv->GetCapabilities(Ts_CompObj, &Capabilities) < 0)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
            else
            {
                /* Store maximum X and Y on context */
                Ts_Ctx.MaxX = Capabilities.MaxXl;
                Ts_Ctx.MaxY = Capabilities.MaxYl;
                /* Initialize previous position in order to always detect first touch */
                for (int i = 0; i < TS_TOUCH_NBR; i++)
                {
                    Ts_Ctx.PreviousX[i] = TS_Init->Width + TS_Init->Accuracy + 1U;
                    Ts_Ctx.PreviousY[i] = TS_Init->Height + TS_Init->Accuracy + 1U;
                }
            }
        }
    }

    return ret;
}

/**
  * @brief  De-Initializes the touch screen functionalities
  * @retval BSP status
  */
int32_t BSP_TS_DeInit(void)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Ts_Drv != NULL)
    {
        if (Ts_Drv->DeInit(Ts_CompObj) < 0)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_NONE;
        }
    }

    return ret;
}


/**
  * @brief  Get Touch Screen instance capabilities
  * @param  Capabilities pointer to Touch Screen capabilities
  * @retval BSP status
  */
int32_t BSP_TS_GetCapabilities(TS_Capabilities_t *Capabilities)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Ts_Drv != NULL)
    {
        (void)Ts_Drv->GetCapabilities(Ts_CompObj, Capabilities);
        ret = BSP_ERROR_NONE;
    }

    return ret;
}

/**
  * @brief  Configures and enables the touch screen interrupts.
  * @retval BSP status
  */
int32_t BSP_TS_EnableIT(void)
{
    int32_t ret = BSP_ERROR_NONE;
    GPIO_InitTypeDef gpio_init_structure;

    if (Ts_Drv != NULL)
    {
        TS_INT_GPIO_CLK_ENABLE();

        /* Configure Interrupt mode for TS_INT pin falling edge : when a new touch is available */
        /* TS_INT pin is active on low level on new touch available */
        gpio_init_structure.Pin = TS_INT_PIN;
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
        gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;
        HAL_GPIO_Init(TS_INT_GPIO_PORT, &gpio_init_structure);

        if (Ts_Drv->EnableIT(Ts_CompObj) < 0)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            (void)HAL_EXTI_GetHandle(&hts_exti, TS_EXTI_LINE);
            (void)HAL_EXTI_RegisterCallback(&hts_exti, HAL_EXTI_COMMON_CB_ID, TsCallback);
            HAL_NVIC_SetPriority((IRQn_Type)(TS_INT_EXTI_IRQn), BSP_TS_IT_PRIORITY, 0x00);
            HAL_NVIC_EnableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
            ret = BSP_ERROR_NONE;
        }
    }
    return ret;
}

/**
  * @brief  Disables the touch screen interrupts.
  * @retval BSP status
  */
int32_t BSP_TS_DisableIT(void)
{
    int32_t ret = BSP_ERROR_NONE;

    GPIO_InitTypeDef gpio_init_structure;

    if (Ts_Drv != NULL)
    {
        gpio_init_structure.Pin = TS_INT_PIN;
        HAL_GPIO_DeInit(TS_INT_GPIO_PORT, gpio_init_structure.Pin);
        HAL_NVIC_DisableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
        if (Ts_Drv->DisableIT(Ts_CompObj) != FT6X06_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            ret = BSP_ERROR_NONE;
        }
    }

    return ret;
}

/**
  * @brief  BSP TS Callback.
  * @retval None.
  */
__weak void BSP_TS_Callback(void)
{
  /* Prevent unused argument(s) compilation warning */

  /* This function should be implemented by the user application.
     It is called into this driver when an event on TS touch detection */
}

/**
  * @brief  TS EXTI touch detection callbacks.
  * @retval None
  */
static void TS_EXTI_Callback(void)
{
    BSP_TS_Callback();
}

/**
  * @brief  This function handles TS interrupt request.
  * @param  Instance TS instance
  * @retval None
  */
void BSP_TS_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&hts_exti);
}


/*********************************************************************************************/
/******                         touch sensor property Functions                        *******/
/*********************************************************************************************/


/**
  * @brief  Returns positions of a single touch screen.
  * @param  TS_State  Pointer to touch screen current state structure
  * @retval BSP status
  */
int32_t BSP_TS_GetState(TS_State_t *TS_State)
{
    int32_t ret = BSP_ERROR_NONE;
    uint32_t x_oriented, y_oriented;
    uint32_t x_diff, y_diff;

    if (Ts_Drv != NULL)
    {
        FT6X06_State_t state;

        /* Get each touch coordinates */
        if (Ts_Drv->GetState(Ts_CompObj, &state) < 0)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Check and update the number of touches active detected */
        else if (state.TouchDetected != 0U)
        {
            x_oriented = state.TouchX;
            y_oriented = state.TouchY;

            if ((Ts_Ctx.Orientation & TS_SWAP_XY) == TS_SWAP_XY)
            {
                x_oriented = state.TouchY;
                y_oriented = state.TouchX;
            }

            if ((Ts_Ctx.Orientation & TS_SWAP_X) == TS_SWAP_X)
            {
                x_oriented = Ts_Ctx.MaxX - x_oriented - 1UL;
            }

            if ((Ts_Ctx.Orientation & TS_SWAP_Y) == TS_SWAP_Y)
            {
                y_oriented = Ts_Ctx.MaxY - y_oriented - 1UL;
            }

            /* Apply boundary */
            TS_State->TouchX = (x_oriented * Ts_Ctx.Width) / Ts_Ctx.MaxX;
            TS_State->TouchY = (y_oriented * Ts_Ctx.Height) / Ts_Ctx.MaxY;
            /* Store Current TS state */
            TS_State->TouchDetected = state.TouchDetected;

            /* Check accuracy */
            x_diff = (TS_State->TouchX > Ts_Ctx.PreviousX[0]) ? (TS_State->TouchX - Ts_Ctx.PreviousX[0]) : (Ts_Ctx.PreviousX[0] - TS_State->TouchX);

            y_diff = (TS_State->TouchY > Ts_Ctx.PreviousY[0]) ? (TS_State->TouchY - Ts_Ctx.PreviousY[0]) : (Ts_Ctx.PreviousY[0] - TS_State->TouchY);

            if ((x_diff > Ts_Ctx.Accuracy) || (y_diff > Ts_Ctx.Accuracy))
            {
                /* New touch detected */
                Ts_Ctx.PreviousX[0] = TS_State->TouchX;
                Ts_Ctx.PreviousY[0] = TS_State->TouchY;
            }
            else
            {
                TS_State->TouchX = Ts_Ctx.PreviousX[0];
                TS_State->TouchY = Ts_Ctx.PreviousY[0];
            }
        }
        else
        {
            TS_State->TouchDetected = 0U;
            TS_State->TouchX = Ts_Ctx.PreviousX[0];
            TS_State->TouchY = Ts_Ctx.PreviousY[0];
        }
    }

    return ret;
}

#if (USE_TS_MULTI_TOUCH > 0)
/**
  * @brief  Returns positions of multi touch screen.
  * @param  Instance  TS instance. Could be only 0.
  * @param  TS_State  Pointer to touch screen current state structure
  * @retval BSP status
  */
int32_t BSP_TS_Get_MultiTouchState(TS_MultiTouch_State_t *TS_State)
{
    int32_t ret = BSP_ERROR_NONE;
    uint32_t x_oriented[2], y_oriented[2];
    uint32_t x_diff, y_diff;
    uint32_t index;
    if (Ts_Drv != NULL)
    {
        TS_MultiTouch_State_t state;
        /* Get each touch coordinates */
        if (Ts_Drv->GetMultiTouchState(Ts_CompObj, &state) < 0)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Check and update the number of touches active detected */
        else if (state.TouchDetected != 0U)
        {
            for (index = 0; index < state.TouchDetected; index++)
            {
                x_oriented[index] = state.TouchX[index];
                y_oriented[index] = state.TouchY[index];

                if ((Ts_Ctx.Orientation & TS_SWAP_XY) == TS_SWAP_XY)
                {
                    x_oriented[index] = state.TouchY[index];
                    y_oriented[index] = state.TouchX[index];
                }

                if ((Ts_Ctx.Orientation & TS_SWAP_X) == TS_SWAP_X)
                {
                    x_oriented[index] = Ts_Ctx.MaxX - x_oriented[index] - 1UL;
                }

                if ((Ts_Ctx.Orientation & TS_SWAP_Y) == TS_SWAP_Y)
                {
                    y_oriented[index] = Ts_Ctx.MaxY - y_oriented[index] - 1UL;
                }

                /* Apply boundary */
                TS_State->TouchX[index] = (x_oriented[index] * Ts_Ctx.Width) / Ts_Ctx.MaxX;
                TS_State->TouchY[index] = (y_oriented[index] * Ts_Ctx.Height) / Ts_Ctx.MaxY;
                /* Store Current TS state */
                TS_State->TouchDetected = state.TouchDetected;

                /* Check accuracy */
                x_diff = (TS_State->TouchX[index] > Ts_Ctx.PreviousX[0]) ? (TS_State->TouchX[index] - Ts_Ctx.PreviousX[0]) : (Ts_Ctx.PreviousX[0] - TS_State->TouchX[index]);

                y_diff = (TS_State->TouchY[index] > Ts_Ctx.PreviousY[0]) ? (TS_State->TouchY[index] - Ts_Ctx.PreviousY[0]) : (Ts_Ctx.PreviousY[0] - TS_State->TouchY[index]);

                if ((x_diff > Ts_Ctx.Accuracy) || (y_diff > Ts_Ctx.Accuracy))
                {
                    /* New touch detected */
                    Ts_Ctx.PreviousX[index] = TS_State->TouchX[index];
                    Ts_Ctx.PreviousY[index] = TS_State->TouchY[index];
                }
                else
                {
                    TS_State->TouchX[index] = Ts_Ctx.PreviousX[index];
                    TS_State->TouchY[index] = Ts_Ctx.PreviousY[index];
                }
            }
        }
        else
        {
            TS_State->TouchDetected = 0U;
            for (index = 0; index < TS_TOUCH_NBR; index++)
            {
                TS_State->TouchX[index] = Ts_Ctx.PreviousX[index];
                TS_State->TouchY[index] = Ts_Ctx.PreviousY[index];
            }
        }
    }

    return ret;
}
#endif /* USE_TS_MULTI_TOUCH > 1 */

#if (USE_TS_GESTURE > 0)
/**
  * @brief  Update gesture Id following a touch detected.
  * @param  Instance      TS instance. Could be only 0.
  * @param  GestureConfig Pointer to gesture configuration structure
  * @retval BSP status
  */
int32_t BSP_TS_GestureConfig(TS_Gesture_Config_t *GestureConfig)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Ts_Drv != NULL)
    {
        if (Ts_Drv->GestureConfig(Ts_CompObj, GestureConfig) < 0)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }

    return ret;
}

/**
  * @brief  Update gesture Id following a touch detected.
  * @param  GestureId  Pointer to gesture ID
  * @retval BSP status
  */
int32_t BSP_TS_GetGestureId(uint32_t *GestureId)
{
    int32_t ret = BSP_ERROR_NONE;
    uint8_t tmp = 0;

    if (Ts_Drv == NULL)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    } /* Get gesture Id */
    else if (Ts_Drv->GetGesture(Ts_CompObj, &tmp) < 0)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        /* Remap gesture Id to a TS_Gesture_Id_t value */
        switch (tmp)
        {
        case FT6X06_GEST_ID_NO_GESTURE:
            *GestureId = GESTURE_ID_NO_GESTURE;
            break;
        case FT6X06_GEST_ID_MOVE_UP:
            *GestureId = GESTURE_ID_MOVE_UP;
            break;
        case FT6X06_GEST_ID_MOVE_RIGHT:
            *GestureId = GESTURE_ID_MOVE_RIGHT;
            break;
        case FT6X06_GEST_ID_MOVE_DOWN:
            *GestureId = GESTURE_ID_MOVE_DOWN;
            break;
        case FT6X06_GEST_ID_MOVE_LEFT:
            *GestureId = GESTURE_ID_MOVE_LEFT;
            break;
        case FT6X06_GEST_ID_ZOOM_IN:
            *GestureId = GESTURE_ID_ZOOM_IN;
            break;
        case FT6X06_GEST_ID_ZOOM_OUT:
            *GestureId = GESTURE_ID_ZOOM_OUT;
            break;
        default:
            *GestureId = GESTURE_ID_NO_GESTURE;
            break;
        }

        ret = BSP_ERROR_NONE;
    }

    return ret;
}
#endif /* USE_TS_GESTURE > 0 */

/**
  * @brief  Set TS orientation
  * @param  Orientation Orientation to be set
  * @retval BSP status
  */
int32_t BSP_TS_Set_Orientation(uint32_t Orientation)
{
    Ts_Ctx.Orientation = Orientation;
    return BSP_ERROR_NONE;
}

/**
  * @brief  Get TS orientation
  * @param  Orientation Current Orientation to be returned
  * @retval BSP status
  */
int32_t BSP_TS_Get_Orientation(uint32_t *Orientation)
{
    *Orientation = Ts_Ctx.Orientation;
    return BSP_ERROR_NONE;
}



