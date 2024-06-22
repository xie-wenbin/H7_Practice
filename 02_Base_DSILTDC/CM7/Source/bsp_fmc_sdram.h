/*
*********************************************************************************************************
*
*   [module   ] : SDRAM (For STM32H7)
*   [file name] : bsp_fmc_sdram.h
*   [version  ] : V1.0
*   [brief    ] : fmc sdram module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_FMC_SDRAM_H_
#define BSP_FMC_SDRAM_H_

#include "stm32h7xx_hal.h"

#define SDRAM_INSTANCES_NUMBER      1U
#define SDRAM_DEVICE01_INSTANCE     0U
#define SDRAM_DEVICE01_ADDR         0xD0000000U
#define SDRAM_DEVICE01_SIZE         0x02000000U  //32M

/* MDMA definitions for SDRAM DMA transfer */
#define SDRAM_MDMAx_CLK_ENABLE             __HAL_RCC_MDMA_CLK_ENABLE
#define SDRAM_MDMAx_CLK_DISABLE            __HAL_RCC_MDMA_CLK_DISABLE
#define SDRAM_MDMAx_CHANNEL                MDMA_Channel0
#define SDRAM_MDMAx_IRQn                   MDMA_IRQn
#define SDRAM_MDMA_IRQHandler              MDMA_IRQHandler

#define LCD_FRAME_LAYER_SIZE        0x200000U //(2 * 1024 * 1024) //2M
#define LCD_LAYER_NBR               2
#define LCD_FRAME_BUFFER1           SDRAM_DEVICE01_ADDR
#define LCD_FRAME_BUFFER2           (LCD_FRAME_BUFFER1 + LCD_FRAME_LAYER_SIZE)

#define SDRAM_APP_BUF		(SDRAM_DEVICE01_ADDR + LCD_FRAME_LAYER_SIZE * LCD_LAYER_NBR)
#define SDRAM_APP_SIZE		(SDRAM_DEVICE01_SIZE - LCD_FRAME_LAYER_SIZE * LCD_LAYER_NBR)

void bsp_InitExtSDRAM(uint32_t Instance);

#endif /* BSP_FMC_SDRAM_H_ */
