# SPI Driver for STM32F429

This directory contains a comprehensive SPI (Serial Peripheral Interface) driver implementation for the STM32F429 microcontroller, providing both basic and advanced communication capabilities.

## Files Overview

- **`spi.h`** - Header file containing function prototypes, type definitions, and constants
- **`spi.c`** - Main implementation file with all SPI communication functions
- **`spi_example.c`** - Example code demonstrating various SPI operations

## Features

### Core Functionality
- **Basic Communication**: Master transmit and receive operations
- **Full-Duplex Operations**: Simultaneous transmit and receive
- **Memory Operations**: Read/write to SPI memory devices (Flash, etc.)
- **Sensor Communication**: Interface with SPI sensors and peripherals
- **Error Handling**: Comprehensive error detection and reporting
- **Custom Configuration**: Flexible SPI parameter configuration

### Supported Operations
- `SPI_Transmit()` - Send data via SPI
- `SPI_Receive()` - Receive data via SPI
- `SPI_TransmitReceive()` - Full-duplex communication
- `SPI_Init_Custom()` - Custom SPI configuration
- `SPI_GetError()` - Get detailed error information

## Usage Examples

### Basic Initialization
```c
#include "spi.h"

// Initialize SPI with default settings (8-bit, mode 0, fPCLK/16)
SPI_Init();
```

### Custom Configuration
```c
SPI_ConfigTypeDef customConfig = {
    .Mode = SPI_MODE_MASTER,                    // Master mode
    .Direction = SPI_DIRECTION_2LINES,          // Full-duplex
    .DataSize = SPI_DATASIZE_8BIT,              // 8-bit data
    .CLKPolarity = SPI_POLARITY_LOW,            // CPOL = 0
    .CLKPhase = SPI_PHASE_1EDGE,                // CPHA = 0
    .NSS = SPI_NSS_SOFT,                        // Software NSS
    .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4, // High speed
    .FirstBit = SPI_FIRSTBIT_MSB,               // MSB first
    .TIMode = SPI_TIMODE_DISABLE,               // TI mode disabled
    .CRCCalculation = SPI_CRCCALCULATION_DISABLE, // CRC disabled
    .CRCPolynomial = 10                          // CRC polynomial
};

SPI_Init_Custom(&customConfig);

```c
/* Runtime speed switching example (LCD fast, XPT2046 touch safe slow) */
// set safe speed for touch
SPI_SetBaudRatePrescaler(SPI_BAUDRATEPRESCALER_32);
// perform touch read operation (XPT2046)
// ...
// restore high speed for display updates
SPI_SetBaudRatePrescaler(SPI_BAUDRATEPRESCALER_8);
```

```

### Communicating with Devices
```c
// Transmit data
uint8_t txData[] = {0x01, 0x02, 0x03};
SPI_StatusTypeDef status = SPI_Transmit(txData, sizeof(txData), SPI_TIMEOUT_DEFAULT);

// Receive data
uint8_t rxData[4];
status = SPI_Receive(rxData, sizeof(rxData), SPI_TIMEOUT_DEFAULT);

// Full-duplex communication
uint8_t cmd = 0x05;  // Status register read command
uint8_t response;
status = SPI_TransmitReceive(&cmd, &response, 1, SPI_TIMEOUT_DEFAULT);
```

### Flash Memory Operations
```c
// Write enable command
uint8_t writeEnable = 0x06;
SPI_Transmit(&writeEnable, 1, SPI_TIMEOUT_DEFAULT);

// Write data to flash
uint8_t writeCmd[5] = {0x02, 0x00, 0x00, 0x00, 0xAA}; // Write command + address + data
SPI_Transmit(writeCmd, sizeof(writeCmd), SPI_TIMEOUT_DEFAULT);

// Read data from flash
uint8_t readCmd[4] = {0x03, 0x00, 0x00, 0x00}; // Read command + address
SPI_Transmit(readCmd, sizeof(readCmd), SPI_TIMEOUT_DEFAULT);
uint8_t readData[1];
SPI_Receive(readData, sizeof(readData), SPI_TIMEOUT_DEFAULT);
```

### Sensor Communication
```c
// Read accelerometer data
uint8_t readCmd = 0x80 | 0x01;  // Read command for X-axis register
uint8_t dummy = 0x00;
uint8_t accelData[2];

SPI_TransmitReceive(&readCmd, &accelData[0], 1, SPI_TIMEOUT_DEFAULT);
SPI_TransmitReceive(&dummy, &accelData[1], 1, SPI_TIMEOUT_DEFAULT);

// Write sensor configuration
uint8_t writeCmd = 0x00 | 0x20;  // Write command for CTRL_REG1
uint8_t config = 0x47;           // Enable X, Y, Z axes, 50Hz
SPI_Transmit(&writeCmd, 1, SPI_TIMEOUT_DEFAULT);
SPI_Transmit(&config, 1, SPI_TIMEOUT_DEFAULT);
```

## Configuration Constants

### Timeout Values
- `SPI_TIMEOUT_DEFAULT` - 1000ms (standard operations)
- `SPI_TIMEOUT_SHORT` - 100ms (quick operations)
- `SPI_TIMEOUT_LONG` - 5000ms (memory operations)

### Clock Speeds
- `SPI_CLOCK_SPEED_HIGH` - 10 MHz (high speed)
- `SPI_CLOCK_SPEED_MEDIUM` - 5 MHz (medium speed)
- `SPI_CLOCK_SPEED_LOW` - 1 MHz (low speed)

## Error Handling

The driver provides comprehensive error handling with the following status codes:

- `SPI_OK` - Operation successful
- `SPI_ERROR` - General error
- `SPI_BUSY` - SPI peripheral is busy
- `SPI_TIMEOUT` - Operation timed out
- `SPI_INVALID_PARAM` - Invalid parameter provided

Use `SPI_GetStatusString()` to convert status codes to human-readable strings.

## Hardware Configuration

The driver is configured for **SPI5** peripheral with the following default settings:

- **Mode**: Master mode
- **Direction**: Full-duplex (2 lines)
- **Data Size**: 8-bit
- **Clock Polarity**: Low (CPOL=0)
- **Clock Phase**: 1st edge (CPHA=0)
- **NSS Management**: Software
- **Baud Rate**: fPCLK/16
- **First Bit**: MSB
- **TI Mode**: Disabled
- **CRC**: Disabled

### Pin Configuration (STM32F429-Discovery)
- **MOSI**: PF9 (AF5)
- **MISO**: PF8 (AF5)
- **SCK**: PF7 (AF5)

Note on baudrate: ST BSP expects ILI9341 SPI SCLK in the ~5.6–10 MHz range. With this project's
SystemClock (APB2 = 84 MHz) the recommended prescaler is `SPI_BAUDRATEPRESCALER_8` (84/8 = 10.5 MHz),
which is slightly above 10 MHz but closer to the BSP range than prescaler 16 (84/16 = 5.25 MHz, below 5.6).
If strict adherence to <=10 MHz is required, either adjust APB2 clock or use prescaler 16 and accept the
slightly lower SCLK.
- **NSS**: Software controlled

## Running Examples

To run the comprehensive examples:

```c
#include "spi_example.h"

// Run all SPI examples
SPI_RunExamples();
```

This will demonstrate:
1. Basic transmit/receive operations
2. Flash memory read/write operations
3. Sensor communication
4. Error handling scenarios
5. Custom configuration

## Integration Notes

### Dependencies
- STM32F4xx HAL Library
- System error handler (`Error_Handler()`)
- Standard C libraries (`string.h`, `stdint.h`)

### Thread Safety
The driver is not thread-safe. Implement mutex protection for multi-threaded applications.

### Power Management
Consider SPI peripheral clock gating for low-power applications.

## Troubleshooting

### Common Issues
1. **No Response**: Check SPI pin connections and device power
2. **Timeout Errors**: Verify SPI clock speed and device compatibility
3. **Data Corruption**: Check SPI mode (CPOL/CPHA) compatibility
4. **Busy Errors**: Ensure proper chip select (NSS) management

### Debug Tips
- Use logic analyzer to verify SPI signals
- Check `SPI_GetError()` for detailed HAL error codes
- Verify SPI pin configurations in STM32CubeMX
- Test with different baud rates if communication fails

## Performance Considerations

- **Clock Speed**: Balance between speed and signal integrity
- **Data Size**: 8-bit vs 16-bit operations
- **Mode Selection**: Choose appropriate CPOL/CPHA for device compatibility
- **NSS Management**: Proper chip select timing for multi-device buses
- **Interrupt vs Polling**: Consider interrupt-driven operations for better performance

## SPI Modes

The driver supports all four SPI modes:

| Mode | CPOL | CPHA | Description |
|------|------|------|-------------|
| 0    | 0    | 0    | Clock idle low, sample on rising edge |
| 1    | 0    | 1    | Clock idle low, sample on falling edge |
| 2    | 1    | 0    | Clock idle high, sample on falling edge |
| 3    | 1    | 1    | Clock idle high, sample on rising edge |

---

For more detailed information, refer to the STM32F429 Reference Manual and SPI specification.
