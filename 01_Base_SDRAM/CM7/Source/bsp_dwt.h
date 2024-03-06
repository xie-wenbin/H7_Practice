/*
*********************************************************************************************************
*
*   [module   ] : SDRAM (For STM32H7)
*   [file name] : bsp_dwt.h
*   [version  ] : V1.0
*   [brief    ] : cortex m7 data watchpoint and trace uint
*                 In CM3, CM4, CM7 can have three trace sources: ETM, ITM and DWT, 
*                 this driver mainly implements the DWT clock cycle (CYCCNT) counting function
*                 it can be very convenient to calculate the number of clock cycles of program execution.
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_DWT_H_
#define BSP_DWT_H_

#include "stm32h7xx_hal.h"

#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

void bsp_InitDWT(void);

#endif /* BSP_DWT_H_ */
