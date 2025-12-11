# CRC Peripheral Driver

This directory contains the CRC (Cyclic Redundancy Check) peripheral driver for STM32F429 microcontrollers. The driver provides a comprehensive interface for data integrity verification using both hardware and software CRC calculations.

## Features

- **Hardware CRC Support**: Utilizes STM32F429's built-in CRC hardware accelerator for high performance
- **Software CRC Fallback**: Software implementation supporting various CRC standards
- **Multiple CRC Standards**: Support for CRC-32, CRC-32C, CRC-16, CRC-16-CCITT, and CRC-8
- **Flexible Configuration**: Configurable polynomials, initial values, and data formats
- **Data Accumulation**: Process large data sets in chunks
- **Callback Support**: Register callbacks for completion and error events
- **Status Monitoring**: Comprehensive status tracking and error reporting
- **Input/Output Inversion**: Support for bit reflection on input/output data

## Files

- `crc.h` - Header file with function prototypes and data structures
- `crc.c` - Main implementation file
- `crc_example.c` - Usage examples and test cases
- `README.md` - This documentation file

## API Overview

### Initialization and Configuration

```c
// Get default configuration
CRC_Config config;
CRC_GetDefaultConfig(&config);

// Initialize CRC
HAL_StatusTypeDef status = CRC_Init(&config);
```

### Basic CRC Calculation

```c
// Calculate CRC for byte data
uint32_t crc_value;
uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
status = CRC_Calculate(data, sizeof(data), &crc_value);

// Calculate CRC for 32-bit word data
uint32_t data32[] = {0x11223344, 0x55667788};
status = CRC_Calculate32(data32, 2, &crc_value);
```

### Advanced Features

```c
// Data accumulation
uint32_t crc = 0;
CRC_Accumulate(chunk1, size1, &crc);
CRC_Accumulate(chunk2, size2, &crc);

// Reset CRC state
CRC_Reset();

// Get status information
CRC_Status status_info;
CRC_GetStatus(&status_info);
```

### Custom Configuration

```c
CRC_Config config;
config.method = CRC_METHOD_SOFTWARE;
config.polynomial = CRC_POLY_CRC16_CCITT;
config.init_value = 0xFFFF;
config.input_reverse = false;
config.output_reverse = false;
config.xor_output = true;
config.xor_value = 0x0000;

CRC_Init(&config);
```

## Configuration Options

### Calculation Methods

- `CRC_METHOD_HARDWARE`: Use STM32 hardware CRC unit (fastest)
- `CRC_METHOD_SOFTWARE`: Software implementation (flexible)

### Supported Polynomials

- `CRC_POLY_CRC32`: Standard CRC-32 (0x04C11DB7)
- `CRC_POLY_CRC32C`: CRC-32C (Castagnoli) (0x1EDC6F41)
- `CRC_POLY_CRC16`: CRC-16 (0x8005)
- `CRC_POLY_CRC16_CCITT`: CRC-16-CCITT (0x1021)
- `CRC_POLY_CRC8`: CRC-8 (0x07)

### Data Formats

- `CRC_FORMAT_8BIT`: 8-bit byte data
- `CRC_FORMAT_16BIT`: 16-bit word data
- `CRC_FORMAT_32BIT`: 32-bit word data

## Usage Examples

The `crc_example.c` file contains comprehensive examples demonstrating:

1. **Basic Initialization**: Setting up CRC with default configuration
2. **Byte Data Calculation**: Computing CRC for byte arrays
3. **32-bit Data Calculation**: Optimized calculation for word-aligned data
4. **Data Accumulation**: Processing large datasets in chunks
5. **Custom Configuration**: Using different CRC standards and parameters
6. **Callback Usage**: Registering completion and error callbacks
7. **Status Monitoring**: Tracking CRC operations and statistics
8. **Reset Functionality**: Resetting CRC calculation state

### Running Examples

```c
// Run all examples
HAL_StatusTypeDef status = CRC_RunAllExamples();
```

## Performance Considerations

- **Hardware CRC**: Fastest method, recommended for most applications
- **Software CRC**: Flexible but slower, use for unsupported polynomials
- **Data Alignment**: 32-bit calculations are most efficient
- **Chunk Size**: Balance memory usage with processing efficiency for accumulation

## Error Handling

The driver provides comprehensive error handling:

- `CRC_ERROR_NONE`: No error
- `CRC_ERROR_INVALID_PARAM`: Invalid function parameters
- `CRC_ERROR_DATA_SIZE`: Data size exceeds limits
- `CRC_ERROR_HARDWARE`: Hardware CRC peripheral error
- `CRC_ERROR_TIMEOUT`: Operation timeout

## Integration

To use this driver in your project:

1. Include the header: `#include "Peripherals/CRC/crc.h"`
2. Initialize the CRC peripheral with desired configuration
3. Use the calculation functions as needed
4. Handle errors appropriately

## Dependencies

- STM32F4xx HAL Driver
- Standard C libraries (stdint.h, stdbool.h, string.h)

## Notes

- Hardware CRC uses fixed polynomial (CRC-32), software implementation supports custom polynomials
- Maximum data size for single calculation is 4096 bytes
- Callbacks are optional but recommended for robust applications
- Status monitoring provides valuable debugging information
