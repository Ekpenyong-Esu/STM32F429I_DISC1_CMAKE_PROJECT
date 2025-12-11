# FMC (Flexible Memory Controller) Driver

This directory contains the FMC driver implementation for STM32F429 microcontroller, following STM32 HAL library best practices. The FMC peripheral enables interfacing with external memories including SDRAM, NOR Flash, and NAND Flash.

## Files

- `fmc.h` - Header file with function prototypes and data structures
- `fmc.c` - Main implementation with HAL-compliant functions  
- `fmc_example.h` - Example usage header file
- `fmc_example.c` - Example usage and test functions

## Key Features

- **HAL Best Practices Compliance**: All functions follow STM32 HAL naming conventions and patterns
- **Conflict-Free API**: Functions prefixed with `FMC_Driver_` to avoid conflicts with existing HAL functions
- **Comprehensive Error Handling**: Proper error codes and status returns
- **Handle-Based Design**: Uses handle structures for state management
- **Memory Type Support**: SDRAM, NOR Flash, and NAND Flash interfaces

## API Functions

### Initialization Functions
- `FMC_Driver_SDRAM_Init()` - Initialize SDRAM interface
- `FMC_Driver_NOR_Init()` - Initialize NOR Flash interface  
- `FMC_Driver_NAND_Init()` - Initialize NAND Flash interface
- `FMC_Driver_DeInit()` - Deinitialize FMC driver

### SDRAM Functions
- `FMC_Driver_SDRAM_Write()` - Write data to SDRAM
- `FMC_Driver_SDRAM_Read()` - Read data from SDRAM
- `FMC_Driver_SDRAM_Test()` - Test SDRAM functionality

### NOR Flash Functions
- `FMC_Driver_NOR_Write()` - Write data to NOR Flash
- `FMC_Driver_NOR_Read()` - Read data from NOR Flash
- `FMC_Driver_NOR_EraseSector()` - Erase NOR Flash sector

### NAND Flash Functions
- `FMC_Driver_NAND_Write()` - Write data to NAND Flash
- `FMC_Driver_NAND_Read()` - Read data from NAND Flash
- `FMC_Driver_NAND_EraseBlock()` - Erase NAND Flash block

## Usage Example

```c
#include "fmc.h"

FMC_Driver_Handle_t fmcHandle;
FMC_Driver_SDRAM_Config_t sdramConfig = {
    .bank = FMC_SDRAM_BANK1,
    .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
    .rowBits = FMC_SDRAM_ROW_BITS_NUM_11,
    .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
    // ... other configuration parameters
};

// Initialize SDRAM
HAL_StatusTypeDef status = FMC_Driver_SDRAM_Init(&fmcHandle, &sdramConfig);
if (status == HAL_OK) {
    // Use SDRAM functions
    uint8_t data[] = "Hello SDRAM!";
    FMC_Driver_SDRAM_Write(&fmcHandle, 0xC0000000, data, sizeof(data));
}

// Cleanup
FMC_Driver_DeInit(&fmcHandle);
```

## Configuration Structures

### FMC_Driver_SDRAM_Config_t
Contains SDRAM-specific configuration including bank selection, timing parameters, and memory organization.

### FMC_Driver_NOR_Config_t  
Contains NOR Flash configuration including timing, access modes, and bus parameters.

### FMC_Driver_NAND_Config_t
Contains NAND Flash configuration including ECC settings, timing, and bank selection.

## Hardware Configuration

The FMC driver requires proper GPIO and clock configuration:

1. **GPIO Configuration**: Configure FMC pins (address, data, control signals)
2. **Clock Enable**: Enable FMC peripheral clock
3. **Memory-Specific Setup**: Configure external memory chips according to their specifications

## Best Practices Implemented

1. **Naming Convention**: All public functions use `FMC_Driver_` prefix to avoid HAL conflicts
2. **Error Handling**: Comprehensive error checking and status reporting
3. **Handle Validation**: Input parameter validation in all functions
4. **Private Functions**: Internal helper functions are properly declared as private
5. **HAL Integration**: Uses existing HAL functions where appropriate
6. **Documentation**: Complete function and structure documentation

## Memory Addresses

- **SDRAM**: Base address 0xC0000000 (Bank 1)
- **NOR Flash**: Base address 0x60000000 (Bank 1) 
- **NAND Flash**: Base address 0x70000000 (Bank 2)

## Error Handling

The driver provides comprehensive error handling:

- **HAL_OK**: Operation successful
- **HAL_ERROR**: General error
- **HAL_BUSY**: Peripheral busy
- **HAL_TIMEOUT**: Operation timeout
- **FMC_DRIVER_ERROR_INVALID_PARAM**: Invalid parameter
- **FMC_DRIVER_ERROR_CONFIG**: Configuration error
- **FMC_DRIVER_ERROR_OPERATION**: Operation error

## Dependencies

- STM32F4xx HAL Library
- CMSIS Core headers
- Standard C library (stdio.h for examples)

## Notes

- This driver is specifically designed for STM32F429 but can be adapted for other STM32F4 series
- External memory initialization requires proper hardware setup and configuration
- Test functions are provided for validation but may need adjustment based on specific memory chips
- The driver supports multiple memory types but only one type can be active at a time per handle

## License

This driver is provided as-is for educational and development purposes.
