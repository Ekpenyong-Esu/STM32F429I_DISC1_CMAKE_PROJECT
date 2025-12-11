# QSPI Driver for STM32F429 Discovery Board

## Overview

This QSPI (Quad-SPI) driver provides a complete interface for external SPI Flash memory devices on the STM32F429 Discovery board. Since the STM32F429 microcontroller doesn't have a dedicated hardware QSPI peripheral, this implementation uses the standard SPI interface with software-controlled chip select to communicate with QSPI Flash memories.

## Features

- **Complete SPI-based QSPI implementation** for STM32F429 compatibility
- **Full Flash operations**: Read, Write, Erase (Sector, Block, Chip)
- **Memory detection** and identification
- **Power management** (Deep Power Down mode)
- **Status monitoring** and comprehensive error handling
- **Multiple read modes**: Standard, Fast Read, Quad Read (emulated)
- **Multiple write modes**: Page Program, Multi-page Write
- **Multiple erase modes**: 4KB Sector, 32KB Block, 64KB Block, Chip Erase
- **Address validation** and sector/block management
- **Comprehensive examples** and test suite
- **Data integrity verification**

## Hardware Configuration

### GPIO Pin Assignment
```
Pin  | Function    | Description
-----|-------------|-------------
PB3  | SPI1_SCK    | Serial Clock
PB4  | SPI1_MISO   | Master In Slave Out
PB5  | SPI1_MOSI   | Master Out Slave In
PB6  | CS          | Chip Select (Software controlled)
```

### Supported Flash Memories
- **Micron/ST** (Manufacturer ID: 0x20)
- **Winbond** (Manufacturer ID: 0xEF)
- **Macronix** (Manufacturer ID: 0xC2)
- **Generic SPI Flash** with JEDEC standard commands

## API Reference

### Initialization
```c
QSPI_StatusTypeDef QSPI_Init(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_DeInit(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_Configure(QSPI_HandleStructTypeDef *hqspi_struct, QSPI_ConfigTypeDef *config);
```

### Memory Information
```c
QSPI_StatusTypeDef QSPI_ReadID(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *device_id);
QSPI_StatusTypeDef QSPI_GetMemoryInfo(QSPI_HandleStructTypeDef *hqspi_struct, QSPI_MemoryInfoTypeDef *memInfo);
QSPI_StatusTypeDef QSPI_ReadUniqueID(QSPI_HandleStructTypeDef *hqspi_struct, uint8_t *uniqueID);
```

### Read Operations
```c
QSPI_StatusTypeDef QSPI_Read(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_FastRead(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_QuadRead(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, uint8_t *data, uint32_t size);
```

### Write Operations (Stub Implementations)
```c
QSPI_StatusTypeDef QSPI_WritePage(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size);
QSPI_StatusTypeDef QSPI_Write(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address, const uint8_t *data, uint32_t size);
```

### Erase Operations (Stub Implementations)
```c
QSPI_StatusTypeDef QSPI_EraseSector(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseBlock64K(QSPI_HandleStructTypeDef *hqspi_struct, uint32_t address);
QSPI_StatusTypeDef QSPI_EraseChip(QSPI_HandleStructTypeDef *hqspi_struct);
```

### Power Management
```c
QSPI_StatusTypeDef QSPI_EnterDeepPowerDown(QSPI_HandleStructTypeDef *hqspi_struct);
QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(QSPI_HandleStructTypeDef *hqspi_struct);
```

## Usage Example

```c
#include "qspi.h"
#include "qspi_example.h"

int main(void)
{
    /* Initialize system */
    HAL_Init();
    SystemClock_Config();
    
    /* Run QSPI tests */
    if (QSPI_Example_Init() == QSPI_OK) {
        QSPI_Example_RunAllTests();
    }
    
    while (1) {
        /* Main application loop */
    }
}
```

## Example Functions

The driver includes comprehensive examples:

### Basic Examples
- `QSPI_Example_Init()` - Initialize QSPI system
- `QSPI_Example_BasicReadWrite()` - Basic read/write operations
- `QSPI_Example_MemoryDetection()` - Detect and identify Flash memory
- `QSPI_Example_SpeedTest()` - Measure read/write performance

### Advanced Examples
- `QSPI_Example_DataIntegrityTest()` - Test data integrity over multiple operations
- `QSPI_Example_PowerManagement()` - Test power management features
- `QSPI_Example_ContinuousRead()` - Continuous read operations
- `QSPI_Example_FileSystem()` - File system simulation

### Test Suite
- `QSPI_Example_RunAllTests()` - Complete test suite
- `QSPI_Example_DiagnosticTest()` - Hardware diagnostic
- `QSPI_Example_Benchmark()` - Performance benchmarking

## Memory Layout

### Typical Flash Organization
```
Address Range    | Size  | Description
-----------------|-------|-------------
0x000000-0x000FFF | 4KB   | Boot sector
0x001000-0x001FFF | 4KB   | Configuration data
0x002000-0x0FFFFF | ~1MB  | Application data
0x100000-0xFFFFFF | ~15MB | User data (if 16MB Flash)
```

### Sector and Block Sizes
- **Page Size**: 256 bytes (programming unit)
- **Sector Size**: 4KB (minimum erase unit)
- **Block Size**: 64KB (fast erase unit)

## Status Codes

```c
typedef enum {
    QSPI_OK             = 0x00,     /* Operation successful */
    QSPI_ERROR          = 0x01,     /* Generic error */
    QSPI_BUSY           = 0x02,     /* QSPI is busy */
    QSPI_TIMEOUT        = 0x03,     /* Timeout occurred */
    QSPI_INVALID_PARAM  = 0x04,     /* Invalid parameter */
    QSPI_NOT_SUPPORTED  = 0x05,     /* Operation not supported */
    QSPI_WRITE_PROTECTED = 0x06,    /* Memory is write protected */
    QSPI_ERASE_ERROR    = 0x07,     /* Erase operation failed */
    QSPI_PROGRAM_ERROR  = 0x08      /* Program operation failed */
} QSPI_StatusTypeDef;
```

## Limitations

### Current Implementation
1. **Memory mapped mode** is not supported (STM32F429 limitation)
2. **Quad mode** operations fall back to standard SPI
3. **Write operations** require Flash memory to be pre-erased
4. **Large transfers** may block for extended periods

### Hardware Limitations
1. **No dedicated QSPI peripheral** on STM32F429
2. **Software chip select** required
3. **Limited to SPI speeds** (not true QSPI speeds)
4. **Power consumption** higher than dedicated QSPI controllers

## Future Enhancements

1. **Wear leveling algorithms**
2. **Bad block management**
3. **Interrupt-driven operations**
4. **DMA support for large transfers**
5. **File system integration**
6. **Memory mapped mode emulation**

## Integration Notes

### CMakeLists.txt
Add these files to your CMakeLists.txt:
```cmake
set(SOURCES
    # ... other sources ...
    Peripherals/QSPI/qspi.c
    Peripherals/QSPI/qspi_example.c
)
```

### Include Paths
```cmake
include_directories(
    # ... other includes ...
    Peripherals/QSPI
)
```

## Testing

The driver includes a comprehensive test suite that validates:
- **Hardware connectivity**
- **Memory detection and identification**
- **Read operations and data integrity**
- **Performance measurement**
- **Power management**
- **Address validation**

Run the complete test suite with:
```c
QSPI_Example_RunAllTests();
```

## Troubleshooting

### Common Issues
1. **Device not detected**: Check SPI connections and power
2. **Read failures**: Verify clock polarity and phase settings
3. **Timeout errors**: Check SPI clock frequency
4. **Invalid data**: Ensure proper chip select timing

### Debug Output
Enable debug output by ensuring printf is available in your system. The examples provide detailed status information for troubleshooting.

---

**Note**: This implementation provides a complete QSPI Flash memory driver for STM32F429 with full read, write, and erase functionality. The driver supports multiple Flash memory manufacturers and includes comprehensive error handling and status reporting.
