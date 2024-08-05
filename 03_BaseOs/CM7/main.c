/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "appGUI.h"
#include "appSd.h"

/* Public variables ---------------------------------------------------------*/
uint8_t gucWukpKeyState = 0xFF;
uint8_t gucJoyUKeyState = 0xFF;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* 任务的属性设置 */
const osThreadAttr_t ThreadRoot_Attr =
{
    /* 未使用 */
    //	.cb_mem = &worker_thread_tcb_1,
    //	.cb_size = sizeof(worker_thread_tcb_1),
    //	.stack_mem = &worker_thread_stk_1[0],
    //	.stack_size = sizeof(worker_thread_stk_1),
    //	.priority = osPriorityAboveNormal,
    //	.tz_module = 0

    .name = "osRootThread",
    .attr_bits = osThreadDetached,
    .priority = osPriorityHigh,
    .stack_size = 2048,
};

const osThreadAttr_t ThreadLED_Attr =
{
    .name = "osLEDThread",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal2,
    .stack_size = 512,
};

const osThreadAttr_t ThreadUserIF_Attr =
{
    .name = "osThreadUserIF",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal1,
    .stack_size = 1024,
};

const osThreadAttr_t ThreadGUI_Attr =
{
    .name = "osThreadGUIX",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 2048,
};

const osThreadAttr_t ThreadPeriph_Attr =
{
    .name = "osThreadPeriph",
    .attr_bits = osThreadDetached,
    .priority = osPriorityNormal,
    .stack_size = 4096,
};

/* 任务句柄 */
osThreadId_t ThreadIdTaskGUI = NULL;
osThreadId_t ThreadIdTaskUserIF = NULL;
osThreadId_t ThreadIdTaskLED = NULL;
osThreadId_t ThreadIdRoot = NULL;

osThreadId_t ThreadIdPeriph = NULL;

/* Private function prototypes -----------------------------------------------*/
void AppTaskRoot(void *argument);
void AppTaskUserIF(void *argument);
void AppTaskLED(void *argument);
void AppTaskPeripheral(void *argument);

static void AppTaskCreate (void);
static void PrintfLogo(void);


/* Private functions ---------------------------------------------------------*/
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
        printf("LTDC DSI Example\n\r");
    }

    printf("\n\r");
    printf("*************************************************************\n\r");
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
    /* HAL库，MPU，Cache，时钟等系统初始化 */
    System_Init();
    bsp_Init();
    PrintfLogo();

    /* 内核开启前关闭HAL的时间基准 */
    HAL_SuspendTick();

    /* 内核初始化 */
    osKernelInitialize();

    /* 创建启动任务 */
    ThreadIdRoot = osThreadNew(AppTaskRoot, NULL, &ThreadRoot_Attr);  

    /* 开启多任务 */
    osKernelStart();

    while(1);
}

extern void appQspi_demo(void);

/*
*********************************************************************************************************
*	函 数 名: AppTaskPeripheral
*	功能说明: 外设测试任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: osPriorityNormal  (数值越小优先级越低，这个跟uCOS相反) 
*********************************************************************************************************
*/
void AppTaskPeripheral(void *argument)
{
    while (1)
    {
        if (1 == gucJoyUKeyState)
        {
            gucJoyUKeyState = 0;
            appQspi_demo();
            
        }
        else if (2 == gucJoyUKeyState)
        {
        }
        osDelay(5);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUI
*	功能说明: GUI应用任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: osPriorityNormal  (数值越小优先级越低，这个跟uCOS相反) 
*********************************************************************************************************
*/
void AppTaskGUI(void *argument)
{
    appGUI_InitializeDisplay();

    while (1)
    {
        // appGUI_MainProc();
        osDelay(5);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: 按键消息处理		
*	形    参: 无
*	返 回 值: 无
*   优 先 级: osPriorityNormal1
*********************************************************************************************************
*/
void AppTaskUserIF(void *argument)
{
    uint8_t ucKeyCode;
    uint8_t cmd;

    while (1)
    {
        ucKeyCode = bsp_GetKey();

        if (ucKeyCode != KEY_NONE)
        {
            switch (ucKeyCode)
            {
                case KEY_1_DOWN: /*WUKP*/
                    {
                        if (0 == gucWukpKeyState)
                        {
                            gucWukpKeyState = 1;
                        }
                        else
                        {
                            gucWukpKeyState = 0;
                        }
                        printf("K1 DOWM, gucWukpKeyState: %d\r\n", gucWukpKeyState);
                    }
                    break;
                case KEY_2_DOWN:  /*JOY U*/
                    {
                        if (0 == gucJoyUKeyState)
                        {
                            gucJoyUKeyState = 1;
                        }
                        else if (1 == gucJoyUKeyState)
                        {
                            gucJoyUKeyState = 0;
                        }
                        else
                        {
                            gucJoyUKeyState = 0;
                        }
                        printf("K2 DOWM, gucWukpKeyState: %d\r\n", gucJoyUKeyState);
                    }
                    break;
                case KEY_3_DOWN: /*JOY D*/
                    printf("K3 DOWM\r\n");
                    break;
                case KEY_4_DOWN: /*JOY L*/
                    osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SHOW_PREV_PIC);
                    printf("K4 DOWM\r\n");
                    break;
                case KEY_5_DOWN: /*JOY R*/
                    osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SHOW_NEXT_PIC);
                    printf("K5 DOWM\r\n");
                    break;
                case KEY_6_DOWN: /*JOY OK*/
                    osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SHOW_FRT_PIC);
                    printf("OK DOWM\r\n");
                    break;
                /* 其他的键值不处理 */
                default:
                    break;
            }
        }

        if (comGetChar(SERIAL_COM1, &cmd)) /* 从串口读入一个字符(非阻塞方式) */
        {
            printf("\r\n");
            switch (cmd)
            {
            case '1':
                printf("【1 - ViewRootDir】\r\n");
                osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SHOW_ROOTDIR);
                break;

            case '2':
                printf("【2 - CreateNewFile】\r\n");
                osEventFlagsSet(gEventId_sdapp, APPSD_EVT_NEWFILE);
                break;

            case '3':
                printf("【3 - ReadFileData】\r\n");
                osEventFlagsSet(gEventId_sdapp, APPSD_EVT_READFILE);
                break;

            case '4':
                printf("【4 - CreateDir】\r\n");
                osEventFlagsSet(gEventId_sdapp, APPSD_EVT_NEWDIR);
                break;

            case '5':
                printf("【5 - DeleteDirFile】\r\n");
                osEventFlagsSet(gEventId_sdapp, APPSD_EVT_DELETEALL);
                break;

            case '6':
                printf("【6 - TestSpeed】\r\n");
                osEventFlagsSet(gEventId_sdapp, APPSD_EVT_SPEED_TEST);
                break;

            default:
                break;
            }
        }

        osDelay(20);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskLED
*	功能说明: LED闪烁。
*	形    参: 无
*	返 回 值: 无
*   优 先 级: osPriorityNormal2
*********************************************************************************************************
*/
void AppTaskLED(void *argument)
{
    const uint16_t usFrequency = 1000; /* 延迟周期 */
    uint32_t tick;

    /* 获取当前时间 */
    tick = osKernelGetTickCount();

    while (1)
    {
        bsp_LedToggle(BSP_LED_GREEN);
        /* 相对延迟 */
        tick += usFrequency;
        osDelayUntil(tick);
    }
}


/*
*********************************************************************************************************
*	函 数 名: AppTaskRoot
*	功能说明: 启动任务，这里用作BSP驱动包处理。
*	形    参: 无
*	返 回 值: 无
*   优 先 级: osPriorityNormal6  
*********************************************************************************************************
*/
void AppTaskRoot(void *argument)
{
    const uint16_t usFrequency = 1; /* 延迟周期 */
    uint32_t tick;

    /* 初始化外设 */
    HAL_ResumeTick();

    /* 创建任务 */
    AppTaskCreate();

    /* 获取当前时间 */
    tick = osKernelGetTickCount();

    while (1)
    {
        /* 需要周期性处理的程序，对应裸机工程调用的SysTick_ISR */
        bsp_ProPer1ms();

        /* 相对延迟 */
        tick += usFrequency;
        osDelayUntil(tick);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	ThreadIdTaskLED = osThreadNew(AppTaskLED, NULL, &ThreadLED_Attr);  
	ThreadIdTaskUserIF = osThreadNew(AppTaskUserIF, NULL, &ThreadUserIF_Attr);
	ThreadIdTaskGUI = osThreadNew(AppTaskGUI, NULL, &ThreadGUI_Attr);
    ThreadIdPeriph = osThreadNew(AppTaskPeripheral, NULL, &ThreadPeriph_Attr); 

    appSd_CreateTask();
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
