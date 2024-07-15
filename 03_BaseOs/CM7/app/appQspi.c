#include "bsp.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUFFER_SIZE         ((uint32_t)0x0200)
#define WRITE_READ_ADDR     ((uint32_t)0x0050)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t qspi_aTxBuffer[BUFFER_SIZE];
uint8_t qspi_aRxBuffer[BUFFER_SIZE];
/* Private function prototypes -----------------------------------------------*/
static void     Fill_Buffer (uint8_t *pBuffer, uint32_t uwBufferLength, uint32_t uwOffset);
static uint8_t  Buffercmp   (uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);


void appQspi_demo(void)
{
    static uint32_t uwOffset = 0x0F;
        /* QSPI info structure */
    BSP_QSPI_Info_t pQSPI_Info;

    /* Read the QSPI memory info */
    BSP_QSPI_GetChipInfo(&pQSPI_Info);

    /* Test the correctness */
    if((pQSPI_Info.FlashSize != 0x8000000) || (pQSPI_Info.EraseSectorSize != 0x2000)  ||
       (pQSPI_Info.ProgPageSize != 0x100)  || (pQSPI_Info.EraseSectorsNumber != 0x4000) ||
         (pQSPI_Info.ProgPagesNumber != 0x80000))
    {
        printf("QSPI GET INFO : FAILED.\r\n");
        printf("QSPI Test Aborted.\r\n");
    }
    else
    {
        printf("QSPI GET INFO : OK.   \r\n");

        /*##-3- Erase QSPI memory ################################################*/
        if (BSP_QSPI_EraseBlock(WRITE_READ_ADDR, BSP_QSPI_ERASE_8K) != BSP_ERROR_NONE)
        {
            printf("QSPI ERASE : FAILED.\r\n");
            printf("QSPI Test Aborted.\r\n");
        }
        else
        {
            printf("QSPI ERASE : OK.   \r\n");

            /*##-4- QSPI memory read/write access  #################################*/
            /* Fill the buffer to write */
            Fill_Buffer(qspi_aTxBuffer, BUFFER_SIZE, uwOffset);

            // uwOffset += 0x20;

            /* Write data to the QSPI memory */
            if (BSP_QSPI_SyncWrite(qspi_aTxBuffer, WRITE_READ_ADDR, BUFFER_SIZE) != BSP_ERROR_NONE)
            {
                printf("QSPI WRITE : FAILED.\r\n");
                printf("QSPI Test Aborted.\r\n");
            }
            else
            {
                printf("QSPI WRITE : OK.     \r\n");

                /* Read back data from the QSPI memory */
                if (BSP_QSPI_Read(qspi_aRxBuffer, WRITE_READ_ADDR, BUFFER_SIZE) != BSP_ERROR_NONE)
                {
                    printf("QSPI READ : FAILED.\r\n");
                    printf("QSPI Test Aborted.\r\n");
                }
                else
                {
                    printf("QSPI READ :  OK.    \r\n");

                    /*##-5- Checking data integrity ############################################*/
                    if (Buffercmp(qspi_aRxBuffer, qspi_aTxBuffer, BUFFER_SIZE) > 0)
                    {
                        printf("QSPI COMPARE : FAILED.\r\n");
                        printf("QSPI Test Aborted.\r\n");
                    }
                    else
                    {
                        printf("QSPI COMPARE : OK.     \r\n");
                        
                    }
                    /*##-6-Memory Mapped Mode ###############################################*/
                    if (BSP_QSPI_EnableMemoryMappedMode() != BSP_ERROR_NONE)
                    {
                        printf("QSPI Memory Mapped Mode : FAILED.     \r\n");
                        printf("QSPI Test Aborted.\r\n");
                    }
                    else
                    {
                        printf("QSPI Memory Mapped Mode : OK.     \r\n");
                        printf("QSPI Test : OK.     \r\n");
                    }
                }
            }
        }
    }
}



/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the buffer to fill
  * @param  uwBufferLenght: size of the buffer to fill
  * @param  uwOffset: first value to fill on the buffer
  * @retval None
  */
static void Fill_Buffer(uint8_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
  uint32_t tmpIndex = 0;

  /* Put in global buffer different values */
  for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++ )
  {
    pBuffer[tmpIndex] = tmpIndex + uwOffset;
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 1: pBuffer identical to pBuffer1
  *         0: pBuffer differs from pBuffer1
  */
static uint8_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
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

