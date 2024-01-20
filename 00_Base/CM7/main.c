/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
    uint8_t ucKeyCode;

    bsp_Init();

    bsp_StartAutoTimer(0, 100);

    /* Infinite loop */
    while (1)
    {
        if (bsp_CheckTimer(0))
        {
            //bsp_LedToggle(BSP_LED_BLUE);
        }

        ucKeyCode = bsp_GetKey();
        if (ucKeyCode != KEY_NONE)
        {
            switch (ucKeyCode)
            {
                case KEY_1_DOWN:
                    printf("K1 DOWM\r\n");
                    bsp_LedOn(BSP_LED_GREEN);
                    break;
                case KEY_1_LONG:
                    printf("K1 LONG\r\n");
                    bsp_LedOff(BSP_LED_GREEN);
                    break;
                case KEY_2_DOWN:
                    printf("K2 DOWM\r\n");
                    bsp_LedOn(BSP_LED_ORANGE);
                    break;
                case KEY_2_UP:
                    printf("K2 UP\r\n");
                    bsp_LedOff(BSP_LED_ORANGE);
                    break;
                case KEY_3_DOWN:
                    printf("K3 DOWM\r\n");
                    bsp_LedOn(BSP_LED_RED);
                    break;
                case KEY_3_UP:
                    printf("K3 UP\r\n");
                    bsp_LedOff(BSP_LED_RED);
                    break;
                case KEY_4_DOWN:
                    printf("K4 DOWM\r\n");
                    bsp_LedOn(BSP_LED_BLUE);
                    break;
                case KEY_4_UP:
                    printf("K4 UP\r\n");
                    bsp_LedOff(BSP_LED_BLUE);
                    break;
            }
        }
    }
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
