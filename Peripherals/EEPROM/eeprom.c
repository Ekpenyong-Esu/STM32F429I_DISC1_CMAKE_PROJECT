/**
 * @file    eeprom.c
 * @brief   EEPROM Driver Implementation for STM32F429
 * @details Core implementation for I2C EEPROM driver
 *          Supports M24LR64 and M24Cxx series EEPROMs
 * @version 2.0
 * @date    2026-01-03
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "i2c.h"
#include <string.h>
#include "log.h"

/* Private constants ---------------------------------------------------------*/

/**
 * @brief EEPROM type configurations lookup table
 */
static const EEPROM_ConfigTypeDef eepromConfigs[] = {
    /* M24LR64 - 64Kbit (8KB) */
    {
        .i2cAddress = EEPROM_I2C_ADDRESS_A01,
        .i2cAddressAlt = EEPROM_I2C_ADDRESS_A02,
        .totalSize = EEPROM_SIZE_8KB,
        .pageSize = EEPROM_PAGESIZE_4,
        .addressSize = 2,
        .writeTime = 5
    },
    /* M24C01 - 1Kbit (128B) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_128B,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1,
        .writeTime = 5
    },
    /* M24C02 - 2Kbit (256B) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_256B,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1,
        .writeTime = 5
    },
    /* M24C04 - 4Kbit (512B) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_512B,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1,
        .writeTime = 5
    },
    /* M24C08 - 8Kbit (1KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_1KB,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1,
        .writeTime = 5
    },
    /* M24C16 - 16Kbit (2KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_2KB,
        .pageSize = EEPROM_PAGESIZE_16,
        .addressSize = 1,
        .writeTime = 5
    },
    /* M24C32 - 32Kbit (4KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_4KB,
        .pageSize = EEPROM_PAGESIZE_32,
        .addressSize = 2,
        .writeTime = 5
    },
    /* M24C64 - 64Kbit (8KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_8KB,
        .pageSize = EEPROM_PAGESIZE_32,
        .addressSize = 2,
        .writeTime = 5
    },
    /* M24C128 - 128Kbit (16KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_16KB,
        .pageSize = EEPROM_PAGESIZE_64,
        .addressSize = 2,
        .writeTime = 5
    },
    /* M24C256 - 256Kbit (32KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_32KB,
        .pageSize = EEPROM_PAGESIZE_64,
        .addressSize = 2,
        .writeTime = 5
    },
    /* M24C512 - 512Kbit (64KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_64KB,
        .pageSize = EEPROM_PAGESIZE_128,
        .addressSize = 2,
        .writeTime = 5
    },
    /* AT24C256 - 256Kbit (32KB) */
    {
        .i2cAddress = 0x50,
        .i2cAddressAlt = 0x50,
        .totalSize = EEPROM_SIZE_32KB,
        .pageSize = EEPROM_PAGESIZE_64,
        .addressSize = 2,
        .writeTime = 5
    }
};

/* Private function prototypes -----------------------------------------------*/
static EEPROM_StatusTypeDef EEPROM_IO_Write(EEPROM_HandleTypeDef* handle,
                                             uint16_t address,
                                             const uint8_t* data,
                                             uint16_t length);
static EEPROM_StatusTypeDef EEPROM_IO_Read(EEPROM_HandleTypeDef* handle,
                                            uint16_t address,
                                            uint8_t* data,
                                            uint16_t length);
static EEPROM_StatusTypeDef EEPROM_IO_IsDeviceReady(EEPROM_HandleTypeDef* handle);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize EEPROM with default M24LR64 configuration
 */
EEPROM_StatusTypeDef EEPROM_Init(EEPROM_HandleTypeDef* handle)
{
    return EEPROM_InitType(handle, EEPROM_TYPE_M24LR64);
}

/**
 * @brief   Initialize EEPROM with specific type
 */
EEPROM_StatusTypeDef EEPROM_InitType(EEPROM_HandleTypeDef* handle, EEPROM_TypeDef type)
{
    log_debug("EEPROM: Initializing EEPROM");

    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (type >= EEPROM_TYPE_CUSTOM) {
        return EEPROM_INVALID_PARAM;
    }

    /* Get configuration for this type */
    memcpy(&handle->config, &eepromConfigs[type], sizeof(EEPROM_ConfigTypeDef));
    handle->type = type;

    /* Initialize I2C */
    I2C_Init();

    /* Try primary address first */
    handle->activeAddress = handle->config.i2cAddress;
    if (EEPROM_IO_IsDeviceReady(handle) != EEPROM_OK) {
        /* Try alternate address */
        handle->activeAddress = handle->config.i2cAddressAlt;
        if (EEPROM_IO_IsDeviceReady(handle) != EEPROM_OK) {
            handle->initialized = false;
            return EEPROM_ERROR;
        }
    }

    handle->initialized = true;
    log_debug("EEPROM: EEPROM initialized successfully");
    return EEPROM_OK;
}

/**
 * @brief   Initialize EEPROM with custom configuration
 */
EEPROM_StatusTypeDef EEPROM_InitCustom(EEPROM_HandleTypeDef* handle,
                                        const EEPROM_ConfigTypeDef* config)
{
    if (handle == NULL || config == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    /* Validate configuration */
    if (config->totalSize == 0 || config->pageSize == 0) {
        return EEPROM_INVALID_PARAM;
    }

    if (config->addressSize != 1 && config->addressSize != 2) {
        return EEPROM_INVALID_PARAM;
    }

    /* Copy configuration */
    memcpy(&handle->config, config, sizeof(EEPROM_ConfigTypeDef));
    handle->type = EEPROM_TYPE_CUSTOM;

    /* Initialize I2C */
    I2C_Init();

    /* Try primary address */
    handle->activeAddress = handle->config.i2cAddress;
    if (EEPROM_IO_IsDeviceReady(handle) != EEPROM_OK) {
        /* Try alternate address */
        handle->activeAddress = handle->config.i2cAddressAlt;
        if (EEPROM_IO_IsDeviceReady(handle) != EEPROM_OK) {
            handle->initialized = false;
            return EEPROM_ERROR;
        }
    }

    handle->initialized = true;
    return EEPROM_OK;
}

/**
 * @brief   Deinitialize EEPROM
 */
EEPROM_StatusTypeDef EEPROM_DeInit(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    handle->initialized = false;
    return EEPROM_OK;
}

/**
 * @brief   Read single byte from EEPROM
 */
EEPROM_StatusTypeDef EEPROM_ReadByte(EEPROM_HandleTypeDef* handle,
                                      uint16_t address, uint8_t* data)
{
    if (handle == NULL || data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if (address >= handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    return EEPROM_IO_Read(handle, address, data, 1);
}

/**
 * @brief   Read multiple bytes from EEPROM
 */
EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef* handle,
                                  uint16_t address, uint8_t* data, uint16_t length)
{
    if (handle == NULL || data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if (length == 0) {
        return EEPROM_OK;
    }

    if ((address + length) > handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    return EEPROM_IO_Read(handle, address, data, length);
}

/**
 * @brief   Read 16-bit value from EEPROM
 */
EEPROM_StatusTypeDef EEPROM_ReadWord(EEPROM_HandleTypeDef* handle,
                                      uint16_t address, uint16_t* data)
{
    uint8_t buffer[2] = {0};
    EEPROM_StatusTypeDef status = EEPROM_ERROR;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, buffer, 2);
    if (status == EEPROM_OK) {
        *data = ((uint16_t)buffer[0] << 8) | buffer[1];
    }

    return status;
}

/**
 * @brief   Read 32-bit value from EEPROM
 */
EEPROM_StatusTypeDef EEPROM_ReadDWord(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint32_t* data)
{
    uint8_t buffer[4] = {0};
    EEPROM_StatusTypeDef status = EEPROM_ERROR;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, buffer, 4);
    if (status == EEPROM_OK) {
        *data = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) |
                ((uint32_t)buffer[2] << 8) | buffer[3];
    }

    return status;
}

/**
 * @brief   Read float value from EEPROM
 */
EEPROM_StatusTypeDef EEPROM_ReadFloat(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, float* data)
{
    union {
        float f;
        uint8_t b[4];
    } conv = {0};
    EEPROM_StatusTypeDef status = EEPROM_ERROR;

    if (data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    status = EEPROM_Read(handle, address, conv.b, 4);
    if (status == EEPROM_OK) {
        *data = conv.f;
    }

    return status;
}

/**
 * @brief   Write single byte to EEPROM
 */
EEPROM_StatusTypeDef EEPROM_WriteByte(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint8_t data)
{
    EEPROM_StatusTypeDef status = EEPROM_ERROR;

    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if (address >= handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    status = EEPROM_IO_Write(handle, address, &data, 1);
    if (status != EEPROM_OK) {
        return status;
    }

    /* Wait for write cycle to complete */
    return EEPROM_WaitReady(handle);
}

/**
 * @brief   Write single page to EEPROM
 */
EEPROM_StatusTypeDef EEPROM_WritePage(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, const uint8_t* data, uint8_t length)
{
    EEPROM_StatusTypeDef status = EEPROM_ERROR;

    if (handle == NULL || data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if (length == 0) {
        return EEPROM_OK;
    }

    if (length > handle->config.pageSize) {
        length = handle->config.pageSize;
    }

    if ((address + length) > handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    status = EEPROM_IO_Write(handle, address, data, length);
    if (status != EEPROM_OK) {
        return status;
    }

    /* Wait for write cycle to complete */
    return EEPROM_WaitReady(handle);
}

/**
 * @brief   Write multiple bytes to EEPROM (handles page boundaries)
 */
EEPROM_StatusTypeDef EEPROM_Write(EEPROM_HandleTypeDef* handle,
                                   uint16_t address, const uint8_t* data, uint16_t length)
{
    EEPROM_StatusTypeDef status = EEPROM_ERROR;
    uint16_t pageSize = 0;
    uint16_t offset = 0;
    uint16_t bytesToWrite = 0;
    uint16_t bytesWritten = 0;

    if (handle == NULL || data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if (length == 0) {
        return EEPROM_OK;
    }

    if ((address + length) > handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    pageSize = handle->config.pageSize;

    while (bytesWritten < length) {
        /* Calculate offset within current page */
        offset = (address + bytesWritten) % pageSize;

        /* Calculate bytes remaining in current page */
        bytesToWrite = pageSize - offset;

        /* Don't write more than remaining data */
        if (bytesToWrite > (length - bytesWritten)) {
            bytesToWrite = length - bytesWritten;
        }

        /* Write to current page */
        status = EEPROM_WritePage(handle, address + bytesWritten,
                                   data + bytesWritten, (uint8_t)bytesToWrite);
        if (status != EEPROM_OK) {
            return status;
        }

        bytesWritten += bytesToWrite;
    }

    return EEPROM_OK;
}

/**
 * @brief   Write 16-bit value to EEPROM
 */
EEPROM_StatusTypeDef EEPROM_WriteWord(EEPROM_HandleTypeDef* handle,
                                       uint16_t address, uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data & 0xFF);

    return EEPROM_Write(handle, address, buffer, 2);
}

/**
 * @brief   Write 32-bit value to EEPROM
 */
EEPROM_StatusTypeDef EEPROM_WriteDWord(EEPROM_HandleTypeDef* handle,
                                        uint16_t address, uint32_t data)
{
    uint8_t buffer[4];

    buffer[0] = (uint8_t)(data >> 24);
    buffer[1] = (uint8_t)(data >> 16);
    buffer[2] = (uint8_t)(data >> 8);
    buffer[3] = (uint8_t)(data & 0xFF);

    return EEPROM_Write(handle, address, buffer, 4);
}

/**
 * @brief   Write float value to EEPROM
 */
EEPROM_StatusTypeDef EEPROM_WriteFloat(EEPROM_HandleTypeDef* handle,
                                        uint16_t address, float data)
{
    union {
        float f;
        uint8_t b[4];
    } conv;

    conv.f = data;
    return EEPROM_Write(handle, address, conv.b, 4);
}

/**
 * @brief   Wait for EEPROM to complete internal write cycle
 */
EEPROM_StatusTypeDef EEPROM_WaitReady(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    /* Poll device until ready */
    if (EEPROM_IO_IsDeviceReady(handle) != EEPROM_OK) {
        return EEPROM_TIMEOUT;
    }

    return EEPROM_OK;
}

/**
 * @brief   Check if EEPROM is ready for operations
 */
bool EEPROM_IsReady(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    return (EEPROM_IO_IsDeviceReady(handle) == EEPROM_OK);
}

/**
 * @brief   Erase entire EEPROM (write 0xFF to all locations)
 */
EEPROM_StatusTypeDef EEPROM_Erase(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    return EEPROM_EraseRange(handle, 0, handle->config.totalSize);
}

/**
 * @brief   Erase range of EEPROM memory
 */
EEPROM_StatusTypeDef EEPROM_EraseRange(EEPROM_HandleTypeDef* handle,
                                        uint16_t startAddress, uint16_t length)
{
    EEPROM_StatusTypeDef status = EEPROM_ERROR;
    uint8_t eraseBuffer[64] = {0};  /* Use page-sized buffer for efficiency */
    uint16_t bytesToErase = 0;
    uint16_t bytesErased = 0;
    uint16_t pageSize = 0;

    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if ((startAddress + length) > handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    /* Fill erase buffer with 0xFF */
    memset(eraseBuffer, 0xFF, sizeof(eraseBuffer));

    pageSize = handle->config.pageSize;
    if (pageSize > sizeof(eraseBuffer)) {
        pageSize = sizeof(eraseBuffer);
    }

    while (bytesErased < length) {
        bytesToErase = length - bytesErased;
        if (bytesToErase > pageSize) {
            bytesToErase = pageSize;
        }

        status = EEPROM_Write(handle, startAddress + bytesErased,
                               eraseBuffer, bytesToErase);
        if (status != EEPROM_OK) {
            return status;
        }

        bytesErased += bytesToErase;
    }

    return EEPROM_OK;
}

/**
 * @brief   Get EEPROM total size
 */
uint32_t EEPROM_GetSize(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0;
    }
    return handle->config.totalSize;
}

/**
 * @brief   Get EEPROM page size
 */
uint16_t EEPROM_GetPageSize(EEPROM_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0;
    }
    return handle->config.pageSize;
}

/**
 * @brief   Verify data in EEPROM matches buffer
 */
EEPROM_StatusTypeDef EEPROM_Verify(EEPROM_HandleTypeDef* handle,
                                    uint16_t address, const uint8_t* data, uint16_t length)
{
    EEPROM_StatusTypeDef status = EEPROM_ERROR;
    uint8_t readBuffer[64] = {0};
    uint16_t bytesToVerify = 0;
    uint16_t bytesVerified = 0;

    if (handle == NULL || data == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if ((address + length) > handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    while (bytesVerified < length) {
        bytesToVerify = length - bytesVerified;
        if (bytesToVerify > sizeof(readBuffer)) {
            bytesToVerify = sizeof(readBuffer);
        }

        status = EEPROM_Read(handle, address + bytesVerified,
                              readBuffer, bytesToVerify);
        if (status != EEPROM_OK) {
            return status;
        }

        if (memcmp(readBuffer, data + bytesVerified, bytesToVerify) != 0) {
            return EEPROM_ERROR;
        }

        bytesVerified += bytesToVerify;
    }

    return EEPROM_OK;
}

/**
 * @brief   Test EEPROM by writing and reading back test pattern
 */
EEPROM_StatusTypeDef EEPROM_Test(EEPROM_HandleTypeDef* handle, uint16_t testAddress)
{
    EEPROM_StatusTypeDef status = EEPROM_ERROR;
    uint8_t testPattern[] = {0xAA, 0x55, 0x00, 0xFF};
    uint8_t readBack[4] = {0};
    uint8_t originalData[4] = {0};

    if (handle == NULL) {
        return EEPROM_INVALID_PARAM;
    }

    if (!handle->initialized) {
        return EEPROM_NOT_INITIALIZED;
    }

    if ((testAddress + sizeof(testPattern)) > handle->config.totalSize) {
        return EEPROM_INVALID_ADDRESS;
    }

    /* Save original data */
    status = EEPROM_Read(handle, testAddress, originalData, sizeof(originalData));
    if (status != EEPROM_OK) {
        return status;
    }

    /* Write test pattern */
    status = EEPROM_Write(handle, testAddress, testPattern, sizeof(testPattern));
    if (status != EEPROM_OK) {
        return status;
    }

    /* Read back and verify */
    status = EEPROM_Read(handle, testAddress, readBack, sizeof(readBack));
    if (status != EEPROM_OK) {
        return status;
    }

    if (memcmp(testPattern, readBack, sizeof(testPattern)) != 0) {
        /* Restore original data before returning error */
        EEPROM_Write(handle, testAddress, originalData, sizeof(originalData));
        return EEPROM_ERROR;
    }

    /* Restore original data */
    status = EEPROM_Write(handle, testAddress, originalData, sizeof(originalData));

    return status;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Low-level I2C write to EEPROM
 */
static EEPROM_StatusTypeDef EEPROM_IO_Write(EEPROM_HandleTypeDef* handle,
                                             uint16_t address,
                                             const uint8_t* data,
                                             uint16_t length)
{
    uint16_t devAddr = (uint16_t)(handle->activeAddress << 1);
    uint16_t memAddrSize = handle->config.addressSize;

    if (I2C_Mem_Write(devAddr, address, memAddrSize, (uint8_t*)data,
                       length, EEPROM_TIMEOUT_DEFAULT) != I2C_OK) {
        return EEPROM_ERROR;
    }

    return EEPROM_OK;
}

/**
 * @brief   Low-level I2C read from EEPROM
 */
static EEPROM_StatusTypeDef EEPROM_IO_Read(EEPROM_HandleTypeDef* handle,
                                            uint16_t address,
                                            uint8_t* data,
                                            uint16_t length)
{
    uint16_t devAddr = (uint16_t)(handle->activeAddress << 1);
    uint16_t memAddrSize = handle->config.addressSize;

    if (I2C_Mem_Read(devAddr, address, memAddrSize, data,
                      length, EEPROM_TIMEOUT_DEFAULT) != I2C_OK) {
        return EEPROM_ERROR;
    }

    return EEPROM_OK;
}

/**
 * @brief   Check if EEPROM device is ready (ACK polling)
 */
static EEPROM_StatusTypeDef EEPROM_IO_IsDeviceReady(EEPROM_HandleTypeDef* handle)
{
    uint16_t devAddr = (uint16_t)(handle->activeAddress << 1);

    if (I2C_IsDeviceReady(devAddr, EEPROM_MAX_TRIALS,
                           EEPROM_TIMEOUT_DEFAULT) != I2C_OK) {
        return EEPROM_TIMEOUT;
    }

    return EEPROM_OK;
}
