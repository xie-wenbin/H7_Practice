#include "bsp.h"
#include "utils.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUFFER_SIZE         ((uint32_t)0x0200)
#define WRITE_READ_ADDR     ((uint32_t)0x0050)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t qspi_aTxBuffer[BUFFER_SIZE];
uint8_t qspi_aRxBuffer[BUFFER_SIZE];
/* Private function prototypes -----------------------------------------------*/


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

