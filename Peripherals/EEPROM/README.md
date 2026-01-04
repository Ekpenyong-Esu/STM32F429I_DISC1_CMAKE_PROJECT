# EEPROM Driver for STM32F429

A comprehensive I2C EEPROM driver supporting M24LR64, M24Cxx series, and AT24Cxx series EEPROMs with handle-based API design.

## Features

- **Handle-Based Design**: Consistent with other peripheral drivers
- **Multiple EEPROM Support**: M24LR64, M24C01-M24C512, AT24C256
- **Custom Configuration**: Support for any I2C EEPROM
- **Page-Aligned Writes**: Automatic handling of page boundaries
- **Data Type Helpers**: Read/write byte, word, dword, and float
- **Utility Functions**: Erase, verify, test, and ready check
- **Robust Error Handling**: Detailed status codes

## Supported EEPROMs

| Type | Size | Page Size | Address Size |
|------|------|-----------|--------------|
| M24LR64 | 8KB (64Kbit) | 4 bytes | 2 bytes |
| M24C01 | 128B (1Kbit) | 16 bytes | 1 byte |
| M24C02 | 256B (2Kbit) | 16 bytes | 1 byte |
| M24C04 | 512B (4Kbit) | 16 bytes | 1 byte |
| M24C08 | 1KB (8Kbit) | 16 bytes | 1 byte |
| M24C16 | 2KB (16Kbit) | 16 bytes | 1 byte |
| M24C32 | 4KB (32Kbit) | 32 bytes | 2 bytes |
| M24C64 | 8KB (64Kbit) | 32 bytes | 2 bytes |
| M24C128 | 16KB (128Kbit) | 64 bytes | 2 bytes |
| M24C256 | 32KB (256Kbit) | 64 bytes | 2 bytes |
| M24C512 | 64KB (512Kbit) | 128 bytes | 2 bytes |
| AT24C256 | 32KB (256Kbit) | 64 bytes | 2 bytes |

## Hardware Configuration

### STM32F429 Discovery Board (M24LR64)

The board includes an M24LR64 EEPROM connected via I2C3:

| Signal | Pin | Description |
|--------|-----|-------------|
| SCL | PC0 | I2C3 Clock |
| SDA | PC1 | I2C3 Data |

### I2C Address

- **M24LR64 Primary**: 0x50 (7-bit) / 0xA0 (8-bit)
- **M24LR64 Alternate**: 0x53 (7-bit) / 0xA6 (8-bit)

## Quick Start

### 1. Basic Usage (M24LR64)

```c
#include "eeprom.h"

EEPROM_HandleTypeDef eeprom;

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* Initialize EEPROM (uses default M24LR64 configuration) */
    if (EEPROM_Init(&eeprom) != EEPROM_OK) {
        /* Handle error */
        Error_Handler();
    }

    /* Write data */
    uint8_t data[] = "Hello!";
    EEPROM_Write(&eeprom, 0x0000, data, sizeof(data));

    /* Read data */
    uint8_t buffer[10];
    EEPROM_Read(&eeprom, 0x0000, buffer, sizeof(data));

    while (1) {
        /* Main loop */
    }
}
```

### 2. Using Specific EEPROM Type

```c
EEPROM_HandleTypeDef eeprom;

/* Initialize for AT24C256 */
EEPROM_InitType(&eeprom, EEPROM_TYPE_AT24C256);
```

### 3. Custom Configuration

```c
EEPROM_HandleTypeDef eeprom;
EEPROM_ConfigTypeDef config = {
    .i2cAddress = 0x54,
    .i2cAddressAlt = 0x54,
    .totalSize = 4096,
    .pageSize = 32,
    .addressSize = 2,
    .writeTime = 5
};

EEPROM_InitCustom(&eeprom, &config);
```

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `EEPROM_Init()` | Initialize with default M24LR64 config |
| `EEPROM_InitType()` | Initialize with specific EEPROM type |
| `EEPROM_InitCustom()` | Initialize with custom configuration |
| `EEPROM_DeInit()` | Deinitialize EEPROM |

### Read Functions

| Function | Description |
|----------|-------------|
| `EEPROM_ReadByte()` | Read single byte |
| `EEPROM_Read()` | Read multiple bytes |
| `EEPROM_ReadWord()` | Read 16-bit value |
| `EEPROM_ReadDWord()` | Read 32-bit value |
| `EEPROM_ReadFloat()` | Read float value |

### Write Functions

| Function | Description |
|----------|-------------|
| `EEPROM_WriteByte()` | Write single byte |
| `EEPROM_Write()` | Write multiple bytes (handles pages) |
| `EEPROM_WritePage()` | Write single page |
| `EEPROM_WriteWord()` | Write 16-bit value |
| `EEPROM_WriteDWord()` | Write 32-bit value |
| `EEPROM_WriteFloat()` | Write float value |

### Utility Functions

| Function | Description |
|----------|-------------|
| `EEPROM_WaitReady()` | Wait for write cycle completion |
| `EEPROM_IsReady()` | Check if device is ready |
| `EEPROM_Erase()` | Erase entire EEPROM |
| `EEPROM_EraseRange()` | Erase memory range |
| `EEPROM_GetSize()` | Get total size |
| `EEPROM_GetPageSize()` | Get page size |
| `EEPROM_Verify()` | Verify data integrity |
| `EEPROM_Test()` | Self-test function |

## Status Codes

| Status | Description |
|--------|-------------|
| `EEPROM_OK` | Operation successful |
| `EEPROM_ERROR` | General error |
| `EEPROM_TIMEOUT` | Operation timed out |
| `EEPROM_BUSY` | Device busy (write in progress) |
| `EEPROM_INVALID_PARAM` | Invalid parameter |
| `EEPROM_INVALID_ADDRESS` | Address out of range |
| `EEPROM_NOT_INITIALIZED` | Driver not initialized |

## Usage Examples

### Store Configuration Structure

```c
typedef struct {
    uint32_t deviceId;
    float calibration;
    uint16_t settings;
} Config_t;

Config_t config = {0x12345678, 1.5f, 0x0001};

/* Write config */
EEPROM_Write(&eeprom, 0x0000, (uint8_t*)&config, sizeof(config));

/* Read config */
Config_t readConfig;
EEPROM_Read(&eeprom, 0x0000, (uint8_t*)&readConfig, sizeof(readConfig));
```

### Boot Counter

```c
uint32_t bootCount;
EEPROM_ReadDWord(&eeprom, 0x0000, &bootCount);
bootCount++;
EEPROM_WriteDWord(&eeprom, 0x0000, bootCount);
```

### Data Logger

```c
#define LOG_ADDRESS  0x0100
#define LOG_SIZE     16

for (int i = 0; i < numEntries; i++) {
    EEPROM_Write(&eeprom, LOG_ADDRESS + (i * LOG_SIZE), 
                 &logData[i], LOG_SIZE);
}
```

## Important Notes

### Page Boundary Handling

The `EEPROM_Write()` function automatically handles page boundaries. For manual page writes, ensure data doesn't cross page boundaries.

### Write Cycle Time

After each write operation, the EEPROM needs time to complete the internal write cycle (typically 5ms). The driver handles this automatically with `EEPROM_WaitReady()`.

### Endurance

Typical EEPROM endurance is 1 million write cycles per byte. For frequently updated data, consider wear leveling.

### Data Alignment

Multi-byte values (word, dword, float) are stored in big-endian format for portability.

## Dependencies

- STM32F4xx HAL Driver
- I2C Peripheral Driver (`../I2C/i2c.h`)

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Init fails | Check I2C connections and pull-ups |
| Read returns wrong data | Verify address and EEPROM type |
| Write fails | Check write protection and wait for ready |
| Timeout errors | Increase timeout or check I2C bus |

## License

MIT License - see LICENSE file for details.
