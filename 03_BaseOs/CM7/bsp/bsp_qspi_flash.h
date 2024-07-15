/*
*********************************************************************************************************
*
*   [module   ] : QSPI FLASH(For STM32H7)
*   [file name] : bsp_qspi_flash.h
*   [version  ] : V1.0
*   [brief    ] : QSPI FLASH module
*
*	Copyright (C), 2023-2030
*
*********************************************************************************************************
*/
#ifndef BSP_QSPI_FLASH_H
#define BSP_QSPI_FLASH_H

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery_errno.h"
#include "mt25tl01g.h"

/* Exported types ------------------------------------------------------------*/
/** @defgroup STM32H747I_DISCO_QSPI_Exported_Types Exported Types
  * @{
  */
#define BSP_QSPI_Info_t                 MT25TL01G_Info_t
#define BSP_QSPI_Interface_e            MT25TL01G_Interface_enu
#define BSP_QSPI_Transfer_e             MT25TL01G_Transfer_enu
#define BSP_QSPI_DualFlash_e            MT25TL01G_DualFlash_enu
// #define BSP_QSPI_ODS_t                  MT25TL01G_ODS_t

typedef enum
{
  BSP_QSPI_ERASE_8K   =  MT25TL01G_ERASE_4K ,       /*!< 8K size Sector erase = 2 x 4K as Dual flash mode is used for this board   */
  BSP_QSPI_ERASE_64K  =  MT25TL01G_ERASE_32K ,      /*!< 64K size Sector erase = 2 x 32K as Dual flash mode is used for this board */
  BSP_QSPI_ERASE_128K =  MT25TL01G_ERASE_64K ,      /*!< 128K size Sector erase = 2 x 64K as Dual mode is used for this board      */
  BSP_QSPI_ERASE_CHIP =  MT25TL01G_ERASE_CHIP       /*!< Whole chip erase */

} BSP_QSPI_Erase_e;

typedef enum
{
  QSPI_ACCESS_NONE = 0,          /*!<  Instance not initialized,             */
  QSPI_ACCESS_INDIRECT,          /*!<  Instance use indirect mode access     */
  QSPI_ACCESS_MMP                /*!<  Instance use Memory Mapped Mode read  */
} BSP_QSPI_Access_e;

typedef struct
{
  BSP_QSPI_Access_e    IsInitialized;   /*!<  Instance access Flash method     */
  BSP_QSPI_Interface_e InterfaceMode;   /*!<  Flash Interface mode of Instance */
  BSP_QSPI_Transfer_e  TransferRate;    /*!<  Flash Transfer mode of Instance  */
  uint32_t             DualFlashMode;   /*!<  Flash dual mode                  */
  uint32_t             IsMspCallbacksValid;
} BSP_QSPI_Ctx_t;


typedef struct
{
    BSP_QSPI_Interface_e InterfaceMode;  /*!<  Current Flash Interface mode */
    BSP_QSPI_Transfer_e  TransferRate;   /*!<  Current Flash Transfer mode  */
    BSP_QSPI_DualFlash_e DualFlashMode;  /*!<  Dual Flash mode              */
} BSP_QSPI_Init_t;


/***************************************************************/
/* STM32H7 Hardware resource macro defines                     */
/***************************************************************/

/* Definition for QSPI Pins */
#define QSPI_CLK_PIN               GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT         GPIOB
/* Bank 1 */
#define QSPI_BK1_CS_PIN            GPIO_PIN_6
#define QSPI_BK1_CS_GPIO_PORT      GPIOG
#define QSPI_BK1_D0_PIN            GPIO_PIN_11
#define QSPI_BK1_D0_GPIO_PORT      GPIOD
#define QSPI_BK1_D1_PIN            GPIO_PIN_9
#define QSPI_BK1_D1_GPIO_PORT      GPIOF
#define QSPI_BK1_D2_PIN            GPIO_PIN_7
#define QSPI_BK1_D2_GPIO_PORT      GPIOF
#define QSPI_BK1_D3_PIN            GPIO_PIN_6
#define QSPI_BK1_D3_GPIO_PORT      GPIOF

/* Bank 2 */
#define QSPI_BK2_CS_PIN            GPIO_PIN_6
#define QSPI_BK2_CS_GPIO_PORT      GPIOG
#define QSPI_BK2_D0_PIN            GPIO_PIN_2
#define QSPI_BK2_D0_GPIO_PORT      GPIOH
#define QSPI_BK2_D1_PIN            GPIO_PIN_3
#define QSPI_BK2_D1_GPIO_PORT      GPIOH
#define QSPI_BK2_D2_PIN            GPIO_PIN_9
#define QSPI_BK2_D2_GPIO_PORT      GPIOG
#define QSPI_BK2_D3_PIN            GPIO_PIN_14
#define QSPI_BK2_D3_GPIO_PORT      GPIOG

/* Definition for QSPI clock resources */
#define QSPI_CLK_ENABLE()              __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()             __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK2_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPI_BK2_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOH_CLK_ENABLE()
#define QSPI_BK2_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOH_CLK_ENABLE()
#define QSPI_BK2_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPI_BK2_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOG_CLK_ENABLE()

#define QSPI_FORCE_RESET()         __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()       __HAL_RCC_QSPI_RELEASE_RESET()

/* QSPI Base Address */
#define QSPI_BASE_ADDRESS          0x90000000

/***************************************************************/
/* Flash Chirp macro defines                                   */
/***************************************************************/

/* MT25TL01G Micron memory */
/* Size of the flash */
#define QSPI_FLASH_SIZE            26     /* Address bus width to access whole memory space */
#define QSPI_PAGE_SIZE             256

/* Definition for QSPI modes */
#define BSP_QSPI_SPI_MODE            (BSP_QSPI_Interface_e)MT25TL01G_SPI_MODE      /* 1 Cmd Line, 1 Address Line and 1 Data Line    */
// #define BSP_QSPI_SPI_1I2O_MODE       (BSP_QSPI_Interface_e)MT25TL01G_SPI_1I2O_MODE /* 1 Cmd Line, 1 Address Line and 2 Data Lines   */
#define BSP_QSPI_SPI_2IO_MODE        (BSP_QSPI_Interface_e)MT25TL01G_SPI_2IO_MODE  /* 1 Cmd Line, 2 Address Lines and 2 Data Lines  */
// #define BSP_QSPI_SPI_1I4O_MODE       (BSP_QSPI_Interface_e)MT25TL01G_SPI_1I4O_MODE /* 1 Cmd Line, 1 Address Line and 4 Data Lines   */
#define BSP_QSPI_SPI_4IO_MODE        (BSP_QSPI_Interface_e)MT25TL01G_SPI_4IO_MODE  /* 1 Cmd Line, 4 Address Lines and 4 Data Lines  */
// #define BSP_QSPI_DPI_MODE            (BSP_QSPI_Interface_e)MT25TL01G_DPI_MODE      /* 2 Cmd Lines, 2 Address Lines and 2 Data Lines */
#define BSP_QSPI_QPI_MODE            (BSP_QSPI_Interface_e)MT25TL01G_QPI_MODE      /* 4 Cmd Lines, 4 Address Lines and 4 Data Lines */

/* Definition for QSPI transfer rates */
#define BSP_QSPI_STR_TRANSFER        (BSP_QSPI_Transfer_e)MT25TL01G_STR_TRANSFER /* Single Transfer Rate */
#define BSP_QSPI_DTR_TRANSFER        (BSP_QSPI_Transfer_e)MT25TL01G_DTR_TRANSFER /* Double Transfer Rate */

/* Definition for QSPI dual flash mode */
#define BSP_QSPI_DUALFLASH_DISABLE   (BSP_QSPI_DualFlash_e)MT25TL01G_DUALFLASH_DISABLE   /* Dual flash mode enabled  */
/* Definition for QSPI Flash ID */
#define BSP_QSPI_FLASH_ID            QSPI_FLASH_ID_1

/** @addtogroup Exported_Variables
  * @{
  */
extern QSPI_HandleTypeDef hqspi;



/* Exported functions --------------------------------------------------------*/
/** @addtogroup Exported_Functions
  * @{
  */

void BSP_QSPI_Init(BSP_QSPI_Init_t *Init);
int32_t BSP_QSPI_DeInit(void);
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
int32_t BSP_QSPI_RegisterMspCallbacks (BSP_QSPI_Cb_t *CallBacks);
int32_t BSP_QSPI_RegisterDefaultMspCallbacks (void);
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

int32_t BSP_QSPI_GetStatus(void);
void BSP_QSPI_GetChipInfo(BSP_QSPI_Info_t *pInfo);
int32_t BSP_QSPI_ReadChipID(uint8_t *Id);
int32_t BSP_QSPI_EnableMemoryMappedMode(void);
int32_t BSP_QSPI_DisableMemoryMappedMode(void);
int32_t BSP_QSPI_EraseChip(void);
int32_t BSP_QSPI_EraseBlock(uint32_t BlockAddress, BSP_QSPI_Erase_e BlockSize);
int32_t BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
int32_t BSP_QSPI_SyncWrite(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
int32_t BSP_QSPI_PageWrite(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);

void bsp_InitQspiFlash_MT25(void);


#endif /* BSP_QSPI_FLASH_H */
