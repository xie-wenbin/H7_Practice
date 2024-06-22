/*
*********************************************************************************************************
*
*   [module   ] : SDRAM (For STM32H7)
*   [file name] : bsp_dwt.c
*   [version  ] : V1.0
*   [brief    ] : cortex m7 data watchpoint and trace uint
*                 In CM3, CM4, CM7 can have three trace sources: ETM, ITM and DWT, 
*                 this driver mainly implements the DWT clock cycle (CYCCNT) counting function
*                 it can be very convenient to calculate the number of clock cycles of program execution.
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#include "bsp_dwt.h"

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

void bsp_InitDWT(void)
{
    DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   
	DWT_CYCCNT      = (unsigned int)0u;
	DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}

