/**
  ******************************************************************************
  * @file    flash.h
  * @brief   Internal Flash memory module interface
  * @details This file contains all the function prototypes for
  *          the internal Flash memory read/write/erase operations.
  *          It provides APIs to store and retrieve data from Flash
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief FLASH Status enumeration
 * @note  Prefixed with FLASH_STATUS_ to avoid conflicts with HAL defines
 */
typedef enum {
    FLASH_STATUS_OK = 0,              /**< Operation completed successfully */
    FLASH_STATUS_ERROR,               /**< General error occurred */
    FLASH_STATUS_ERROR_PROGRAM,       /**< Programming error */
    FLASH_STATUS_ERROR_WRP,           /**< Write protection error */
    FLASH_STATUS_ERROR_OP,            /**< Operation error */
    FLASH_STATUS_INVALID_PARAM,       /**< Invalid parameter provided */
    FLASH_STATUS_INVALID_ADDRESS,     /**< Invalid address */
    FLASH_STATUS_BUSY                 /**< Flash is busy */
} FLASH_StatusTypeDef;

/**
 * @brief Flash sector information structure
 */
typedef struct {
    uint32_t SectorNumber;     /**< Sector number (0-23 for 2MB) */
    uint32_t StartAddress;     /**< Sector start address */
    uint32_t Size;             /**< Sector size in bytes */
} FLASH_SectorInfoTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup FLASH_Constants FLASH Driver Constants
 * @{
 */

/**
 * @brief STM32F429 Flash memory map
 * @note  Total Flash: 2MB (Bank 1: 1MB, Bank 2: 1MB)
 */
#define FLASH_BASE_ADDRESS          0x08000000U     /**< Flash base address */
#define FLASH_END_ADDRESS           0x081FFFFFU     /**< Flash end address (2MB) */
#define FLASH_BANK1_END             0x080FFFFFU     /**< Bank 1 end address */
#define FLASH_BANK2_BASE            0x08100000U     /**< Bank 2 base address */

/**
 * @brief Flash sector sizes for STM32F429
 * @note  Bank 1 and Bank 2 have identical layout
 */
#define FLASH_SECTOR_SIZE_16KB      0x4000U         /**< 16 KB sector */
#define FLASH_SECTOR_SIZE_64KB      0x10000U        /**< 64 KB sector */
#define FLASH_SECTOR_SIZE_128KB     0x20000U        /**< 128 KB sector */

/**
 * @brief User data storage area (using last sector of Bank 1)
 * @note  Sector 11 (128KB) is typically safe for user data
 */
#define FLASH_USER_START_ADDRESS    0x080E0000U     /**< Sector 11 start */
#define FLASH_USER_END_ADDRESS      0x080FFFFFU     /**< Sector 11 end */
#define FLASH_USER_SECTOR           FLASH_SECTOR_11 /**< User data sector */
#define FLASH_USER_SIZE             FLASH_SECTOR_SIZE_128KB

/**
 * @brief Flash operation timeout
 */
#define FLASH_TIMEOUT_VALUE         50000U          /**< 50 seconds timeout */

/**
 * @brief Number of sectors
 */
#define FLASH_SECTOR_TOTAL          24U             /**< Total sectors (both banks) */
#define FLASH_SECTOR_BANK1          12U             /**< Sectors in Bank 1 */
#define FLASH_SECTOR_BANK2          12U             /**< Sectors in Bank 2 */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup FLASH_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Unlock Flash for programming
 * @details Must be called before any write/erase operation
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_Unlock(void);

/**
 * @brief   Lock Flash to prevent accidental writes
 * @details Should be called after write/erase operations
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_Lock(void);

/** @} */

/** @defgroup FLASH_Erase_Functions Erase Functions
 * @{
 */

/**
 * @brief   Erase a Flash sector
 * @details Erases specified sector (sets all bytes to 0xFF)
 * @param   sector Sector number (FLASH_SECTOR_0 to FLASH_SECTOR_23)
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseSector(uint32_t sector);

/**
 * @brief   Erase multiple sectors
 * @details Erases sectors from startSector to endSector
 * @param   startSector First sector to erase
 * @param   endSector Last sector to erase
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseSectors(uint32_t startSector, uint32_t endSector);

/**
 * @brief   Erase user data sector
 * @details Erases the designated user data storage area
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseUserSector(void);

/** @} */

/** @defgroup FLASH_Write_Functions Write Functions
 * @{
 */

/**
 * @brief   Write a byte to Flash
 * @details Programs a single byte at specified address
 * @param   address Flash address to write
 * @param   data Byte value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteByte(uint32_t address, uint8_t data);

/**
 * @brief   Write a half-word (16-bit) to Flash
 * @details Programs 16-bit value at specified address
 * @param   address Flash address to write (must be 2-byte aligned)
 * @param   data Half-word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteHalfWord(uint32_t address, uint16_t data);

/**
 * @brief   Write a word (32-bit) to Flash
 * @details Programs 32-bit value at specified address
 * @param   address Flash address to write (must be 4-byte aligned)
 * @param   data Word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteWord(uint32_t address, uint32_t data);

/**
 * @brief   Write a double-word (64-bit) to Flash
 * @details Programs 64-bit value at specified address
 * @param   address Flash address to write (must be 8-byte aligned)
 * @param   data Double-word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteDoubleWord(uint32_t address, uint64_t data);

/**
 * @brief   Write buffer to Flash
 * @details Programs array of bytes to Flash
 * @param   address Start address in Flash
 * @param   data Pointer to data buffer
 * @param   length Number of bytes to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t* data, uint32_t length);

/**
 * @brief   Write 32-bit buffer to Flash
 * @details Programs array of 32-bit words to Flash
 * @param   address Start address in Flash (must be 4-byte aligned)
 * @param   data Pointer to data buffer
 * @param   count Number of 32-bit words to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteBuffer32(uint32_t address, const uint32_t* data, uint32_t count);

/** @} */

/** @defgroup FLASH_Read_Functions Read Functions
 * @{
 */

/**
 * @brief   Read a byte from Flash
 * @details Reads single byte from specified address
 * @param   address Flash address to read
 * @retval  uint8_t Byte value at address
 */
uint8_t FLASH_ReadByte(uint32_t address);

/**
 * @brief   Read a half-word from Flash
 * @details Reads 16-bit value from specified address
 * @param   address Flash address to read (should be 2-byte aligned)
 * @retval  uint16_t Half-word value at address
 */
uint16_t FLASH_ReadHalfWord(uint32_t address);

/**
 * @brief   Read a word from Flash
 * @details Reads 32-bit value from specified address
 * @param   address Flash address to read (should be 4-byte aligned)
 * @retval  uint32_t Word value at address
 */
uint32_t FLASH_ReadWord(uint32_t address);

/**
 * @brief   Read buffer from Flash
 * @details Reads array of bytes from Flash
 * @param   address Start address in Flash
 * @param   data Pointer to destination buffer
 * @param   length Number of bytes to read
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_ReadBuffer(uint32_t address, uint8_t* data, uint32_t length);

/** @} */

/** @defgroup FLASH_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Get sector number from address
 * @details Returns sector number containing specified address
 * @param   address Flash address
 * @retval  uint32_t Sector number (0-23) or 0xFFFFFFFF if invalid
 */
uint32_t FLASH_GetSector(uint32_t address);

/**
 * @brief   Get sector information
 * @details Returns start address and size of specified sector
 * @param   sector Sector number
 * @param   info Pointer to sector info structure
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_GetSectorInfo(uint32_t sector, FLASH_SectorInfoTypeDef* info);

/**
 * @brief   Check if address is valid Flash address
 * @details Validates address is within Flash memory range
 * @param   address Address to validate
 * @retval  bool True if address is valid
 */
bool FLASH_IsValidAddress(uint32_t address);

/**
 * @brief   Check if Flash region is erased
 * @details Verifies all bytes in region are 0xFF
 * @param   address Start address
 * @param   length Number of bytes to check
 * @retval  bool True if region is erased
 */
bool FLASH_IsErased(uint32_t address, uint32_t length);

/**
 * @brief   Get Flash status string
 * @details Converts status code to human-readable string
 * @param   status Flash status code
 * @retval  const char* Status description string
 */
const char* FLASH_GetStatusString(FLASH_StatusTypeDef status);

/**
 * @brief   Wait for Flash operation to complete
 * @details Polls busy flag until operation completes
 * @param   timeout Timeout in milliseconds
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WaitForOperation(uint32_t timeout);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H__ */
