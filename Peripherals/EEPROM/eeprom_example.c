/**
 * @file    eeprom_example.c
 * @brief   EEPROM Driver Usage Examples
 * @details Demonstrates how to use the EEPROM driver
 * @version 1.0
 * @date    2026-01-03
 */

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "stm32f4xx_hal.h"

/* Private variables ---------------------------------------------------------*/
static EEPROM_HandleTypeDef eepromHandle;

/* Example 1: Basic Initialization and Read/Write
 * ===============================================
 */
void EEPROM_Example_Basic(void)
{
    EEPROM_StatusTypeDef status;
    uint8_t writeData = 0x42;
    uint8_t readData = 0;

    /* Initialize EEPROM with default M24LR64 configuration */
    status = EEPROM_Init(&eepromHandle);
    if (status != EEPROM_OK) {
        /* Handle initialization error */
        return;
    }

    /* Write a single byte */
    status = EEPROM_WriteByte(&eepromHandle, 0x0000, writeData);
    if (status != EEPROM_OK) {
        /* Handle write error */
        return;
    }

    /* Read back the byte */
    status = EEPROM_ReadByte(&eepromHandle, 0x0000, &readData);
    if (status != EEPROM_OK) {
        /* Handle read error */
        return;
    }

    /* Verify data */
    if (readData == writeData) {
        /* Success! */
    }
}

/* Example 2: Initialize with Specific EEPROM Type
 * ================================================
 */
void EEPROM_Example_SpecificType(void)
{
    EEPROM_StatusTypeDef status;

    /* Initialize for AT24C256 (32KB EEPROM) */
    status = EEPROM_InitType(&eepromHandle, EEPROM_TYPE_AT24C256);
    if (status != EEPROM_OK) {
        return;
    }

    /* Now you can use the EEPROM with correct page size and addressing */
}

/* Example 3: Custom EEPROM Configuration
 * ======================================
 */
void EEPROM_Example_CustomConfig(void)
{
    EEPROM_StatusTypeDef status;
    EEPROM_ConfigTypeDef customConfig;

    /* Configure for a custom EEPROM */
    customConfig.i2cAddress = 0x54;         /* Custom I2C address */
    customConfig.i2cAddressAlt = 0x54;      /* Same alternate address */
    customConfig.totalSize = 4096;          /* 4KB */
    customConfig.pageSize = 32;             /* 32-byte pages */
    customConfig.addressSize = 2;           /* 2-byte addressing */
    customConfig.writeTime = 10;            /* 10ms write cycle */

    status = EEPROM_InitCustom(&eepromHandle, &customConfig);
    if (status != EEPROM_OK) {
        return;
    }
}

/* Example 4: Write and Read Multiple Bytes
 * =========================================
 */
void EEPROM_Example_MultipleBytes(void)
{
    EEPROM_StatusTypeDef status;
    uint8_t writeBuffer[] = "Hello EEPROM!";
    uint8_t readBuffer[20] = {0};

    /* Assume EEPROM is already initialized */

    /* Write string to EEPROM (handles page boundaries automatically) */
    status = EEPROM_Write(&eepromHandle, 0x0100, writeBuffer, sizeof(writeBuffer));
    if (status != EEPROM_OK) {
        return;
    }

    /* Read back the data */
    status = EEPROM_Read(&eepromHandle, 0x0100, readBuffer, sizeof(writeBuffer));
    if (status != EEPROM_OK) {
        return;
    }

    /* Verify */
    status = EEPROM_Verify(&eepromHandle, 0x0100, writeBuffer, sizeof(writeBuffer));
    if (status == EEPROM_OK) {
        /* Data matches! */
    }
}

/* Example 5: Store Configuration Structure
 * =========================================
 */
typedef struct {
    uint32_t deviceId;
    float calibrationFactor;
    uint16_t sampleRate;
    uint8_t mode;
    uint8_t checksum;
} DeviceConfig_t;

void EEPROM_Example_StoreConfig(void)
{
    EEPROM_StatusTypeDef status;
    DeviceConfig_t config;
    DeviceConfig_t readConfig;

    /* Prepare configuration */
    config.deviceId = 0x12345678;
    config.calibrationFactor = 1.234f;
    config.sampleRate = 1000;
    config.mode = 3;
    config.checksum = 0xAB;

    /* Write configuration to EEPROM */
    status = EEPROM_Write(&eepromHandle, 0x0000,
                           (uint8_t*)&config, sizeof(DeviceConfig_t));
    if (status != EEPROM_OK) {
        return;
    }

    /* Read configuration back */
    status = EEPROM_Read(&eepromHandle, 0x0000,
                          (uint8_t*)&readConfig, sizeof(DeviceConfig_t));
    if (status != EEPROM_OK) {
        return;
    }
}

/* Example 6: Store Individual Data Types
 * =======================================
 */
void EEPROM_Example_DataTypes(void)
{
    EEPROM_StatusTypeDef status;
    uint16_t wordVal = 0x1234;
    uint32_t dwordVal = 0xDEADBEEF;
    float floatVal = 3.14159f;

    uint16_t readWord;
    uint32_t readDword;
    float readFloat;

    /* Write different data types */
    status = EEPROM_WriteWord(&eepromHandle, 0x0000, wordVal);
    if (status != EEPROM_OK) return;

    status = EEPROM_WriteDWord(&eepromHandle, 0x0002, dwordVal);
    if (status != EEPROM_OK) return;

    status = EEPROM_WriteFloat(&eepromHandle, 0x0006, floatVal);
    if (status != EEPROM_OK) return;

    /* Read back */
    status = EEPROM_ReadWord(&eepromHandle, 0x0000, &readWord);
    if (status != EEPROM_OK) return;

    status = EEPROM_ReadDWord(&eepromHandle, 0x0002, &readDword);
    if (status != EEPROM_OK) return;

    status = EEPROM_ReadFloat(&eepromHandle, 0x0006, &readFloat);
    if (status != EEPROM_OK) return;
}

/* Example 7: EEPROM Test and Diagnostics
 * =======================================
 */
void EEPROM_Example_Test(void)
{
    EEPROM_StatusTypeDef status;
    uint32_t size;
    uint16_t pageSize;

    /* Initialize EEPROM */
    status = EEPROM_Init(&eepromHandle);
    if (status != EEPROM_OK) {
        return;
    }

    /* Get EEPROM information */
    size = EEPROM_GetSize(&eepromHandle);
    pageSize = EEPROM_GetPageSize(&eepromHandle);

    /* Run self-test (uses address 0x1000, will preserve original data) */
    status = EEPROM_Test(&eepromHandle, 0x1000);
    if (status == EEPROM_OK) {
        /* EEPROM test passed */
    } else {
        /* EEPROM test failed */
    }

    /* Check if device is ready */
    if (EEPROM_IsReady(&eepromHandle)) {
        /* Device is ready for operations */
    }
}

/* Example 8: Erase EEPROM Section
 * ================================
 */
void EEPROM_Example_Erase(void)
{
    EEPROM_StatusTypeDef status;

    /* Erase a specific range (100 bytes starting at address 0x0500) */
    status = EEPROM_EraseRange(&eepromHandle, 0x0500, 100);
    if (status != EEPROM_OK) {
        return;
    }

    /* Note: Full erase of large EEPROMs can take several seconds
     * status = EEPROM_Erase(&eepromHandle);
     */
}

/* Example 9: Boot Counter Application
 * =====================================
 */
#define BOOT_COUNT_ADDRESS  0x0000

void EEPROM_Example_BootCounter(void)
{
    EEPROM_StatusTypeDef status;
    uint32_t bootCount = 0;

    /* Initialize EEPROM */
    status = EEPROM_Init(&eepromHandle);
    if (status != EEPROM_OK) {
        return;
    }

    /* Read current boot count */
    status = EEPROM_ReadDWord(&eepromHandle, BOOT_COUNT_ADDRESS, &bootCount);
    if (status != EEPROM_OK) {
        /* First boot or read error - start from 0 */
        bootCount = 0;
    }

    /* Increment and save */
    bootCount++;
    EEPROM_WriteDWord(&eepromHandle, BOOT_COUNT_ADDRESS, bootCount);
}

/* Example 10: Wear Leveling Concept
 * ==================================
 */
#define LOG_ENTRY_SIZE      16
#define LOG_START_ADDRESS   0x0100
#define LOG_MAX_ENTRIES     100

typedef struct {
    uint32_t timestamp;
    uint16_t sensorValue;
    uint16_t status;
    uint32_t reserved;
    uint32_t sequence;
} LogEntry_t;

static uint16_t currentLogIndex = 0;

void EEPROM_Example_DataLogger(void)
{
    LogEntry_t entry;
    uint16_t address;

    /* Prepare log entry */
    entry.timestamp = HAL_GetTick();
    entry.sensorValue = 1234;  /* Example sensor reading */
    entry.status = 0;
    entry.reserved = 0;
    entry.sequence = currentLogIndex;

    /* Calculate address (circular buffer) */
    address = LOG_START_ADDRESS + (currentLogIndex % LOG_MAX_ENTRIES) * LOG_ENTRY_SIZE;

    /* Write log entry */
    EEPROM_Write(&eepromHandle, address, (uint8_t*)&entry, sizeof(LogEntry_t));

    /* Move to next index */
    currentLogIndex++;
}
