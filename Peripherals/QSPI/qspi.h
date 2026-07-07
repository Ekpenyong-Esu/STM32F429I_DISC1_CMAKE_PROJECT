/**
  ******************************************************************************
  * @file    qspi.h
  * @brief   QSPI driver interface for STM32F429 Discovery Board
  * @details This file contains function prototypes and definitions for
  *          the Quad-SPI Flash memory interface on the STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef QSPI_H
#define QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
#define QSPI_TIMEOUT_DEFAULT            5000    /* Default timeout value */
#define QSPI_PAGE_SIZE                  256     /* Flash page size (256 bytes) */
#define QSPI_SECTOR_SIZE                4096    /* Flash sector size (4KB) */
#define QSPI_BLOCK_SIZE                 65536   /* Flash block size (64KB) */
#define QSPI_DEVICE_NAME_MAX_LENGTH     32      /* Maximum device name length */

/* QSPI Flash Commands (Generic) */
#define QSPI_CMD_WRITE_ENABLE           0x06    /* Write Enable */
#define QSPI_CMD_WRITE_DISABLE          0x04    /* Write Disable */
#define QSPI_CMD_READ_STATUS_REG        0x05    /* Read Status Register */
#define QSPI_CMD_WRITE_STATUS_REG       0x01    /* Write Status Register */
#define QSPI_CMD_READ_DATA              0x03    /* Read Data */
#define QSPI_CMD_FAST_READ              0x0B    /* Fast Read */
#define QSPI_CMD_QUAD_READ              0x6B    /* Quad Output Fast Read */
#define QSPI_CMD_PAGE_PROGRAM           0x02    /* Page Program */
#define QSPI_CMD_QUAD_PAGE_PROGRAM      0x32    /* Quad Input Fast Program */
#define QSPI_CMD_SECTOR_ERASE           0x20    /* Sector Erase (4KB) */
#define QSPI_CMD_BLOCK_ERASE_32K        0x52    /* Block Erase 32KB */
#define QSPI_CMD_BLOCK_ERASE_64K        0xD8    /* Block Erase 64KB */
#define QSPI_CMD_CHIP_ERASE             0xC7    /* Chip Erase */
#define QSPI_CMD_READ_ID                0x9F    /* Read JEDEC ID */
#define QSPI_CMD_READ_UNIQUE_ID         0x4B    /* Read Unique ID */
#define QSPI_CMD_DEEP_POWER_DOWN        0xB9    /* Deep Power Down */
#define QSPI_CMD_RELEASE_POWER_DOWN     0xAB    /* Release from Deep Power Down */

/* Status Register Bits */
#define QSPI_SR_BUSY                    0x01    /* Busy bit */
#define QSPI_SR_WEL                     0x02    /* Write Enable Latch */
#define QSPI_SR_BP0                     0x04    /* Block Protect 0 */
#define QSPI_SR_BP1                     0x08    /* Block Protect 1 */
#define QSPI_SR_BP2                     0x10    /* Block Protect 2 */
#define QSPI_SR_TB                      0x20    /* Top/Bottom Protect */
#define QSPI_SR_SEC                     0x40    /* Sector Protect */
#define QSPI_SR_SRP0                    0x80    /* Status Register Protect 0 */

/* GPIO Pin Definitions for QSPI */
#define QSPI_CLK_PIN                    GPIO_PIN_10   /* PF10 - QSPI_CLK */
#define QSPI_CLK_GPIO_PORT              GPIOF
#define QSPI_NCS_PIN                    GPIO_PIN_6    /* PB6 - QSPI_BK1_NCS */
#define QSPI_NCS_GPIO_PORT              GPIOB
#define QSPI_IO0_PIN                    GPIO_PIN_8    /* PF8 - QSPI_BK1_IO0 */
#define QSPI_IO0_GPIO_PORT              GPIOF
#define QSPI_IO1_PIN                    GPIO_PIN_9    /* PF9 - QSPI_BK1_IO1 */
#define QSPI_IO1_GPIO_PORT              GPIOF
#define QSPI_IO2_PIN                    GPIO_PIN_7    /* PF7 - QSPI_BK1_IO2 */
#define QSPI_IO2_GPIO_PORT              GPIOF
#define QSPI_IO3_PIN                    GPIO_PIN_6    /* PF6 - QSPI_BK1_IO3 */
#define QSPI_IO3_GPIO_PORT              GPIOF

/* QSPI Configuration */
#define QSPI_CLOCK_PRESCALER            1       /* Clock prescaler */
#define QSPI_FIFO_THRESHOLD             4       /* FIFO threshold */
#define QSPI_CHIP_SELECT_HIGH_TIME      2       /* Chip select high time */
#define QSPI_FLASH_SIZE                 23      /* Flash size (2^24 = 16MB) */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief QSPI Status enumeration
 */
typedef enum {
    QSPI_OK             = 0x00,     /**< Operation successful */
    QSPI_ERROR          = 0x01,     /**< Generic error */
    QSPI_BUSY           = 0x02,     /**< QSPI is busy */
    QSPI_TIMEOUT        = 0x03,     /**< Timeout occurred */
    QSPI_INVALID_PARAM  = 0x04,     /**< Invalid parameter */
    QSPI_NOT_SUPPORTED  = 0x05,     /**< Operation not supported */
    QSPI_WRITE_PROTECTED = 0x06,    /**< Memory is write protected */
    QSPI_ERASE_ERROR    = 0x07,     /**< Erase operation failed */
    QSPI_PROGRAM_ERROR  = 0x08      /**< Program operation failed */
} QSPI_StatusTypeDef;

/**
 * @brief QSPI Memory Information structure
 */
typedef struct {
    uint32_t FlashSize;             /**< Flash memory size in bytes */
    uint32_t PageSize;              /**< Page size in bytes */
    uint32_t SectorSize;            /**< Sector size in bytes */
    uint32_t BlockSize;             /**< Block size in bytes */
    uint8_t ManufacturerID;         /**< Manufacturer ID */
    uint8_t DeviceID1;              /**< Device ID 1 */
    uint8_t DeviceID2;              /**< Device ID 2 */
    char DeviceName[QSPI_DEVICE_NAME_MAX_LENGTH];            /**< Device name string */
} QSPI_MemoryInfoTypeDef;

/**
 * @brief QSPI Configuration structure
 */
typedef struct {
    uint32_t ClockPrescaler;        /**< Clock prescaler value */
    uint32_t FifoThreshold;         /**< FIFO threshold */
    uint32_t SampleShifting;        /**< Sample shifting mode */
    uint32_t FlashSize;             /**< Flash size (2^FlashSize bytes) */
    uint32_t ChipSelectHighTime;    /**< Chip select high time */
    uint32_t ClockMode;             /**< Clock mode */
    bool DualFlash;                 /**< Dual flash mode enable */
} QSPI_ConfigTypeDef;

/**
 * @brief QSPI Handle structure
 */
typedef struct {
    SPI_HandleTypeDef *hspi;        /**< HAL SPI handle for QSPI communication */
    QSPI_ConfigTypeDef Config;      /**< Configuration parameters */
    QSPI_MemoryInfoTypeDef MemInfo; /**< Memory information */
    uint32_t Timeout;               /**< Timeout value */
    bool IsInitialized;             /**< Initialization status */
    bool IsMemoryMapped;            /**< Memory mapped mode status */
} QSPI_HandleStructTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/* Initialization and Configuration */
QSPI_StatusTypeDef QSPI_Init(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_DeInit(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_Configure(QSPI_HandleStructTypeDef *hqspi_struct, QSPI_ConfigTypeDef *config);
QSPI_StatusTypeDef QSPI_Reset(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_ConfigTypeDef QSPI_GetDefaultConfig(void);

/* Memory Information */
QSPI_StatusTypeDef QSPI_GetMemoryInfo(QSPI_HandleStructTypeDef *hqspi_struct, QSPI_MemoryInfoTypeDef *memInfo);
QSPI_StatusTypeDef QSPI_ReadID(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *device_id);
QSPI_StatusTypeDef QSPI_ReadUniqueID(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *uniqueID);

/* Status and Control */
QSPI_StatusTypeDef QSPI_GetStatus(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *status);
QSPI_StatusTypeDef QSPI_WaitForWriteEnd(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_WriteEnable(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_WriteDisable(QSPI_HandleStructTypeDef *hqspi_struct);

/* Read Operations */
QSPI_StatusTypeDef QSPI_Read(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_FastRead(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_QuadRead(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size);

/* Write Operations */
QSPI_StatusTypeDef QSPI_WritePage(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_QuadWritePage(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_Write(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size);

/* Erase Operations */
QSPI_StatusTypeDef QSPI_EraseSector(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseBlock32K(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseBlock64K(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseChip(QSPI_HandleStructTypeDef *hqspi_struct);

/* Memory Mapped Mode */
QSPI_StatusTypeDef QSPI_EnableMemoryMappedMode(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_DisableMemoryMappedMode(QSPI_HandleStructTypeDef *hqspi_struct);

/* Power Management */
QSPI_StatusTypeDef QSPI_EnterDeepPowerDown(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(QSPI_HandleStructTypeDef *hqspi_struct);

/* Utility Functions */
bool QSPI_IsAddressValid(uint32_t address, uint32_t size);
uint32_t QSPI_GetSectorAddress(uint32_t address);
uint32_t QSPI_GetBlockAddress(uint32_t address);
const char* QSPI_GetStatusString(QSPI_StatusTypeDef status);

/* Callback Functions */
void QSPI_ErrorCallback(QSPI_HandleStructTypeDef *hqspi_struct);
void QSPI_CompletionCallback(QSPI_HandleStructTypeDef *hqspi_struct);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
