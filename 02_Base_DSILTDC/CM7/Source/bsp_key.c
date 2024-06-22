#include "bsp_key.h"


#define HARD_KEY_NUM        6
#define KEY_COUNT           (HARD_KEY_NUM)

#define ALL_KEY_GPIO_CLK_ENABLE() { \
        __HAL_RCC_GPIOC_CLK_ENABLE();   \
        __HAL_RCC_GPIOK_CLK_ENABLE();   \
    };

typedef struct
{
    GPIO_TypeDef* gpio;
    uint16_t pin;
    uint8_t ActiveLevel;
}M_GPIO_T;

static const M_GPIO_T s_gpio_list[HARD_KEY_NUM] = {
    {GPIOC, GPIO_PIN_13, 1},  /* WAKEWP */
    {GPIOK, GPIO_PIN_6, 0},  /* JOY_U */
    {GPIOK, GPIO_PIN_3, 0},  /* JOY_D */
    {GPIOK, GPIO_PIN_4, 0},  /* JOY_L */
    {GPIOK, GPIO_PIN_5, 0},  /* JOY_R */
    {GPIOK, GPIO_PIN_2, 0},  /* JOY_OK */
};

static KEY_T s_tBtn[KEY_COUNT] = {0};
static KEY_FIFO_T s_tKey;

static void bsp_InitKeyVar(void);
static void bsp_InitKeyHard(void);

/**
 * @brief determine whether the key is pressed
 * 
 * @param ucKeyId 
 * @return uint8_t 1: pressed (on), 0: not pressed (release)
 */
static uint8_t KeyPinActive(uint8_t ucKeyId)
{
    uint8_t level;

    if ((s_gpio_list[ucKeyId].gpio->IDR & s_gpio_list[ucKeyId].pin) == 0)
    {
        level = 0;
    }
    else
    {
        level = 1;
    }

    if (level == s_gpio_list[ucKeyId].ActiveLevel)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Determine whether any key is pressed
 * 
 * @param ucKeyId 
 * @return uint8_t 1: pressed (on), 0: not pressed (release)
 */
static uint8_t IsKeyDownFunc(uint8_t ucKeyId)
{
	/* 实体单键 */
	if (ucKeyId < HARD_KEY_NUM)
	{
		uint8_t i;
		uint8_t count = 0;
		uint8_t save = 255;
		
		/* how many keys are pressed */
		for (i = 0; i < HARD_KEY_NUM; i++)
		{
			if (KeyPinActive(i)) 
			{
				count++;
				save = i;
			}
		}
		
		if (count == 1 && save == ucKeyId)
		{
			return 1;	/* It works only one key is pressed */
		}		

		return 0;
	}
	
	return 0;
}

/**
 * @brief init key module, should be called by bsp_Init()
 * 
 */
void bsp_InitKey(void)
{
    bsp_InitKeyVar();
    bsp_InitKeyHard();
}


static void bsp_InitKeyHard(void)
{
    GPIO_InitTypeDef gpio_init;
    uint8_t i;

    ALL_KEY_GPIO_CLK_ENABLE();

    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    for (i = 0; i < HARD_KEY_NUM; i++)
    {
        if (i >= KID_JOY_U)
        {
            gpio_init.Pull = GPIO_PULLUP;
        }
        gpio_init.Pin = s_gpio_list[i].pin;
        HAL_GPIO_Init(s_gpio_list[i].gpio, &gpio_init);
    }
}

static void bsp_InitKeyVar(void)
{
    uint8_t i;

    s_tKey.Read = 0;
    s_tKey.Write = 0;

    for (i = 0; i < KEY_COUNT; i++)
    {
        s_tBtn[i].LongTime = KEY_LONG_TIME;
        s_tBtn[i].Count = KEY_FILTER_TIME / 2;
        s_tBtn[i].State = 0;
        s_tBtn[i].RepeatSpeed = 0;
        s_tBtn[i].RepeatCount = 0;
    }

    /* If you need to change the parameter of a key separately, you can reassign it separately here */

    /* Supports holding down for 1s */
    bsp_SetKeyParam(KID_JOY_U, 100, 6);
    bsp_SetKeyParam(KID_JOY_D, 100, 6);
    bsp_SetKeyParam(KID_JOY_L, 100, 6);
    bsp_SetKeyParam(KID_JOY_R, 100, 6);
}

/**
 * @brief get key press state
 * 
 * @param ucKeyID key ID
 * @return uint8_t 1:pressed , 0:unpressed
 */
uint8_t bsp_GetKeyState(KEY_ID_E ucKeyID)
{
	return s_tBtn[ucKeyID].State;
}

/**
 * @brief Set key parameters
 * 
 * @param ucKeyID : key ID, start from 0
 * @param usLongTime : long press event time
 * @param ucRepeatSpeed : key repeat event speed
 */
void bsp_SetKeyParam(uint8_t ucKeyID, uint16_t usLongTime, uint8_t ucRepeatSpeed)
{
    s_tBtn[ucKeyID].LongTime = usLongTime;        /* longtime = 0 indicates that long key events are not detected */
    s_tBtn[ucKeyID].RepeatSpeed = ucRepeatSpeed;
    s_tBtn[ucKeyID].RepeatCount = 0;
}

/**
 * @brief Pushes a key value into the key FIFO buffer. Can be used to simulate a keystroke
 * 
 * @param ucKeyActionCode 
 */
void bsp_PutKey(uint8_t ucKeyActionCode)
{
    s_tKey.Buf[s_tKey.Write] = ucKeyActionCode;

    if (++s_tKey.Write  >= KEY_FIFO_SIZE)
    {
        s_tKey.Write = 0;
    }
}

/**
 * @brief Reads a key value from the key FIFO buffer
 * 
 * @return uint8_t key action coder
 */
uint8_t bsp_GetKey(void)
{
    uint8_t ret;

    if (s_tKey.Read == s_tKey.Write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = s_tKey.Buf[s_tKey.Read];

        if (++s_tKey.Read >= KEY_FIFO_SIZE)
        {
            s_tKey.Read = 0;
        }
        return ret;
    }
}

/**
 * @brief Clear the key FIFO buffer
 * 
 */
void bsp_ClearKey(void)
{
	s_tKey.Read = s_tKey.Write;
}

/**
 * @brief detect a key. non-blocking state , 10ms period
 * 
 * @param ucKeyId 
 */
static void bsp_DetectKey(uint8_t ucKeyId)
{
    KEY_T *pBtn;

    pBtn = &s_tBtn[ucKeyId];
    if (BSP_KEY_PRESSED == IsKeyDownFunc(ucKeyId))
    {
        if (pBtn->Count < KEY_FILTER_TIME)
        {
            pBtn->Count = KEY_FILTER_TIME;
        }
        else if(pBtn->Count < 2 * KEY_FILTER_TIME)
        {
            pBtn->Count++;
        }
        else
        {
            if (pBtn->State == 0)
            {
                pBtn->State = 1;

                bsp_PutKey((uint8_t)(3 * ucKeyId + 1));
            }

            if (pBtn->LongTime > 0)
            {
                if (pBtn->LongCount < pBtn->LongTime)
                {
                    /* Send a message when the button is pressed continuously */
                    if (++pBtn->LongCount == pBtn->LongTime)
                    {
                        /* Key value into key FIFO */
                        bsp_PutKey((uint8_t)(3 * ucKeyId + 3));
                    }
                }
                else
                {
                    if (pBtn->RepeatSpeed > 0)
                    {
                        if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
                        {
                            pBtn->RepeatCount = 0;

                            bsp_PutKey((uint8_t)(3 * ucKeyId + 1));
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(pBtn->Count > KEY_FILTER_TIME)
        {
            pBtn->Count = KEY_FILTER_TIME;
        }
        else if(pBtn->Count != 0)
        {
            pBtn->Count--;
        }
        else
        {
            if (pBtn->State == 1)
            {
                pBtn->State = 0;

                bsp_PutKey((uint8_t)(3 * ucKeyId + 2));
            }
        }

        pBtn->LongCount = 0;
        pBtn->RepeatCount = 0;
    }
}

/**
 * @brief scan all keys. non-blocking, interrupted periodically by systick, 10ms period
 * 
 */
void bsp_KeyScan10ms(void)
{
    uint8_t iloop = 0;

    for (iloop = 0; iloop < KEY_COUNT; iloop++)
    {
        bsp_DetectKey(iloop);
    }
}

