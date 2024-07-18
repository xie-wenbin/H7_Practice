/*
*********************************************************************************************************
*
*   [module   ] : SDIO SD(For STM32H7)
*   [file name] : bsp_sdio_sd.h
*   [version  ] : V1.0
*   [brief    ] : SDIO SD module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_SDIO_SD_H_
#define BSP_SDIO_SD_H_

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery_errno.h"

#define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(SD_HandleTypeDef *);
  void (* pMspDeInitCb)(SD_HandleTypeDef *);
}BSP_SD_Cb_t;
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1) */

/**
  * @brief SD interface bus width selection
  *        4-bit wide data bus can be disabled to avoid conflict with
  *        camera pins (PC9 and PC11) on the Disco board.
  *        1-bit (SDMMC_D0) databus width will be used instead.
  */
#ifndef USE_SD_BUS_WIDE_4B
#define USE_SD_BUS_WIDE_4B       1U
#endif

/**
  * @brief  SD transfer state definition
  */
#define   SD_TRANSFER_OK         0U
#define   SD_TRANSFER_BUSY       1U

/**
  * @brief SD-detect signal
  */
#define SD_PRESENT               1UL
#define SD_NOT_PRESENT           0UL

/**
  * @brief SD-detect signal
  */
#define SD_DETECT_PIN                        GPIO_PIN_8
#define SD_DETECT_GPIO_PORT                  GPIOI
#define SD_DETECT_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOI_CLK_ENABLE()
#define SD_DETECT_GPIO_CLK_DISABLE()         __HAL_RCC_GPIOI_CLK_DISABLE()
#define SD_DETECT_EXTI_IRQn                  EXTI9_5_IRQn

#define SD_DETECT_EXTI_LINE                  EXTI_LINE_8
#define SD_DetectIRQHandler()                HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8)

extern SD_HandleTypeDef    hsd_sdmmc;
extern EXTI_HandleTypeDef  hsd_exti;



int32_t BSP_SD_Init(void);
int32_t BSP_SD_DeInit(void);
int32_t BSP_SD_RegisterDetectIntr(void);

int32_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr, uint32_t Timeout);
int32_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr, uint32_t Timeout);
int32_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr);
int32_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr);
int32_t BSP_SD_ReadBlocks_IT(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr);
int32_t BSP_SD_WriteBlocks_IT(uint32_t *pData, uint32_t BlockAddr, uint32_t BlocksNbr);
int32_t BSP_SD_Erase(uint32_t BlockAddr, uint32_t BlocksNbr);

int32_t BSP_SD_GetCardState(void);
int32_t BSP_SD_GetCardInfo(BSP_SD_CardInfo *CardInfo);
int32_t BSP_SD_IsDetected(void);

/* These functions can be modified in case the current settings (e.g. DMA stream or IT)
   need to be changed for specific application needs */
__weak void BSP_SD_AbortCallback(void);
__weak void BSP_SD_WriteCpltCallback(void);
__weak void BSP_SD_ReadCpltCallback(void);
__weak void BSP_SD_ErrorCallback(void);
__weak void BSP_SD_DetectCallback(uint32_t Status);
__weak void BSP_SD_DriveTransciver_1_8V_Callback(FlagStatus status);


/* interrupt handler functions */
void BSP_SD_DETECT_IRQHandler(void);
void BSP_SD_IRQHandler(void);

#endif /* BSP_SDIO_SD_H_ */
