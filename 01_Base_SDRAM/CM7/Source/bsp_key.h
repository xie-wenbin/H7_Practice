/*
*********************************************************************************************************
*
*   [module   ] : KEYS & JOY (For STM32H7)
*   [file name] : bsp_key.h
*   [version  ] : V1.0
*   [brief    ] : KEYS & JOY module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/

#ifndef BSP_KEY_H_
#define BSP_KEY_H_

#include "stm32h7xx_hal.h"

#define BSP_KEY_RELEASED                    0U
#define BSP_KEY_PRESSED                     1U

typedef enum
{
    KID_K1 = 0,
    KID_JOY_U,
    KID_JOY_D,
    KID_JOY_L,
    KID_JOY_R,
    KID_JOY_OK
} KEY_ID_E;

typedef enum
{
    KEY_NONE = 0,

    KEY_1_DOWN,
    KEY_1_UP,
    KEY_1_LONG,

    KEY_2_DOWN,
    KEY_2_UP,
    KEY_2_LONG,

    KEY_3_DOWN,
    KEY_3_UP,
    KEY_3_LONG,

    KEY_4_DOWN,
    KEY_4_UP,
    KEY_4_LONG,

    KEY_5_DOWN,
    KEY_5_UP,
    KEY_5_LONG,

    KEY_6_DOWN,
    KEY_6_UP,
    KEY_6_LONG,
} KEY_ACTION_ENUM;

/*
    Key filtering time 50ms, unit 10ms
    only when the status is continuously detected for 50ms is consider vaild, including two events of pressing and bouncing
*/
#define KEY_FILTER_TIME   5
#define KEY_LONG_TIME     100			/* uint 10ms， continuously detected 1s as long press event */

typedef struct
{
    uint8_t Count;       /* Filter counter */
    uint16_t LongCount;  /* Long press counter */
    uint16_t LongTime;   /* Key press duration. 0 indicates that long press is not detected */
    uint8_t State;       /* Key current status (pressed or unpressed) */
    uint8_t RepeatSpeed; /* Continuous keying cycle */
    uint8_t RepeatCount; /* ontinuous key counter */
} KEY_T;

#define KEY_FIFO_SIZE 10

typedef struct
{
    uint8_t Buf[KEY_FIFO_SIZE]; /* Key-value buffer */
    uint8_t Read;
    uint8_t Write;
} KEY_FIFO_T;

void bsp_InitKey(void);
uint8_t bsp_GetKeyState(KEY_ID_E ucKeyID);
void bsp_SetKeyParam(uint8_t ucKeyID, uint16_t usLongTime, uint8_t ucRepeatSpeed);
uint8_t bsp_GetKey(void);
void bsp_ClearKey(void);
void bsp_KeyScan10ms(void);

#endif /* BSP_KEY_H_ */
