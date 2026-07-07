/**
  ******************************************************************************
  * @file    flash.c
  * @brief   Internal Flash memory module implementation
  * @details This file provides code for Flash memory operations
  *          including read, write, and erase functions.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/* Sector addresses for STM32F429 (2MB Flash) */
static const uint32_t FLASH_SECTOR_ADDRESSES[] = {
    /* Bank 1 */
    0x08000000U,  /* Sector 0:  16KB */
    0x08004000U,  /* Sector 1:  16KB */
    0x08008000U,  /* Sector 2:  16KB */
    0x0800C000U,  /* Sector 3:  16KB */
    0x08010000U,  /* Sector 4:  64KB */
    0x08020000U,  /* Sector 5:  128KB */
    0x08040000U,  /* Sector 6:  128KB */
    0x08060000U,  /* Sector 7:  128KB */
    0x08080000U,  /* Sector 8:  128KB */
    0x080A0000U,  /* Sector 9:  128KB */
    0x080C0000U,  /* Sector 10: 128KB */
    0x080E0000U,  /* Sector 11: 128KB */
    /* Bank 2 */
    0x08100000U,  /* Sector 12: 16KB */
    0x08104000U,  /* Sector 13: 16KB */
    0x08108000U,  /* Sector 14: 16KB */
    0x0810C000U,  /* Sector 15: 16KB */
    0x08110000U,  /* Sector 16: 64KB */
    0x08120000U,  /* Sector 17: 128KB */
    0x08140000U,  /* Sector 18: 128KB */
    0x08160000U,  /* Sector 19: 128KB */
    0x08180000U,  /* Sector 20: 128KB */
    0x081A0000U,  /* Sector 21: 128KB */
    0x081C0000U,  /* Sector 22: 128KB */
    0x081E0000U,  /* Sector 23: 128KB */
    0x08200000U   /* End address */
};

/* Private function prototypes -----------------------------------------------*/
static FLASH_StatusTypeDef FLASH_ConvertHALStatus(HAL_StatusTypeDef halStatus);
static uint32_t FLASH_GetSectorSize(uint32_t sector);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convert HAL status to FLASH status
 * @param   halStatus HAL status code
 * @retval  FLASH_StatusTypeDef Converted status
 */
static FLASH_StatusTypeDef FLASH_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return FLASH_STATUS_OK;
        case HAL_ERROR:
            return FLASH_STATUS_ERROR;
        case HAL_BUSY:
            return FLASH_STATUS_BUSY;
        case HAL_TIMEOUT:
            return FLASH_STATUS_ERROR_OP;
        default:
            return FLASH_STATUS_ERROR;
    }
}

/**
 * @brief   Get sector size
 * @param   sector Sector number
 * @retval  uint32_t Sector size in bytes
 */
static uint32_t FLASH_GetSectorSize(uint32_t sector)
{
    uint32_t sectorInBank = sector % 12;

    if (sectorInBank < 4)
    {
        return FLASH_SECTOR_SIZE_16KB;
    }
    else if (sectorInBank == 4)
    {
        return FLASH_SECTOR_SIZE_64KB;
    }
    else
    {
        return FLASH_SECTOR_SIZE_128KB;
    }
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Unlock Flash for programming
 * @details Must be called before any write/erase operation
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_Unlock(void)
{
    HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();
    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Lock Flash to prevent accidental writes
 * @details Should be called after write/erase operations
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_Lock(void)
{
    HAL_StatusTypeDef halStatus = HAL_FLASH_Lock();
    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Erase a Flash sector
 * @details Erases specified sector (sets all bytes to 0xFF)
 * @param   sector Sector number (FLASH_SECTOR_0 to FLASH_SECTOR_23)
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseSector(uint32_t sector)
{
    if (sector >= FLASH_SECTOR_TOTAL)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    /* Clear pending flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;  /* 2.7V to 3.6V */
    eraseInit.Sector = sector;
    eraseInit.NbSectors = 1;

    HAL_StatusTypeDef halStatus = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    FLASH_Lock();

    if (halStatus != HAL_OK)
    {
        return FLASH_STATUS_ERROR;
    }

    if (sectorError != 0xFFFFFFFFU)
    {
        return FLASH_STATUS_ERROR;
    }

    return FLASH_STATUS_OK;
}

/**
 * @brief   Erase multiple sectors
 * @details Erases sectors from startSector to endSector
 * @param   startSector First sector to erase
 * @param   endSector Last sector to erase
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseSectors(uint32_t startSector, uint32_t endSector)
{
    if (startSector > endSector || endSector >= FLASH_SECTOR_TOTAL)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    /* Clear pending flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = startSector;
    eraseInit.NbSectors = endSector - startSector + 1;

    HAL_StatusTypeDef halStatus = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    FLASH_Lock();

    if (halStatus != HAL_OK || sectorError != 0xFFFFFFFFU)
    {
        return FLASH_STATUS_ERROR;
    }

    return FLASH_STATUS_OK;
}

/**
 * @brief   Erase user data sector
 * @details Erases the designated user data storage area
 * @param   None
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_EraseUserSector(void)
{
    return FLASH_EraseSector(FLASH_USER_SECTOR);
}

/**
 * @brief   Write a byte to Flash
 * @details Programs a single byte at specified address
 * @param   address Flash address to write
 * @param   data Byte value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteByte(uint32_t address, uint8_t data)
{
    if (!FLASH_IsValidAddress(address))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, data);

    FLASH_Lock();

    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Write a half-word (16-bit) to Flash
 * @details Programs 16-bit value at specified address
 * @param   address Flash address to write (must be 2-byte aligned)
 * @param   data Half-word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteHalfWord(uint32_t address, uint16_t data)
{
    if (!FLASH_IsValidAddress(address) || (address & 0x01))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);

    FLASH_Lock();

    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Write a word (32-bit) to Flash
 * @details Programs 32-bit value at specified address
 * @param   address Flash address to write (must be 4-byte aligned)
 * @param   data Word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteWord(uint32_t address, uint32_t data)
{
    if (!FLASH_IsValidAddress(address) || (address & 0x03))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);

    FLASH_Lock();

    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Write a double-word (64-bit) to Flash
 * @details Programs 64-bit value at specified address
 * @param   address Flash address to write (must be 8-byte aligned)
 * @param   data Double-word value to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteDoubleWord(uint32_t address, uint64_t data)
{
    if (!FLASH_IsValidAddress(address) || (address & 0x07))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);

    FLASH_Lock();

    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Write buffer to Flash
 * @details Programs array of bytes to Flash
 * @param   address Start address in Flash
 * @param   data Pointer to data buffer
 * @param   length Number of bytes to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t* data, uint32_t length)
{
    if (data == NULL || length == 0)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    if (!FLASH_IsValidAddress(address) || !FLASH_IsValidAddress(address + length - 1))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    /* Program bytes one at a time */
    for (uint32_t i = 0; i < length; i++)
    {
        HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                                                        address + i, data[i]);
        if (halStatus != HAL_OK)
        {
            FLASH_Lock();
            return FLASH_STATUS_ERROR_PROGRAM;
        }
    }

    FLASH_Lock();

    /* Verify written data */
    for (uint32_t i = 0; i < length; i++)
    {
        if (FLASH_ReadByte(address + i) != data[i])
        {
            return FLASH_STATUS_ERROR_PROGRAM;
        }
    }

    return FLASH_STATUS_OK;
}

/**
 * @brief   Write 32-bit buffer to Flash
 * @details Programs array of 32-bit words to Flash
 * @param   address Start address in Flash (must be 4-byte aligned)
 * @param   data Pointer to data buffer
 * @param   count Number of 32-bit words to write
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WriteBuffer32(uint32_t address, const uint32_t* data, uint32_t count)
{
    if (data == NULL || count == 0)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    if ((address & 0x03) != 0)
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    if (!FLASH_IsValidAddress(address) ||
        !FLASH_IsValidAddress(address + (count * 4) - 1))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    FLASH_StatusTypeDef status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK)
    {
        return status;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                                        address + (i * 4), data[i]);
        if (halStatus != HAL_OK)
        {
            FLASH_Lock();
            return FLASH_STATUS_ERROR_PROGRAM;
        }
    }

    FLASH_Lock();

    return FLASH_STATUS_OK;
}

/**
 * @brief   Read a byte from Flash
 * @details Reads single byte from specified address
 * @param   address Flash address to read
 * @retval  uint8_t Byte value at address
 */
uint8_t FLASH_ReadByte(uint32_t address)
{
    return *(__IO uint8_t*)address;
}

/**
 * @brief   Read a half-word from Flash
 * @details Reads 16-bit value from specified address
 * @param   address Flash address to read (should be 2-byte aligned)
 * @retval  uint16_t Half-word value at address
 */
uint16_t FLASH_ReadHalfWord(uint32_t address)
{
    return *(__IO uint16_t*)address;
}

/**
 * @brief   Read a word from Flash
 * @details Reads 32-bit value from specified address
 * @param   address Flash address to read (should be 4-byte aligned)
 * @retval  uint32_t Word value at address
 */
uint32_t FLASH_ReadWord(uint32_t address)
{
    return *(__IO uint32_t*)address;
}

/**
 * @brief   Read buffer from Flash
 * @details Reads array of bytes from Flash
 * @param   address Start address in Flash
 * @param   data Pointer to destination buffer
 * @param   length Number of bytes to read
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_ReadBuffer(uint32_t address, uint8_t* data, uint32_t length)
{
    if (data == NULL || length == 0)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    if (!FLASH_IsValidAddress(address) || !FLASH_IsValidAddress(address + length - 1))
    {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    memcpy(data, (const void*)address, length);

    return FLASH_STATUS_OK;
}

/**
 * @brief   Get sector number from address
 * @details Returns sector number containing specified address
 * @param   address Flash address
 * @retval  uint32_t Sector number (0-23) or 0xFFFFFFFF if invalid
 */
uint32_t FLASH_GetSector(uint32_t address)
{
    if (!FLASH_IsValidAddress(address))
    {
        return 0xFFFFFFFFU;
    }

    for (uint32_t i = 0; i < FLASH_SECTOR_TOTAL; i++)
    {
        if (address >= FLASH_SECTOR_ADDRESSES[i] &&
            address < FLASH_SECTOR_ADDRESSES[i + 1])
        {
            return i;
        }
    }

    return 0xFFFFFFFFU;
}

/**
 * @brief   Get sector information
 * @details Returns start address and size of specified sector
 * @param   sector Sector number
 * @param   info Pointer to sector info structure
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_GetSectorInfo(uint32_t sector, FLASH_SectorInfoTypeDef* info)
{
    if (info == NULL || sector >= FLASH_SECTOR_TOTAL)
    {
        return FLASH_STATUS_INVALID_PARAM;
    }

    info->SectorNumber = sector;
    info->StartAddress = FLASH_SECTOR_ADDRESSES[sector];
    info->Size = FLASH_GetSectorSize(sector);

    return FLASH_STATUS_OK;
}

/**
 * @brief   Check if address is valid Flash address
 * @details Validates address is within Flash memory range
 * @param   address Address to validate
 * @retval  bool True if address is valid
 */
bool FLASH_IsValidAddress(uint32_t address)
{
    return (address >= FLASH_BASE_ADDRESS && address <= FLASH_END_ADDRESS);
}

/**
 * @brief   Check if Flash region is erased
 * @details Verifies all bytes in region are 0xFF
 * @param   address Start address
 * @param   length Number of bytes to check
 * @retval  bool True if region is erased
 */
bool FLASH_IsErased(uint32_t address, uint32_t length)
{
    if (!FLASH_IsValidAddress(address) || !FLASH_IsValidAddress(address + length - 1))
    {
        return false;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        if (FLASH_ReadByte(address + i) != 0xFF)
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief   Get Flash status string
 * @details Converts status code to human-readable string
 * @param   status Flash status code
 * @retval  const char* Status description string
 */
const char* FLASH_GetStatusString(FLASH_StatusTypeDef status)
{
    switch (status)
    {
        case FLASH_STATUS_OK:
            return "FLASH_STATUS_OK";
        case FLASH_STATUS_ERROR:
            return "FLASH_STATUS_ERROR";
        case FLASH_STATUS_ERROR_PROGRAM:
            return "FLASH_STATUS_ERROR_PROGRAM";
        case FLASH_STATUS_ERROR_WRP:
            return "FLASH_STATUS_ERROR_WRP";
        case FLASH_STATUS_ERROR_OP:
            return "FLASH_STATUS_ERROR_OP";
        case FLASH_STATUS_INVALID_PARAM:
            return "FLASH_STATUS_INVALID_PARAM";
        case FLASH_STATUS_INVALID_ADDRESS:
            return "FLASH_STATUS_INVALID_ADDRESS";
        case FLASH_STATUS_BUSY:
            return "FLASH_STATUS_BUSY";
        default:
            return "UNKNOWN_STATUS";
    }
}

/**
 * @brief   Wait for Flash operation to complete
 * @details Polls busy flag until operation completes
 * @param   timeout Timeout in milliseconds
 * @retval  FLASH_StatusTypeDef Operation status
 */
FLASH_StatusTypeDef FLASH_WaitForOperation(uint32_t timeout)
{
    HAL_StatusTypeDef halStatus = FLASH_WaitForLastOperation(timeout);
    return FLASH_ConvertHALStatus(halStatus);
}
