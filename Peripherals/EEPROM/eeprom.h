/**
 * @file    eeprom.h
 * @brief   EEPROM Driver for STM32F429 (M24LR64/M24Cxx series)
 * @details This file contains all the function prototypes for
 *          external I2C EEPROM read/write operations.
 *          Supports M24LR64 (64Kbit) and compatible EEPROMs.
 * @version 2.0
 * @date    2026-01-03
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief EEPROM Status enumeration
 */
typedef enum {
    EEPROM_OK = 0,              /**< Operation completed successfully */
    EEPROM_ERROR,               /**< General error occurred */
    EEPROM_TIMEOUT,             /**< Operation timed out */
    EEPROM_BUSY,                /**< EEPROM is busy (internal write cycle) */
    EEPROM_INVALID_PARAM,       /**< Invalid parameter provided */
    EEPROM_INVALID_ADDRESS,     /**< Invalid memory address */
    EEPROM_NOT_INITIALIZED      /**< Driver not initialized */
} EEPROM_StatusTypeDef;

/**
 * @brief EEPROM device type enumeration
 */
typedef enum {
    EEPROM_TYPE_M24LR64 = 0,    /**< M24LR64 - 64Kbit (8KB) EEPROM */
    EEPROM_TYPE_M24C01,         /**< M24C01 - 1Kbit (128B) EEPROM */
    EEPROM_TYPE_M24C02,         /**< M24C02 - 2Kbit (256B) EEPROM */
    EEPROM_TYPE_M24C04,         /**< M24C04 - 4Kbit (512B) EEPROM */
    EEPROM_TYPE_M24C08,         /**< M24C08 - 8Kbit (1KB) EEPROM */
    EEPROM_TYPE_M24C16,         /**< M24C16 - 16Kbit (2KB) EEPROM */
    EEPROM_TYPE_M24C32,         /**< M24C32 - 32Kbit (4KB) EEPROM */
    EEPROM_TYPE_M24C64,         /**< M24C64 - 64Kbit (8KB) EEPROM */
    EEPROM_TYPE_M24C128,        /**< M24C128 - 128Kbit (16KB) EEPROM */
    EEPROM_TYPE_M24C256,        /**< M24C256 - 256Kbit (32KB) EEPROM */
    EEPROM_TYPE_M24C512,        /**< M24C512 - 512Kbit (64KB) EEPROM */
    EEPROM_TYPE_AT24C256,       /**< AT24C256 - 256Kbit (32KB) EEPROM */
    EEPROM_TYPE_CUSTOM          /**< Custom EEPROM configuration */
} EEPROM_TypeDef;

/**
 * @brief EEPROM configuration structure
 */
typedef struct {
    uint8_t i2cAddress;         /**< I2C device address (7-bit) */
    uint8_t i2cAddressAlt;      /**< Alternative I2C address (for dual-address chips) */
    uint32_t totalSize;         /**< Total EEPROM size in bytes */
    uint16_t pageSize;          /**< Page size for write operations */
    uint8_t addressSize;        /**< Memory address size (1 or 2 bytes) */
    uint32_t writeTime;         /**< Write cycle time in milliseconds */
} EEPROM_ConfigTypeDef;

/**
 * @brief EEPROM handle structure
 */
typedef struct {
    EEPROM_ConfigTypeDef config;    /**< EEPROM configuration */
    EEPROM_TypeDef type;            /**< EEPROM type */
    uint8_t activeAddress;          /**< Currently active I2C address */
    bool initialized;               /**< Initialization flag */
} EEPROM_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup EEPROM_Constants EEPROM Driver Constants
 * @{
 */

/**
 * @brief Default I2C addresses for M24LR64
 */
#define EEPROM_I2C_ADDRESS_A01      0x50U   /**< Primary address (A0=1) - 7-bit */
#define EEPROM_I2C_ADDRESS_A02      0x53U   /**< Secondary address (A0=2) - 7-bit */

/**
 * @brief Default page sizes
 */
#define EEPROM_PAGESIZE_4           4U      /**< 4-byte page (M24LR64) */
#define EEPROM_PAGESIZE_8           8U      /**< 8-byte page */
#define EEPROM_PAGESIZE_16          16U     /**< 16-byte page */
#define EEPROM_PAGESIZE_32          32U     /**< 32-byte page */
#define EEPROM_PAGESIZE_64          64U     /**< 64-byte page */
#define EEPROM_PAGESIZE_128         128U    /**< 128-byte page */

/**
 * @brief EEPROM sizes
 */
#define EEPROM_SIZE_128B            128U        /**< 1Kbit */
#define EEPROM_SIZE_256B            256U        /**< 2Kbit */
#define EEPROM_SIZE_512B            512U        /**< 4Kbit */
#define EEPROM_SIZE_1KB             1024U       /**< 8Kbit */
#define EEPROM_SIZE_2KB             2048U       /**< 16Kbit */
#define EEPROM_SIZE_4KB             4096U       /**< 32Kbit */
#define EEPROM_SIZE_8KB             8192U       /**< 64Kbit (M24LR64) */
#define EEPROM_SIZE_16KB            16384U      /**< 128Kbit */
#define EEPROM_SIZE_32KB            32768U      /**< 256Kbit */
#define EEPROM_SIZE_64KB            65536U      /**< 512Kbit */

/**
 * @brief Timeout values
 */
#define EEPROM_TIMEOUT_DEFAULT      1000U   /**< Default timeout in ms */
#define EEPROM_WRITE_CYCLE_TIME     5U      /**< Typical write cycle time in ms */
#define EEPROM_MAX_TRIALS           300U    /**< Max trials for device ready */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup EEPROM_Init Initialization Functions
 * @{
 */

/**
 * @brief   Initialize EEPROM with default M24LR64 configuration
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Init(EEPROM_HandleTypeDef* handle);

/**
 * @brief   Initialize EEPROM with specific type
 * @param   handle Pointer to EEPROM handle
 * @param   type EEPROM type from EEPROM_TypeDef
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_InitType(EEPROM_HandleTypeDef* handle, EEPROM_TypeDef type);

/**
 * @brief   Initialize EEPROM with custom configuration
 * @param   handle Pointer to EEPROM handle
 * @param   config Pointer to configuration structure
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_InitCustom(EEPROM_HandleTypeDef* handle, 
                                        const EEPROM_ConfigTypeDef* config);

/**
 * @brief   Deinitialize EEPROM
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_DeInit(EEPROM_HandleTypeDef* handle);

/** @} */

/** @defgroup EEPROM_Read Read Functions
 * @{
 */

/**
 * @brief   Read single byte from EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store read byte
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadByte(EEPROM_HandleTypeDef* handle, 
                                      uint16_t address, uint8_t* data);

/**
 * @brief   Read multiple bytes from EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Pointer to data buffer
 * @param   length Number of bytes to read
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef* handle,
                                  uint16_t address, uint8_t* data, uint16_t length);

/**
 * @brief   Read 16-bit value from EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store read value
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadWord(EEPROM_HandleTypeDef* handle,
                                      uint16_t address, uint16_t* data);

/**
 * @brief   Read 32-bit value from EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store read value
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadDWord(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint32_t* data);

/**
 * @brief   Read float value from EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to read from
 * @param   data Pointer to store read value
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_ReadFloat(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, float* data);

/** @} */

/** @defgroup EEPROM_Write Write Functions
 * @{
 */

/**
 * @brief   Write single byte to EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Byte to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteByte(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint8_t data);

/**
 * @brief   Write multiple bytes to EEPROM (handles page boundaries)
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Pointer to data buffer
 * @param   length Number of bytes to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Write(EEPROM_HandleTypeDef* handle,
                                   uint16_t address, const uint8_t* data, uint16_t length);

/**
 * @brief   Write single page to EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting address (must be page-aligned)
 * @param   data Pointer to data buffer
 * @param   length Number of bytes to write (max page size)
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WritePage(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, const uint8_t* data, uint8_t length);

/**
 * @brief   Write 16-bit value to EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Value to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteWord(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint16_t data);

/**
 * @brief   Write 32-bit value to EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Value to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteDWord(EEPROM_HandleTypeDef* handle,
                                        uint16_t address, uint32_t data);

/**
 * @brief   Write float value to EEPROM
 * @param   handle Pointer to EEPROM handle
 * @param   address Memory address to write to
 * @param   data Value to write
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WriteFloat(EEPROM_HandleTypeDef* handle,
                                        uint16_t address, float data);

/** @} */

/** @defgroup EEPROM_Utility Utility Functions
 * @{
 */

/**
 * @brief   Wait for EEPROM to complete internal write cycle
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_WaitReady(EEPROM_HandleTypeDef* handle);

/**
 * @brief   Check if EEPROM is ready for operations
 * @param   handle Pointer to EEPROM handle
 * @retval  true if ready, false otherwise
 */
bool EEPROM_IsReady(EEPROM_HandleTypeDef* handle);

/**
 * @brief   Erase entire EEPROM (write 0xFF to all locations)
 * @param   handle Pointer to EEPROM handle
 * @retval  EEPROM_StatusTypeDef Operation status
 * @note    This operation can take several seconds
 */
EEPROM_StatusTypeDef EEPROM_Erase(EEPROM_HandleTypeDef* handle);

/**
 * @brief   Erase range of EEPROM memory
 * @param   handle Pointer to EEPROM handle
 * @param   startAddress Starting address
 * @param   length Number of bytes to erase
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_EraseRange(EEPROM_HandleTypeDef* handle,
                                        uint16_t startAddress, uint16_t length);

/**
 * @brief   Get EEPROM total size
 * @param   handle Pointer to EEPROM handle
 * @retval  Total size in bytes, 0 if not initialized
 */
uint32_t EEPROM_GetSize(EEPROM_HandleTypeDef* handle);

/**
 * @brief   Get EEPROM page size
 * @param   handle Pointer to EEPROM handle
 * @retval  Page size in bytes, 0 if not initialized
 */
uint16_t EEPROM_GetPageSize(EEPROM_HandleTypeDef* handle);

/**
 * @brief   Verify data in EEPROM matches buffer
 * @param   handle Pointer to EEPROM handle
 * @param   address Starting memory address
 * @param   data Pointer to data buffer to compare
 * @param   length Number of bytes to verify
 * @retval  EEPROM_StatusTypeDef EEPROM_OK if match, EEPROM_ERROR if mismatch
 */
EEPROM_StatusTypeDef EEPROM_Verify(EEPROM_HandleTypeDef* handle,
                                    uint16_t address, const uint8_t* data, uint16_t length);

/**
 * @brief   Test EEPROM by writing and reading back test pattern
 * @param   handle Pointer to EEPROM handle
 * @param   testAddress Address to use for test (will be modified)
 * @retval  EEPROM_StatusTypeDef Operation status
 */
EEPROM_StatusTypeDef EEPROM_Test(EEPROM_HandleTypeDef* handle, uint16_t testAddress);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H__ */
