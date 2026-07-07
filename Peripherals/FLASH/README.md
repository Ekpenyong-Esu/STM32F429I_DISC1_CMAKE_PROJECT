# FLASH - Internal Flash Memory Driver

## Overview

The FLASH driver provides read, write, and erase operations for the STM32F429's internal Flash memory. It's useful for storing configuration data, calibration values, or other persistent data.

## Features

- Sector erase operations
- Byte, half-word, word, and double-word programming
- Buffer read/write operations
- Sector information utilities
- Address validation

## STM32F429 Flash Memory Map

| Sector | Address | Size | Notes |
|--------|---------|------|-------|
| 0-3 | 0x08000000 - 0x0800FFFF | 16KB each | Boot/Code |
| 4 | 0x08010000 - 0x0801FFFF | 64KB | Code |
| 5-11 | 0x08020000 - 0x080FFFFF | 128KB each | Code/Data |
| 12-23 | 0x08100000 - 0x081FFFFF | Same as Bank 1 | Bank 2 |

**Total: 2MB (1MB per bank)**

## User Data Storage

By default, **Sector 11** (0x080E0000 - 0x080FFFFF, 128KB) is designated for user data storage. This keeps user data separate from application code.

## Quick Start

### Write and Read Data

```c
#include "flash.h"

/* Erase sector before writing (required) */
FLASH_EraseUserSector();

/* Write a word */
FLASH_WriteWord(FLASH_USER_START_ADDRESS, 0x12345678);

/* Read it back */
uint32_t value = FLASH_ReadWord(FLASH_USER_START_ADDRESS);
```

### Store Configuration

```c
typedef struct {
    uint32_t magic;
    uint32_t version;
    char deviceName[32];
    uint32_t baudRate;
} Config_t;

Config_t config = {
    .magic = 0xDEADBEEF,
    .version = 1,
    .deviceName = "MyDevice",
    .baudRate = 115200
};

/* Erase and write */
FLASH_EraseUserSector();
FLASH_WriteBuffer(FLASH_USER_START_ADDRESS, (uint8_t*)&config, sizeof(config));

/* Read back */
Config_t readConfig;
FLASH_ReadBuffer(FLASH_USER_START_ADDRESS, (uint8_t*)&readConfig, sizeof(readConfig));
```

## API Reference

### Lock/Unlock Functions

| Function | Description |
|----------|-------------|
| `FLASH_Unlock()` | Unlock Flash for write/erase |
| `FLASH_Lock()` | Lock Flash after operations |

### Erase Functions

| Function | Description |
|----------|-------------|
| `FLASH_EraseSector(sector)` | Erase single sector |
| `FLASH_EraseSectors(start, end)` | Erase range of sectors |
| `FLASH_EraseUserSector()` | Erase user data sector |

### Write Functions

| Function | Description |
|----------|-------------|
| `FLASH_WriteByte(addr, data)` | Write 8-bit value |
| `FLASH_WriteHalfWord(addr, data)` | Write 16-bit value |
| `FLASH_WriteWord(addr, data)` | Write 32-bit value |
| `FLASH_WriteDoubleWord(addr, data)` | Write 64-bit value |
| `FLASH_WriteBuffer(addr, data, len)` | Write byte array |
| `FLASH_WriteBuffer32(addr, data, cnt)` | Write word array |

### Read Functions

| Function | Description |
|----------|-------------|
| `FLASH_ReadByte(addr)` | Read 8-bit value |
| `FLASH_ReadHalfWord(addr)` | Read 16-bit value |
| `FLASH_ReadWord(addr)` | Read 32-bit value |
| `FLASH_ReadBuffer(addr, buf, len)` | Read byte array |

### Utility Functions

| Function | Description |
|----------|-------------|
| `FLASH_GetSector(addr)` | Get sector number from address |
| `FLASH_GetSectorInfo(sector, info)` | Get sector details |
| `FLASH_IsValidAddress(addr)` | Validate Flash address |
| `FLASH_IsErased(addr, len)` | Check if region is erased |

## Important Notes

### 1. Erase Before Write
Flash memory can only change bits from 1 to 0. To write new data, you **must erase first** (sets all bits to 1).

```c
/* WRONG - may fail or corrupt data */
FLASH_WriteWord(addr, newValue);

/* CORRECT */
FLASH_EraseSector(sector);
FLASH_WriteWord(addr, newValue);
```

### 2. Sector Size Matters
Erasing affects entire sectors. Plan your data layout to minimize erases.

### 3. Alignment Requirements
- Half-word: 2-byte aligned
- Word: 4-byte aligned
- Double-word: 8-byte aligned

### 4. Write Endurance
Flash has limited write cycles (~10,000). For frequently-changing data, consider:
- Wear leveling
- Using multiple addresses
- External EEPROM

## Configuration Storage Pattern

```c
#define CONFIG_MAGIC 0xCAFEBABE

typedef struct {
    uint32_t magic;
    uint32_t checksum;
    /* your data */
} StoredData_t;

bool LoadConfig(StoredData_t* config)
{
    FLASH_ReadBuffer(FLASH_USER_START_ADDRESS, (uint8_t*)config, sizeof(*config));

    if (config->magic != CONFIG_MAGIC) {
        return false;  /* No valid config */
    }

    /* Validate checksum */
    return ValidateChecksum(config);
}

void SaveConfig(const StoredData_t* config)
{
    FLASH_EraseUserSector();
    FLASH_WriteBuffer(FLASH_USER_START_ADDRESS, (uint8_t*)config, sizeof(*config));
}
```

## Status Codes

| Status | Description |
|--------|-------------|
| `FLASH_OK` | Success |
| `FLASH_ERROR` | General error |
| `FLASH_ERROR_PROGRAM` | Programming failed |
| `FLASH_ERROR_WRITE_PROTECTED` | Sector is protected |
| `FLASH_INVALID_ADDRESS` | Address out of range |
| `FLASH_BUSY` | Operation in progress |

## Safety Tips

1. **Don't erase code sectors** - Sectors 0-10 typically contain your application
2. **Use magic numbers** - Detect uninitialized/corrupt data
3. **Add checksums** - Verify data integrity
4. **Plan for defaults** - Handle case when no valid data exists
5. **Test thoroughly** - Flash corruption can brick the device
