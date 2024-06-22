/*
*********************************************************************************************************
*
*   [module   ] : USART (For STM32H7)
*   [file name] : bsp_uart.h
*   [version  ] : V1.0
*   [brief    ] : USART module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/

#ifndef BSP_USART_H_
#define BSP_USART_H_

#include "stm32h7xx_hal.h"

#define BSP_UART1_EN  1
#define BSP_UART2_EN  0
#define BSP_UART3_EN  0
#define BSP_UART4_EN  0

typedef enum
{
    SERIAL_COM1 = 0,
    SERIAL_COM2 = 1,
    SERIAL_COM3 = 2,
    SERIAL_COM4 = 3,

    SERIAL_COM_MAX_NUM,
}COM_PORT_ENUM;

#if (BSP_UART1_EN == 1)
    #define UART1_BAUD      115200
    #define UART1_TX_BUF_SIZE  1*1024
    #define UART1_RX_BUF_SIZE  1*1024
#endif

#if (BSP_UART2_EN == 1)
    #define UART2_BAUD      115200
    #define UART2_TX_BUF_SIZE  1*1024
    #define UART2_RX_BUF_SIZE  1*1024
#endif

#if (BSP_UART3_EN == 1)
    #define UART3_BAUD      115200
    #define UART3_TX_BUF_SIZE  1*1024
    #define UART3_RX_BUF_SIZE  1*1024
#endif

#if (BSP_UART4_EN == 1)
    #define UART4_BAUD      115200
    #define UART4_TX_BUF_SIZE  1*1024
    #define UART4_RX_BUF_SIZE  1*1024
#endif

/* uart com device struct */
typedef struct
{
    USART_TypeDef *huart;
    uint8_t *pTxBuf;
    uint8_t *pRxBuf;
    uint16_t usTxBufSize;
    uint16_t usRxBufSize;

    volatile uint16_t usTxWrite;
    volatile uint16_t usTxRead;
    volatile uint16_t usTxCount;

    volatile uint16_t usRxWrite;
    volatile uint16_t usRxRead;
    volatile uint16_t usRxCount;

    uint8_t isSending;
}BSP_UART_STRU;



void bsp_InitUart(void);
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode);

void comSendBuf(COM_PORT_ENUM ucPort, uint8_t *aucBuf, uint16_t usLen);
void comSendChar(COM_PORT_ENUM ucPort, uint8_t ucByte);
uint8_t comGetChar(COM_PORT_ENUM ucPort, uint8_t *pByte);
void comClearTxFifo(COM_PORT_ENUM ucPort);
void comClearRxFifo(COM_PORT_ENUM ucPort);

#endif /* BSP_USART_H_ */
