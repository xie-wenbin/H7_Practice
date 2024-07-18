#include "bsp.h"
#include "bsp_sdio_sd.h"

/* Private define ------------------------------------------------------------*/
#define SD_DMA_MODE      2U
#define SD_IT_MODE       1U
#define SD_POLLING_MODE  0U


#ifndef SD_WRITE_TIMEOUT
#define SD_WRITE_TIMEOUT         100U
#endif

#ifndef SD_READ_TIMEOUT
#define SD_READ_TIMEOUT          100U
#endif

#define BLOCK_START_ADDR         0     /* Block start address      */
#define NUM_OF_BLOCKS            5     /* Total number of blocks   */
#define BUFFER_WORDS_SIZE        ((BLOCKSIZE * NUM_OF_BLOCKS) >> 2) /* Total data size in bytes */

/* Private variables ---------------------------------------------------------*/
// ALIGN_32BYTES (uint32_t aTxBuffer[BUFFER_WORDS_SIZE]);
// ALIGN_32BYTES (uint32_t aRxBuffer[BUFFER_WORDS_SIZE]);
// ALIGN_32BYTES (uint32_t aRxBuffer2[BUFFER_WORDS_SIZE]);

ALIGN_32BYTES (uint32_t aTxBuffer[BUFFER_WORDS_SIZE]) __attribute__((section (".RAM_D1")));
ALIGN_32BYTES (uint32_t aRxBuffer[BUFFER_WORDS_SIZE]) __attribute__((section (".RAM_D1")));
ALIGN_32BYTES (uint32_t aRxBuffer2[BUFFER_WORDS_SIZE]) __attribute__((section (".RAM_D1")));

__IO uint32_t SD1WriteStatus = 0, SD1ReadStatus = 0, SD2WriteStatus = 0, SD2ReadStatus = 0;
__IO uint32_t SdmmcTest;

static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset);
static uint8_t Buffercmp(uint32_t* pBuffer1, uint32_t* pBuffer2, uint16_t BufferLength);

/**
* @brief  SD Demo
* @param  Mode SD_DMA_MODE, SD_IT_MODE or SD_POLLING_MODE
* @retval None
*/
static void SD_demo(uint32_t Mode)
{
    uint8_t SD1_state = BSP_ERROR_NONE;
    BSP_SD_CardInfo cardInfo= {0};

    SdmmcTest = 0;

    SD1_state = BSP_SD_Init();
    BSP_SD_RegisterDetectIntr();

    /* Check if the SD card is plugged in the slot */
    if (BSP_SD_IsDetected() == SD_PRESENT)
    {
        printf(" SD1 Connected    \r\n");
    }
    else
    {
        printf("SD1 Not Connected\r\n");
    }

    if (SD1_state == BSP_ERROR_NONE)
    {
        printf("SD1 INITIALIZATION : OK.\r\n");

        SD1_state = BSP_SD_GetCardInfo(&cardInfo);

        if (SD1_state == BSP_ERROR_NONE)
        {
            printf("SD Info : blockNbr:%d, blockSize:%d, speed:%d, ", cardInfo.BlockNbr, cardInfo.BlockSize, cardInfo.CardSpeed);
            printf("CardType:%d, CardVersion:%d, LogBlockSize:%d.\r\n", cardInfo.CardType, cardInfo.CardVersion, cardInfo.LogBlockSize);
        }
        else
        {
            printf("SD1 CARDINFO : FAILED.\r\n");
        }


        SD1_state = BSP_SD_Erase(BLOCK_START_ADDR, BLOCK_START_ADDR + NUM_OF_BLOCKS);
        /* Wait until SD card is ready to use for new operation */
        while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
            ;

        if (SD1_state != BSP_ERROR_NONE)
        {
            printf("SD1 ERASE : FAILED.\r\n");
            printf("SD1 Test Aborted.\r\n");
        }
        else
        {
            printf("SD1 ERASE : OK.\r\n");

            /* Fill the buffer to write */
            Fill_Buffer(aTxBuffer, BUFFER_WORDS_SIZE, 0x22FF);
            if (Mode == SD_DMA_MODE)
            {
                printf("BSP_SD_WriteBlocks_DMA.\r\n");
                SD1_state = BSP_SD_WriteBlocks_DMA((uint32_t *)aTxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS);
                /* Wait for the write process is completed */
                while ((SD1WriteStatus == 0))
                    ;
                SD1WriteStatus = 0;
            }
            else if (Mode == SD_IT_MODE)
            {
                SdmmcTest = 1;
                printf("BSP_SD_WriteBlocks_IT.\r\n");
                SD1_state = BSP_SD_WriteBlocks_IT((uint32_t *)aTxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS);
                /* Wait for the write process is completed */
                while ((SD1WriteStatus == 0))
                    ;
                SD1WriteStatus = 0;
            }
            else
            {
                printf("BSP_SD_WriteBlocks.\r\n");
                SD1_state = BSP_SD_WriteBlocks((uint32_t *)aTxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS, SD_WRITE_TIMEOUT);
            }

            /* Wait until SD cards are ready to use for new operation */
            while ((BSP_SD_GetCardState() != SD_TRANSFER_OK))
                ;

            if (SD1_state != BSP_ERROR_NONE)
            {
                printf("SD1 WRITE : FAILED.\r\n");
                printf("SD1 Test Aborted.\r\n");
            }
            else
            {
                printf("SD1 WRITE : OK.\r\n");
                if (Mode == SD_DMA_MODE)
                {
                    printf("BSP_SD_ReadBlocks_DMA.\r\n");
                    SD1_state = BSP_SD_ReadBlocks_DMA((uint32_t *)aRxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS);

                    /* Wait for the read process is completed */
                    while (SD1ReadStatus == 0)
                        ;
                    SD1ReadStatus = 0;

                    /* Invalidate Data Cache to get the updated content of the SRAM*/
                    SCB_InvalidateDCache_by_Addr((uint32_t *)aRxBuffer, BUFFER_WORDS_SIZE * 4);
                }
                else if (Mode == SD_IT_MODE)
                {
                    printf("BSP_SD_ReadBlocks_IT.\r\n");
                    SD1_state = BSP_SD_ReadBlocks_IT((uint32_t *)aRxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS);
                    /* Wait for the read process is completed */
                    while (SD1ReadStatus == 0)
                        ;
                    SD1ReadStatus = 0;
                }
                else
                {
                    printf("BSP_SD_ReadBlocks.\r\n");
                    SD1_state = BSP_SD_ReadBlocks((uint32_t *)aRxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS, SD_READ_TIMEOUT);
                }
                /* Wait until SD card is ready to use for new operation */
                while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
                    ;

                if (SD1_state != BSP_ERROR_NONE)
                {
                    printf("SD1 READ : FAILED.\r\n");
                    printf("SD1 Test Aborted.\r\n");
                }
                else
                {
                    printf("SD1 READ : OK.\r\n");
                    if (Buffercmp(aTxBuffer, aRxBuffer, BUFFER_WORDS_SIZE) > 0)
                    {
                        printf("SD1 COMPARE : FAILED.\r\n");
                        printf("SD1 Test Aborted.\r\n");
                    }
                    else
                    {
                        printf("SD1 COMPARE : OK.\r\n");
                        printf("data:\r\n");
                        for (int iLoop = 0; iLoop < 512 / 4; iLoop++)
                        {
                            printf("%x ", aRxBuffer[iLoop]);
                        }
                        printf("\r\n");
                    }
                }
            }
        }
    }
    else
    {
        printf("SD INITIALIZATION : FAIL.\r\n");
        printf("SD Test Aborted.\r\n");
    }
}

/**
 * @brief  Fills buffer with user predefined data.
 * @param  pBuffer: pointer on the buffer to fill
 * @param  uwBufferLenght: size of the buffer to fill
 * @param  uwOffset: first value to fill on the buffer
 * @retval None
 */
static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
    uint32_t tmpIndex = 0;

    /* Put in global buffer different values */
    for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++)
    {
        pBuffer[tmpIndex] = tmpIndex + uwOffset;
    }

    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t *)pBuffer, uwBufferLenght * 4);
}

/**
 * @brief  Compares two buffers.
 * @param  pBuffer1, pBuffer2: buffers to be compared.
 * @param  BufferLength: buffer's length
 * @retval 1: pBuffer identical to pBuffer1
 *         0: pBuffer differs from pBuffer1
 */
static uint8_t Buffercmp(uint32_t *pBuffer1, uint32_t *pBuffer2, uint16_t BufferLength)
{
    while (BufferLength--)
    {
        if (*pBuffer1 != *pBuffer2)
        {
            return 1;
        }

        pBuffer1++;
        pBuffer2++;
    }
    return 0;
}

/**
* @brief Tx Transfer completed callbacks
* @param hsd: SD handle
* @retval None
*/
void BSP_SD_WriteCpltCallback(void)
{
    SD1WriteStatus = 1;
    printf(" SD1 SD1WriteStatus    \r\n");
}

/**
* @brief Rx Transfer completed callbacks
* @param hsd: SD handle
* @retval None
*/
void BSP_SD_ReadCpltCallback(void)
{
    SD1ReadStatus = 1;
    printf(" SD1 SD1ReadStatus    \r\n");
}

void BSP_SD_ErrorCallback(void)
{
    printf(" BSP_SD_ErrorCallback   0x%x\r\n", hsd_sdmmc.ErrorCode);
}


/**
* @brief  BSP SD Callback.
* @param  GPIO_Pin: Specifies the pin connected EXTI line
* @retval None.
*/
void BSP_SD_DetectCallback(uint32_t Status)
{
    /* Check if the SD card is plugged in the slot */
    if (Status == SD_PRESENT)
    {
        printf("DetectCallback: SD1 Connected    \r\n");
    }
    else
    {
        printf("DetectCallback: SD1 Not Connected    \r\n");
    }
}


void SD_DMA_demo(uint8_t mode)
{
    switch (mode)
    {
    case 0:
        SD_demo(0);
        break;
    case 1:
        SD_demo(1);
        break;
    case 2:
        SD_demo(2);
        break;
    
    default:
        break;
    }
    
}

