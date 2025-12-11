/**
  ******************************************************************************
  * @file    flash_example.c
  * @brief   Internal Flash memory usage examples
  * @details This file provides example code demonstrating how to use
  *          the Flash driver for storing configuration and user data.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define CONFIG_MAGIC        0xDEADBEEF
#define CONFIG_VERSION      1

/* Private types -------------------------------------------------------------*/

/**
 * @brief Example configuration structure
 */
typedef struct {
    uint32_t magic;             /* Magic number for validation */
    uint32_t version;           /* Configuration version */
    uint32_t deviceId;          /* Device identifier */
    uint8_t  deviceName[32];    /* Device name string */
    uint32_t baudRate;          /* UART baud rate */
    uint8_t  brightness;        /* Display brightness (0-100) */
    uint8_t  volume;            /* Audio volume (0-100) */
    uint16_t reserved;          /* Alignment padding */
    uint32_t checksum;          /* Simple checksum */
} DeviceConfig_t;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Calculate simple checksum
 */
static uint32_t CalculateChecksum(const DeviceConfig_t* config)
{
    uint32_t sum = 0;
    const uint8_t* data = (const uint8_t*)config;

    /* Sum all bytes except checksum field */
    for (size_t i = 0; i < sizeof(DeviceConfig_t) - sizeof(uint32_t); i++)
    {
        sum += data[i];
    }

    return sum;
}

/* Example 1: Basic read/write operations */
void FLASH_Example_Basic(void)
{
    printf("=== FLASH Basic Example ===\r\n\r\n");

    /* Check if user sector is erased */
    printf("Checking if user sector is erased...\r\n");
    if (!FLASH_IsErased(FLASH_USER_START_ADDRESS, 64))
    {
        printf("Erasing user sector...\r\n");
        FLASH_StatusTypeDef status = FLASH_EraseUserSector();
        if (status != FLASH_STATUS_OK)
        {
            printf("Erase failed: %s\r\n", FLASH_GetStatusString(status));
            return;
        }
        printf("Sector erased successfully\r\n");
    }
    else
    {
        printf("Sector already erased\r\n");
    }

    /* Write some test data */
    uint32_t testAddress = FLASH_USER_START_ADDRESS;

    printf("\r\nWriting test data...\r\n");

    /* Write a single byte */
    FLASH_WriteByte(testAddress, 0xAB);
    printf("  Wrote byte 0xAB at 0x%08lX\r\n", testAddress);

    /* Write a word */
    FLASH_WriteWord(testAddress + 4, 0x12345678);
    printf("  Wrote word 0x12345678 at 0x%08lX\r\n", testAddress + 4);

    /* Read back and verify */
    printf("\r\nReading back data...\r\n");
    uint8_t readByte = FLASH_ReadByte(testAddress);
    uint32_t readWord = FLASH_ReadWord(testAddress + 4);

    printf("  Read byte: 0x%02X (expected 0xAB)\r\n", readByte);
    printf("  Read word: 0x%08lX (expected 0x12345678)\r\n", readWord);

    if (readByte == 0xAB && readWord == 0x12345678)
    {
        printf("\r\n✓ Basic read/write test PASSED\r\n");
    }
    else
    {
        printf("\r\n✗ Basic read/write test FAILED\r\n");
    }
}

/* Example 2: Configuration storage */
void FLASH_Example_Config(void)
{
    printf("=== FLASH Configuration Storage Example ===\r\n\r\n");

    /* Create default configuration */
    DeviceConfig_t config = {
        .magic = CONFIG_MAGIC,
        .version = CONFIG_VERSION,
        .deviceId = 0x12345678,
        .deviceName = "STM32F429-DISC1",
        .baudRate = 115200,
        .brightness = 75,
        .volume = 50,
        .reserved = 0
    };
    config.checksum = CalculateChecksum(&config);

    printf("Default configuration:\r\n");
    printf("  Device ID: 0x%08lX\r\n", config.deviceId);
    printf("  Name: %s\r\n", config.deviceName);
    printf("  Baud: %lu\r\n", config.baudRate);
    printf("  Brightness: %u%%\r\n", config.brightness);
    printf("  Volume: %u%%\r\n", config.volume);
    printf("  Checksum: 0x%08lX\r\n", config.checksum);

    /* Erase sector first */
    printf("\r\nErasing configuration sector...\r\n");
    FLASH_EraseUserSector();

    /* Write configuration to Flash */
    printf("Writing configuration to Flash...\r\n");
    FLASH_StatusTypeDef status = FLASH_WriteBuffer(FLASH_USER_START_ADDRESS,
                                                   (uint8_t*)&config,
                                                   sizeof(DeviceConfig_t));
    if (status != FLASH_STATUS_OK)
    {
        printf("Write failed: %s\r\n", FLASH_GetStatusString(status));
        return;
    }

    /* Read configuration back */
    printf("Reading configuration from Flash...\r\n");
    DeviceConfig_t readConfig;
    status = FLASH_ReadBuffer(FLASH_USER_START_ADDRESS,
                              (uint8_t*)&readConfig,
                              sizeof(DeviceConfig_t));
    if (status != FLASH_STATUS_OK)
    {
        printf("Read failed: %s\r\n", FLASH_GetStatusString(status));
        return;
    }

    /* Validate configuration */
    printf("\r\nValidating configuration...\r\n");
    if (readConfig.magic != CONFIG_MAGIC)
    {
        printf("  ✗ Invalid magic number\r\n");
        return;
    }
    printf("  ✓ Magic number valid\r\n");

    uint32_t expectedChecksum = CalculateChecksum(&readConfig);
    if (readConfig.checksum != expectedChecksum)
    {
        printf("  ✗ Checksum mismatch\r\n");
        return;
    }
    printf("  ✓ Checksum valid\r\n");

    printf("\r\nRead configuration:\r\n");
    printf("  Device ID: 0x%08lX\r\n", readConfig.deviceId);
    printf("  Name: %s\r\n", readConfig.deviceName);
    printf("  Baud: %lu\r\n", readConfig.baudRate);

    printf("\r\n✓ Configuration storage test PASSED\r\n");
}

/* Example 3: Sector information */
void FLASH_Example_SectorInfo(void)
{
    printf("=== FLASH Sector Information Example ===\r\n\r\n");

    printf("STM32F429 Flash Memory Map:\r\n");
    printf("%-10s %-14s %-10s\r\n", "Sector", "Address", "Size");
    printf("----------------------------------------\r\n");

    for (uint32_t i = 0; i < FLASH_SECTOR_TOTAL; i++)
    {
        FLASH_SectorInfoTypeDef info;
        FLASH_GetSectorInfo(i, &info);

        const char* sizeStr;
        if (info.Size == FLASH_SECTOR_SIZE_16KB)
            sizeStr = "16 KB";
        else if (info.Size == FLASH_SECTOR_SIZE_64KB)
            sizeStr = "64 KB";
        else
            sizeStr = "128 KB";

        printf("Sector %-2lu  0x%08lX    %s%s\r\n",
               info.SectorNumber, info.StartAddress, sizeStr,
               (i == 11) ? " (User Data)" : "");

        if (i == 11)
        {
            printf("--- Bank 2 ---\r\n");
        }
    }

    printf("\r\nTotal Flash: 2 MB (1 MB per bank)\r\n");
}

/* Example 4: Buffer write */
void FLASH_Example_BufferWrite(void)
{
    printf("=== FLASH Buffer Write Example ===\r\n\r\n");

    /* Prepare test data */
    uint8_t testData[64];
    for (int i = 0; i < 64; i++)
    {
        testData[i] = (uint8_t)i;
    }

    /* Erase sector */
    printf("Erasing sector...\r\n");
    FLASH_EraseUserSector();

    /* Write buffer */
    printf("Writing 64-byte buffer...\r\n");
    FLASH_StatusTypeDef status = FLASH_WriteBuffer(FLASH_USER_START_ADDRESS,
                                                   testData, 64);
    if (status != FLASH_STATUS_OK)
    {
        printf("Write failed: %s\r\n", FLASH_GetStatusString(status));
        return;
    }

    /* Read back */
    uint8_t readData[64];
    FLASH_ReadBuffer(FLASH_USER_START_ADDRESS, readData, 64);

    /* Verify */
    printf("Verifying data...\r\n");
    bool success = true;
    for (int i = 0; i < 64; i++)
    {
        if (readData[i] != testData[i])
        {
            printf("Mismatch at offset %d: wrote 0x%02X, read 0x%02X\r\n",
                   i, testData[i], readData[i]);
            success = false;
        }
    }

    if (success)
    {
        printf("✓ Buffer write test PASSED\r\n");

        /* Display first 16 bytes */
        printf("\r\nFirst 16 bytes:\r\n");
        for (int i = 0; i < 16; i++)
        {
            printf("%02X ", readData[i]);
            if ((i + 1) % 8 == 0) printf("\r\n");
        }
    }
}

/* Example 5: Address lookup */
void FLASH_Example_AddressLookup(void)
{
    printf("=== FLASH Address Lookup Example ===\r\n\r\n");

    uint32_t testAddresses[] = {
        0x08000000,  /* Start of Flash */
        0x08005000,  /* In sector 1 */
        0x08015000,  /* In sector 4 */
        0x080E0000,  /* User sector start */
        0x081FFFFF,  /* End of Flash */
        0x08200000   /* Invalid (beyond Flash) */
    };

    for (size_t i = 0; i < sizeof(testAddresses) / sizeof(testAddresses[0]); i++)
    {
        uint32_t addr = testAddresses[i];
        bool valid = FLASH_IsValidAddress(addr);
        uint32_t sector = FLASH_GetSector(addr);

        printf("Address 0x%08lX: ", addr);
        if (valid)
        {
            printf("Valid, Sector %lu\r\n", sector);
        }
        else
        {
            printf("Invalid\r\n");
        }
    }
}

/**
 * @brief   Run all Flash examples
 */
void FLASH_RunExamples(void)
{
    printf("\r\n========================================\r\n");
    printf("      FLASH Driver Examples\r\n");
    printf("========================================\r\n\r\n");

    FLASH_Example_SectorInfo();
    printf("\r\n");

    FLASH_Example_AddressLookup();
    printf("\r\n");

    FLASH_Example_Basic();
    printf("\r\n");

    FLASH_Example_BufferWrite();
    printf("\r\n");

    FLASH_Example_Config();
}
