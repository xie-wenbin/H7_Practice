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

void bsp_InitExternalSdram(uint32_t Instance);

#endif /* BSP_FMC_SDRAM_H_ */
