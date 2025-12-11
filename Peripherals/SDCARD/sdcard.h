/**
 ******************************************************************************
 * @file    sdcard.h
 * @author  Mahonri
 * @brief   SD Card peripheral driver for STM32F429I Discovery board
 *          This file provides a clean architecture interface for SD Card operations
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_H
#define __SDCARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief SD Card status enumeration
 */
typedef enum {
    SDCARD_STATUS_OK       = 0x00U,
    SDCARD_STATUS_ERROR    = 0x01U,
    SDCARD_STATUS_BUSY     = 0x02U,
    SDCARD_STATUS_TIMEOUT  = 0x03U,
    SDCARD_STATUS_NOT_READY = 0x04U,
    SDCARD_STATUS_NO_CARD  = 0x05U,
    SDCARD_STATUS_WRITE_PROTECTED = 0x06U
} SDCARD_StatusTypeDef;

/**
 * @brief SD Card information structure
 */
typedef struct {
    uint32_t CardType;        /*!< Specifies the card Type */
    uint32_t CardVersion;     /*!< Specifies the card version */
    uint32_t Class;           /*!< Specifies the class of the card class */
    uint32_t RelCardAdd;      /*!< Specifies the Relative Card Address */
    uint32_t BlockNbr;        /*!< Specifies the Card Capacity in blocks */
    uint32_t BlockSize;       /*!< Specifies one block size in bytes */
    uint32_t LogBlockNbr;     /*!< Specifies the Card logical Capacity in blocks */
    uint32_t LogBlockSize;    /*!< Specifies logical block size in bytes */
    uint32_t CardSize;        /*!< Specifies the card size in bytes */
    bool WriteProtected;      /*!< Specifies if the card is write protected */
} SDCARD_CardInfoTypeDef;

/**
 * @brief SD Card operation mode
 */
typedef enum {
    SDCARD_MODE_POLLING     = 0x00U,
    SDCARD_MODE_INTERRUPT   = 0x01U,
    SDCARD_MODE_DMA         = 0x02U
} SDCARD_ModeTypeDef;

/* Exported constants --------------------------------------------------------*/

/**
 * @defgroup SD_BLOCK_SIZE SD Block Size
 * @{
 */
#define SDCARD_BLOCK_SIZE              512U     /*!< SD Card block size in bytes */
/**
 * @}
 */

/**
 * @defgroup SD_TIMEOUT SD Timeout Values
 * @{
 */
#define SDCARD_TIMEOUT_DEFAULT         5000U    /*!< Default timeout in ms */
#define SDCARD_TIMEOUT_READ            1000U    /*!< Read timeout in ms */
#define SDCARD_TIMEOUT_WRITE           2000U    /*!< Write timeout in ms */
#define SDCARD_TIMEOUT_ERASE           10000U   /*!< Erase timeout in ms */
/**
 * @}
 */

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief  Convert sector number to byte address
 * @param  SECTOR: Sector number
 * @retval Byte address
 */
#define SDCARD_SECTOR_TO_BYTES(SECTOR)     ((SECTOR) * SDCARD_BLOCK_SIZE)

/**
 * @brief  Convert byte address to sector number
 * @param  BYTES: Byte address
 * @retval Sector number
 */
#define SDCARD_BYTES_TO_SECTOR(BYTES)      ((BYTES) / SDCARD_BLOCK_SIZE)

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initialize the SD Card peripheral
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Init(void);

/**
 * @brief  Deinitialize the SD Card peripheral
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_DeInit(void);

/**
 * @brief  Get SD Card information
 * @param  pCardInfo: Pointer to card information structure
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_GetCardInfo(SDCARD_CardInfoTypeDef* pCardInfo);

/**
 * @brief  Check if SD Card is present
 * @retval bool: true if card is present, false otherwise
 */
bool SDCARD_IsCardPresent(void);

/**
 * @brief  Check if SD Card is write protected
 * @retval bool: true if card is write protected, false otherwise
 */
bool SDCARD_IsWriteProtected(void);

/**
 * @brief  Read block(s) from SD Card
 * @param  pData: Pointer to buffer to store read data
 * @param  BlockAddr: Block address to read from
 * @param  NumOfBlocks: Number of blocks to read
 * @param  Timeout: Timeout value in milliseconds
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_ReadBlocks(uint8_t* pData, uint32_t BlockAddr,
                                       uint32_t NumOfBlocks, uint32_t Timeout);

/**
 * @brief  Write block(s) to SD Card
 * @param  pData: Pointer to data buffer to write
 * @param  BlockAddr: Block address to write to
 * @param  NumOfBlocks: Number of blocks to write
 * @param  Timeout: Timeout value in milliseconds
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_WriteBlocks(const uint8_t* pData, uint32_t BlockAddr,
                                        uint32_t NumOfBlocks, uint32_t Timeout);

/**
 * @brief  Read block(s) from SD Card using DMA
 * @param  pData: Pointer to buffer to store read data
 * @param  BlockAddr: Block address to read from
 * @param  NumOfBlocks: Number of blocks to read
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_ReadBlocks_DMA(uint8_t* pData, uint32_t BlockAddr,
                                           uint32_t NumOfBlocks);

/**
 * @brief  Write block(s) to SD Card using DMA
 * @param  pData: Pointer to data buffer to write
 * @param  BlockAddr: Block address to write to
 * @param  NumOfBlocks: Number of blocks to write
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_WriteBlocks_DMA(const uint8_t* pData, uint32_t BlockAddr,
                                            uint32_t NumOfBlocks);

/**
 * @brief  Erase block(s) on SD Card
 * @param  StartAddr: Start block address to erase
 * @param  EndAddr: End block address to erase
 * @param  Timeout: Timeout value in milliseconds
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Erase(uint32_t StartAddr, uint32_t EndAddr, uint32_t Timeout);

/**
 * @brief  Get SD Card state
 * @retval HAL_SD_CardStateTypeDef: Current card state
 */
HAL_SD_CardStateTypeDef SDCARD_GetCardState(void);

/**
 * @brief  Abort ongoing SD Card operation
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Abort(void);

/**
 * @brief  Format SD Card (creates basic file system structure)
 * @param  FormatType: Type of format (0 = Quick format, 1 = Full format)
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_Format(uint8_t FormatType);

/**
 * @brief  Get SD Card capacity in bytes
 * @retval uint64_t: Card capacity in bytes
 */
uint64_t SDCARD_GetCapacity(void);

/**
 * @brief  Get free space on SD Card in bytes
 * @retval uint64_t: Free space in bytes
 */
uint64_t SDCARD_GetFreeSpace(void);

/**
 * @brief  Test SD Card performance (read/write speed)
 * @param  pReadSpeed: Pointer to store read speed in KB/s
 * @param  pWriteSpeed: Pointer to store write speed in KB/s
 * @retval SDCARD_StatusTypeDef: Status of the operation
 */
SDCARD_StatusTypeDef SDCARD_TestPerformance(uint32_t* pReadSpeed, uint32_t* pWriteSpeed);

/**
 * @brief  SD Card transfer complete callback (for DMA operations)
 * @note   This function should be implemented by the user
 * @retval None
 */
void SDCARD_TxCpltCallback(void);

/**
 * @brief  SD Card receive complete callback (for DMA operations)
 * @note   This function should be implemented by the user
 * @retval None
 */
void SDCARD_RxCpltCallback(void);

/**
 * @brief  SD Card error callback
 * @note   This function should be implemented by the user
 * @retval None
 */
void SDCARD_ErrorCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* __SDCARD_H */
