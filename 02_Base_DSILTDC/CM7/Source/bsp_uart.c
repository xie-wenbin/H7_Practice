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
#include "bsp.h"
#include "bsp_uart.h"

#define UART_POLL_TIMEOUT              1000

/* Definition for COM port1, connected to USART1 */
#define UART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()
#define UART1_CLK_DISABLE()             __HAL_RCC_USART1_CLK_DISABLE()

#define UART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define UART1_TX_PIN                    GPIO_PIN_9
#define UART1_TX_GPIO_PORT              GPIOA
#define UART1_TX_AF                     GPIO_AF7_USART1

#define UART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define UART1_RX_PIN                    GPIO_PIN_10
#define UART1_RX_GPIO_PORT              GPIOA
#define UART1_RX_AF                     GPIO_AF7_USART1

/* Definition for COM port2, connected to USART2 */
//TODO

/* Definition for COM port3, connected to USART3 */
//TODO

/* Definition for COM port4, connected to USART4 */
//TODO

#if (BSP_UART1_EN == 1)
    static BSP_UART_STRU gComUart1 = {0};
    uint8_t gTxBufCom1[UART1_TX_BUF_SIZE] = {0};
    uint8_t gRxBufCom1[UART1_RX_BUF_SIZE] = {0};
#endif

#if (BSP_UART2_EN == 1)
    static BSP_UART_STRU gComUart2;
    uint8_t gTxBufCom2[UART2_TX_BUF_SIZE];
    uint8_t gRxBufCom2[UART2_RX_BUF_SIZE];
#endif

#if (BSP_UART3_EN == 1)
    static BSP_UART_STRU gComUart3;
    uint8_t gTxBufCom3[UART3_TX_BUF_SIZE];
    uint8_t gRxBufCom3[UART3_RX_BUF_SIZE];
#endif

#if (BSP_UART4_EN == 1)
    static BSP_UART_STRU gComUart4;
    uint8_t gTxBufCom4[UART4_TX_BUF_SIZE];
    uint8_t gRxBufCom4[UART4_RX_BUF_SIZE];
#endif

/*======================================= [FUNCTION DECELARE] =======================================*/

static void UartVarInit(void);
static void InitHardUart(void);

static void UartSend(BSP_UART_STRU *pUartHld, uint8_t *aucBuf, uint16_t usLen);
static uint8_t UartGetChar(BSP_UART_STRU *pUartHld, uint8_t *pByte);
static void UartCommIRQHandler(BSP_UART_STRU *pUartHld);

/*======================================= [BSP UART FUNCTION] =======================================*/
void bsp_InitUart(void)
{
    UartVarInit();
    InitHardUart();
}

/**
 * @brief initializes serial port related variables
 * 
 */
static void UartVarInit(void)
{
#if (BSP_UART1_EN == 1)
    gComUart1.huart = USART1;
    gComUart1.pTxBuf = gTxBufCom1;
    gComUart1.pRxBuf = gRxBufCom1;
    gComUart1.usTxBufSize = UART1_TX_BUF_SIZE;
    gComUart1.usRxBufSize = UART1_RX_BUF_SIZE;
    gComUart1.usTxWrite = 0;
    gComUart1.usTxRead = 0;
    gComUart1.usRxWrite = 0;
    gComUart1.usRxRead = 0;
    gComUart1.usRxCount = 0;
    gComUart1.usTxCount = 0;
    gComUart1.isSending = 0;
#endif

#if (BSP_UART2_EN == 1)
    gComUart2.huart = USART2;
    gComUart2.pTxBuf = gTxBufCom2;
    gComUart2.pRxBuf = gRxBufCom2;
    gComUart2.usTxBufSize = UART2_TX_BUF_SIZE;
    gComUart2.usRxBufSize = UART2_RX_BUF_SIZE;
    gComUart2.usTxWrite = 0;
    gComUart2.usTxRead = 0;
    gComUart2.usRxWrite = 0;
    gComUart2.usRxRead = 0;
    gComUart2.usRxCount = 0;
    gComUart2.usTxCount = 0;
    gComUart2.isSending = 0;
#endif

#if (BSP_UART3_EN == 1)
    gComUart3.huart = USART3;
    gComUart3.pTxBuf = gTxBufCom3;
    gComUart3.pRxBuf = gRxBufCom3;
    gComUart3.usTxBufSize = UART3_TX_BUF_SIZE;
    gComUart3.usRxBufSize = UART3_RX_BUF_SIZE;
    gComUart3.usTxWrite = 0;
    gComUart3.usTxRead = 0;
    gComUart3.usRxWrite = 0;
    gComUart3.usRxRead = 0;
    gComUart3.usRxCount = 0;
    gComUart3.usTxCount = 0;
    gComUart3.isSending = 0;
#endif

#if (BSP_UART4_EN == 1)
    gComUart4.huart = UART4;
    gComUart4.pTxBuf = gTxBufCom4;
    gComUart4.pRxBuf = gRxBufCom4;
    gComUart4.usTxBufSize = UART4_TX_BUF_SIZE;
    gComUart4.usRxBufSize = UART4_RX_BUF_SIZE;
    gComUart4.usTxWrite = 0;
    gComUart4.usTxRead = 0;
    gComUart4.usRxWrite = 0;
    gComUart4.usRxRead = 0;
    gComUart4.usRxCount = 0;
    gComUart4.usTxCount = 0;
    gComUart4.isSending = 0;
#endif

}

/**
 * @brief configure serial port hardware parameters
 * 
 */
static void InitHardUart(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

#if (BSP_UART1_EN == 1)
    /* Enable GPIO clock */
    UART1_TX_GPIO_CLK_ENABLE();
    UART1_RX_GPIO_CLK_ENABLE();

    /* Enable USART clock */
    UART1_CLK_ENABLE();

    /* Configure USART Tx as alternate function */
    GPIO_InitStruct.Pin = UART1_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = UART1_TX_AF;
    HAL_GPIO_Init(UART1_TX_GPIO_PORT, &GPIO_InitStruct);

    /* Configure USART Rx as alternate function */
    GPIO_InitStruct.Pin = UART1_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = UART1_RX_AF;
    HAL_GPIO_Init(UART1_RX_GPIO_PORT, &GPIO_InitStruct);
    
    /* Configure the NVIC for UART */   
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    bsp_SetUartParam(USART1, UART1_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

    SET_BIT(USART1->ICR, USART_ICR_TCCF);	/* clear TC sending completion flag */
    SET_BIT(USART1->RQR, USART_RQR_RXFRQ);  /* clear the RXNE receive flag */
    // USART_CR1_PEIE | USART_CR1_RXNEIE
    SET_BIT(USART1->CR1, USART_CR1_RXNEIE);	/* enable PE. RX reveive interrupt */
#endif

#if (BSP_UART2_EN == 1)
    // TODO
#endif

#if (BSP_UART3_EN == 1)
    // TODO
#endif

#if (BSP_UART4_EN == 1)
    // TODO
#endif

}

/**
 * @brief Set the hardware parameters of the serial port (baud rate, data bit, stop bit, start bit, parity, interrupt enable)
 * 
 * @param Instance USART handle
 * @param BaudRate baud rate
 * @param Parity parity, odd or even
 * @param Mode  sending and receiving modes
 */
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
    UART_HandleTypeDef UartHandle;

    /*##-1- configure serial port hardware parameters ##*/
    /*  UART Mode */
    /* configuration:
        - WordLength    = 8
        - StopBits  = 1 Bit
        - Parity    = param Parity
        - BaudRate  = param BaudRate
        - hardware flow control off (RTS and CTS signals) 
    */
        
    UartHandle.Instance        = Instance;

    UartHandle.Init.BaudRate   = BaudRate;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = Parity;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = Mode;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    UartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    UartHandle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    UartHandle.FifoMode = UART_FIFOMODE_DISABLE;
    UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief fill the data into the UART send buffer and start the TX interrupt
 * 
 * @param pUartHld USART handle
 * @param aucBuf data buffer pointer
 * @param usLen data send len
 */
static void UartSend(BSP_UART_STRU *pUartHld, uint8_t *aucBuf, uint16_t usLen)
{
	uint16_t index;

	for (index = 0; index < usLen; index++)
	{
		/* if the send buffer is full, wait for the buffer empty */
		while (1)
		{
			volatile uint16_t usCount;

			DISABLE_INT();
			usCount = pUartHld->usTxCount;
			ENABLE_INT();

			if (usCount < pUartHld->usTxBufSize)
			{
				break;
			}
			else if(usCount == pUartHld->usTxBufSize)/* data buffer is filled */
			{
				if((pUartHld->huart->CR1 & USART_CR1_TXEIE) == 0)
				{
					SET_BIT(pUartHld->huart->CR1, USART_CR1_TXEIE);
				}  
			}
		}

		/* fill the send buffer with new data */
		pUartHld->pTxBuf[pUartHld->usTxWrite] = aucBuf[index];

		DISABLE_INT();
		if (++pUartHld->usTxWrite >= pUartHld->usTxBufSize)
		{
			pUartHld->usTxWrite = 0;
		}
		pUartHld->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(pUartHld->huart->CR1, USART_CR1_TXEIE);	/* enable TX interrupt (buffer empty) */
}

/**
 * @brief reads 1 byte of data from the serial port receive buffer
 * 
 * @param pUartHld USART handle
 * @param pByte pointer to store read data
 * @return uint8_t 
 */
static uint8_t UartGetChar(BSP_UART_STRU *pUartHld, uint8_t *pByte)
{
    uint16_t usCount;

    DISABLE_INT();
    usCount = pUartHld->usRxCount;
    ENABLE_INT();

    if (usCount == 0)
    {
        return 0;
    }
    else
    {
        *pByte = pUartHld->pRxBuf[pUartHld->usRxRead];		/* get a byte form uart fifo */

        DISABLE_INT();
        if (++pUartHld->usRxRead >= pUartHld->usRxBufSize)
        {
            pUartHld->usRxRead = 0;
        }
        pUartHld->usRxCount--;
        ENABLE_INT();
        return 1;
    }
}

/**
 * @brief convert COM port numbers to BSP_UART Pointers
 * 
 * @param ucPort COM port number
 * @return BSP_UART_STRU* 
 */
BSP_UART_STRU *ComToUartHandler(COM_PORT_ENUM ucPort)
{
    if (ucPort == SERIAL_COM1)
    {
        #if (BSP_UART1_EN == 1)
            return &gComUart1;
        #else
            return NULL;
        #endif
    }
    else if (ucPort == SERIAL_COM2)
    {
        #if (BSP_UART2_EN == 1)
            return &gComUart2;
        #else
            return NULL;
        #endif
    }
    else if (ucPort == SERIAL_COM3)
    {
        #if (BSP_UART3_EN == 1)
            return &gComUart3;
        #else
            return NULL;
        #endif
    }
    else if (ucPort == SERIAL_COM4)
    {
        #if (BSP_UART4_EN == 1)
            return &gComUart4;
        #else
            return NULL;
        #endif
    }
    else
    {
        Error_Handler(__FILE__, __LINE__);
        return NULL;
    }
}

/**
 * @brief send a set of data to the serial port.
 *        The data is returned immediately after being placed in the send buffer 
 *        and is sent in the background by the interrupt service routine
 * 
 * @param ucPort COM Port number
 * @param aucBuf send data buffer
 * @param usLen send data length
 */
void comSendBuf(COM_PORT_ENUM ucPort, uint8_t *aucBuf, uint16_t usLen)
{
    BSP_UART_STRU *pUartHld = NULL;

    pUartHld = ComToUartHandler(ucPort);
    if (pUartHld == NULL)
    {
        return;
    }

    UartSend(pUartHld, aucBuf, usLen);
}

/**
 * @brief send a byte data to the serial port
 * 
 * @param ucPort COM Port number
 * @param ucByte data byte
 */
void comSendChar(COM_PORT_ENUM ucPort, uint8_t ucByte)
{
    comSendBuf(ucPort, &ucByte, 1);
}


/**
 * @brief read 1 byte from the receive buffer, non-blocking. Returns immediately with or without data
 * 
 * @param ucPort COM Port number
 * @param pByte pointer to store read data
 * @return uint8_t 
 */
uint8_t comGetChar(COM_PORT_ENUM ucPort, uint8_t *pByte)
{
    BSP_UART_STRU *pUartHld = NULL;

    pUartHld = ComToUartHandler(ucPort);
    if (pUartHld == NULL)
    {
        return 0;
    }

    return UartGetChar(pUartHld, pByte);
}

/**
 * @brief reset the serial port send buffer
 * 
 * @param ucPort COM Port number
 */
void comClearTxFifo(COM_PORT_ENUM ucPort)
{
    BSP_UART_STRU *pUartHld = NULL;

    pUartHld = ComToUartHandler(ucPort);
    if (pUartHld == NULL)
    {
        return;
    }

    pUartHld->usTxWrite = 0;
    pUartHld->usTxRead = 0;
    pUartHld->usTxCount = 0;
}

/**
 * @brief reset the serial port receive buffer
 * 
 * @param ucPort COM Port number
 */
void comClearRxFifo(COM_PORT_ENUM ucPort)
{
    BSP_UART_STRU *pUartHld = NULL;

    pUartHld = ComToUartHandler(ucPort);
    if (pUartHld == NULL)
    {
        return;
    }

    pUartHld->usRxWrite = 0;
    pUartHld->usRxRead = 0;
    pUartHld->usRxCount = 0;
}


/*======================================= [UARTx IRQ HANDLER] =======================================*/
/**
 * @brief for interrupt service program call, general serial interrupt handling function
 * 
 * @param pUartHld 
 */
static void UartCommIRQHandler(BSP_UART_STRU *pUartHld)
{
    if (pUartHld->huart == NULL)
    {
        return ;
    }

    uint32_t isrflags   = READ_REG(pUartHld->huart->ISR);
    uint32_t cr1its     = READ_REG(pUartHld->huart->CR1);
    uint32_t cr3its     = READ_REG(pUartHld->huart->CR3);

    /* handling receive interrupts  */
    if ((isrflags & USART_ISR_RXNE_RXFNE) != RESET)
    {
        /* read data from the receive data register and store it to the receive FIFO buffer */
        uint8_t ch;

        ch = READ_REG(pUartHld->huart->RDR);
        pUartHld->pRxBuf[pUartHld->usRxWrite] = ch;
        if (++pUartHld->usRxWrite >= pUartHld->usRxBufSize)
        {
            pUartHld->usRxWrite = 0;
        }
        if (pUartHld->usRxCount < pUartHld->usRxBufSize)
        {
            pUartHld->usRxCount++;
        }
    }

    /* handling transmit buffer empty interrupt */
    if ( ((isrflags & USART_ISR_TXE_TXFNF) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
    {
        //if (pUartHld->usTxRead == pUartHld->usTxWrite)
        if (pUartHld->usTxCount == 0)
        {
            /* when the TX buffer data has been taken out, disable TX empty interrupt (note:the last byte not really be transmit end) */
            //USART_ITConfig(pUartHld->huart, USART_IT_TXE, DISABLE);
            CLEAR_BIT(pUartHld->huart->CR1, USART_CR1_TXEIE);

            /* enable data transmit complete interrupt */
            //USART_ITConfig(pUartHld->huart, USART_IT_TC, ENABLE);
            SET_BIT(pUartHld->huart->CR1, USART_CR1_TCIE);
        }
        else
        {
            pUartHld->isSending = 1;
            
            /* fetch a byte data from send FIFO buffer, write into serial send buffer register */
            //USART_SendData(pUartHld->huart, pUartHld->pTxBuf[pUartHld->usTxRead]);
            pUartHld->huart->TDR = pUartHld->pTxBuf[pUartHld->usTxRead];
            if (++pUartHld->usTxRead >= pUartHld->usTxBufSize)
            {
                pUartHld->usTxRead = 0;
            }
            pUartHld->usTxCount--;
        }

    }

    /* handling data sending complete interrupt */
    if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
    {
        //if (pUartHld->usTxRead == pUartHld->usTxWrite)
        if (pUartHld->usTxCount == 0)
        {
            /* data has been transmit complete end, disable TC interrupt */
            //USART_ITConfig(pUartHld->huart, USART_IT_TC, DISABLE);
            CLEAR_BIT(pUartHld->huart->CR1, USART_CR1_TCIE);
            
            pUartHld->isSending = 0;
        }
        else
        {
            /* exception */

            /* if send FIFO buffer not transmit complete, fetch a byte to data TX buffer register */
            //USART_SendData(pUartHld->huart, pUartHld->pTxBuf[pUartHld->usTxRead]);
            pUartHld->huart->TDR = pUartHld->pTxBuf[pUartHld->usTxRead];
            if (++pUartHld->usTxRead >= pUartHld->usTxBufSize)
            {
                pUartHld->usTxRead = 0;
            }
            pUartHld->usTxCount--;
        }
    }

    /* clear interrupt flag */
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_PEF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_FEF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_NEF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_OREF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_IDLEF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_TCF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_LBDF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_CTSF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_CMF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_WUF);
    SET_BIT(pUartHld->huart->ICR, UART_CLEAR_TXFECF);
	
//	*            @arg UART_CLEAR_PEF: Parity Error Clear Flag
//  *            @arg UART_CLEAR_FEF: Framing Error Clear Flag
//  *            @arg UART_CLEAR_NEF: Noise detected Clear Flag
//  *            @arg UART_CLEAR_OREF: OverRun Error Clear Flag
//  *            @arg UART_CLEAR_IDLEF: IDLE line detected Clear Flag
//  *            @arg UART_CLEAR_TCF: Transmission Complete Clear Flag
//  *            @arg UART_CLEAR_LBDF: LIN Break Detection Clear Flag
//  *            @arg UART_CLEAR_CTSF: CTS Interrupt Clear Flag
//  *            @arg UART_CLEAR_RTOF: Receiver Time Out Clear Flag
//  *            @arg UART_CLEAR_CMF: Character Match Clear Flag
//  *            @arg.UART_CLEAR_WUF:  Wake Up from stop mode Clear Flag
//  *            @arg UART_CLEAR_TXFECF: TXFIFO empty Clear Flag	
}


#if (BSP_UART1_EN == 1)
void USART1_IRQHandler(void)
{
    UartCommIRQHandler(&gComUart1);
}
#endif

#if (BSP_UART2_EN == 1)
void USART2_IRQHandler(void)
{
    UartCommIRQHandler(&gComUart2);
}
#endif

#if (BSP_UART3_EN == 1)
void USART3_IRQHandler(void)
{
    UartCommIRQHandler(&gComUart3);
}
#endif

#if (BSP_UART4_EN == 1)
void UART4_IRQHandler(void)
{
    UartCommIRQHandler(&gComUart4);
}
#endif

/*======================================= [redefine the putc function] =======================================*/
// #ifdef __GNUC__
// int __io_putchar (int ch)
// #else
int fputc (int ch, FILE *f)
//#endif /* __GNUC__ */
{
    uint32_t tickstart;

    tickstart = HAL_GetTick();

    USART1->TDR = ch;

    /* wait transmit complete */
    while((USART1->ISR & USART_ISR_TC) == 0)
    {
        if ((HAL_GetTick() - tickstart) > UART_POLL_TIMEOUT)
        {
            Error_Handler(__FILE__, __LINE__);
            break;
        }
    }

    return ch;
}
