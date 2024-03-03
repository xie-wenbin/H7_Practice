/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static void PrintfLogo(void);
/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
    uint8_t ucKeyCode;
    uint8_t read;
    const char buf1[] = "received the serial command 1\r\n";
    const char buf2[] = "received the serial command 2\r\n";
    const char buf3[] = "received the serial command 3\r\n";
    const char buf4[] = "received the serial command 4\r\n";

    bsp_Init();
    PrintfLogo();

    bsp_StartAutoTimer(0, 500);
    
    bsp_SetTIMforInt(TIM6, 20, 2, 0); /*TIM6 20Hz, 0.05s intr cycle*/

    bsp_SetTIMOutPWM(GPIOK, GPIO_PIN_1, TIM1, 1, 1000, 8000); /* TIM1 CH1, 1KHz, 80% duty cycle */

    /* Infinite loop */
    while (1)
    {
        if (bsp_CheckTimer(0))
        {
            bsp_LedToggle(BSP_LED_BLUE);
        }

        if (comGetChar(SERIAL_COM1, &read))
        {
            switch (read)
            {
                case '1':
                    comSendBuf(SERIAL_COM1, (uint8_t *)buf1, strlen(buf1));
                    break;

                case '2':
                    comSendBuf(SERIAL_COM1, (uint8_t *)buf2, strlen(buf2));
                    break;

                case '3':
                    comSendBuf(SERIAL_COM1, (uint8_t *)buf3, strlen(buf3));
                    break;

                case '4':
                    comSendBuf(SERIAL_COM1, (uint8_t *)buf4, strlen(buf4));
                    break;	
                
                default:
                    break;
            }
        }

        ucKeyCode = bsp_GetKey();
        if (ucKeyCode != KEY_NONE)
        {
            switch (ucKeyCode)
            {
                case KEY_1_DOWN:
                    printf("K1 DOWM\r\n");
                    //bsp_LedOn(BSP_LED_GREEN);
                    bsp_SetTimPwmDuty(2000);
                    break;
                case KEY_1_LONG:
                    printf("K1 LONG\r\n");
                    bsp_LedOff(BSP_LED_GREEN);
                    break;
                case KEY_2_DOWN:
                    printf("K2 DOWM\r\n");
                    //bsp_LedOn(BSP_LED_ORANGE);
                    bsp_SetTimPwmDuty(4000);
                    break;
                case KEY_2_UP:
                    printf("K2 UP\r\n");
                    bsp_LedOff(BSP_LED_ORANGE);
                    break;
                case KEY_3_DOWN:
                    printf("K3 DOWM\r\n");
                    //bsp_LedOn(BSP_LED_RED);
                    bsp_SetTimPwmDuty(6000);
                    break;
                case KEY_3_UP:
                    printf("K3 UP\r\n");
                    bsp_LedOff(BSP_LED_RED);
                    break;
                case KEY_4_DOWN:
                    printf("K4 DOWM\r\n");
                    //bsp_LedOn(BSP_LED_BLUE);
                    bsp_SetTimPwmDuty(8000);
                    break;
                case KEY_4_UP:
                    printf("K4 UP\r\n");
                    bsp_LedOff(BSP_LED_BLUE);
                    break;
            }
        }
    }
}


/**
  * @brief  This function handles TIM interrupt request.
  * @param  None
  * @retval None
  */
void TIM6_DAC_IRQHandler(void)
{
	if((TIM6->SR & TIM_FLAG_UPDATE) != RESET)
	{
		TIM6->SR = ~ TIM_FLAG_UPDATE;
		
		bsp_LedToggle(BSP_LED_RED);
	}
}

static void PrintfLogo(void)
{
    printf("*************************************************************\n\r");

    /* detect CPU ID */
    {
        uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
        
        CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
        CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
        CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

        printf("\r\nCPU : STM32H747XIH6, BGA240, frequency: %dMHz\r\n", SystemCoreClock / 1000000);
        printf("UID = 0x%08X 0x%08X 0x%08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
    }

    printf("\n\r");
    printf("*************************************************************\n\r");
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
        ;
    }
}
#endif

/**
 * @}
 */

/**
 * @}
 */
