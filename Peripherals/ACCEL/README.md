# MMA8452Q Accelerometer Driver for STM32F429

This directory contains a comprehensive MMA8452Q accelerometer driver implementation for the STM32F429 microcontroller, providing both basic and advanced motion sensing capabilities.

## Files Overview

- **`accel.h`** - Header file containing function prototypes, type definitions, and register definitions
- **`accel.c`** - Main implementation file with all accelerometer communication functions
- **`accel_example.c`** - Example code demonstrating various accelerometer operations

## Features

### Core Functionality
- **Motion Sensing**: 3-axis acceleration measurement (±2g, ±4g, ±8g ranges)
- **Multiple Data Rates**: 1.56Hz to 800Hz output data rates
- **Raw & Processed Data**: Both raw ADC values and g-force conversion
- **Device Management**: Device identification, status checking, and mode control
- **Calibration**: Automatic and manual offset calibration
- **Interrupt System**: Configurable interrupts for data ready, motion, freefall, tap detection
- **Self-Test**: Built-in self-test functionality

### Supported Operations
- `ACCEL_Init()` - Initialize with default settings
- `ACCEL_ReadData()` - Read processed acceleration data
- `ACCEL_ReadRawData()` - Read raw 14-bit acceleration data
- `ACCEL_SetDataRate()` - Configure output data rate
- `ACCEL_SetRange()` - Set measurement range
- `ACCEL_Calibrate()` - Perform automatic calibration
- `ACCEL_SelfTest()` - Run built-in self-test

## Usage Examples

### Basic Initialization and Reading
```c
#include "accel.h"

// Initialize accelerometer with default settings (100Hz, ±2g, active mode)
ACCEL_StatusTypeDef status = ACCEL_Init();
if (status != ACCEL_OK) {
    // Handle initialization error
}

// Read acceleration data
ACCEL_DataTypeDef data;
status = ACCEL_ReadData(&data);
if (status == ACCEL_OK) {
    printf("X: %.3f g, Y: %.3f g, Z: %.3f g\n",
           data.X_g, data.Y_g, data.Z_g);
}
```

### Custom Configuration
```c
ACCEL_ConfigTypeDef config = {
    .DataRate = ACCEL_ODR_400HZ,      // 400 Hz output rate
    .Range = ACCEL_RANGE_4G,          // ±4g measurement range
    .Mode = ACCEL_MODE_ACTIVE,        // Active mode
    .HighPassFilter = true,           // Enable high-pass filter
    .LowNoise = true                  // Enable low noise mode
};

ACCEL_StatusTypeDef status = ACCEL_Init_Custom(&config);
```

### Raw Data Reading
```c
int16_t xRaw, yRaw, zRaw;
ACCEL_StatusTypeDef status = ACCEL_ReadRawData(&xRaw, &yRaw, &zRaw);
if (status == ACCEL_OK) {
    // Raw values are 14-bit signed (-8192 to +8191)
    printf("Raw: X=%d, Y=%d, Z=%d\n", xRaw, yRaw, zRaw);
}
```

### Calibration
```c
// Automatic calibration (keep sensor level during calibration)
ACCEL_StatusTypeDef status = ACCEL_Calibrate();
if (status == ACCEL_OK) {
    printf("Calibration completed successfully\n");
}

// Manual offset adjustment
status = ACCEL_SetOffset(10, -5, 20);  // X, Y, Z offsets
```

### Interrupt Configuration
```c
ACCEL_IntConfigTypeDef intConfig = {
    .DataReady = true,     // Interrupt on new data
    .Motion = false,       // Motion detection disabled
    .Freefall = false,     // Freefall detection disabled
    .Tap = true            // Tap detection enabled
};

ACCEL_StatusTypeDef status = ACCEL_ConfigInterrupts(&intConfig);
```

## Hardware Configuration

### SPI Interface
The driver communicates with the MMA8452Q via SPI with the following configuration:
- **Mode**: SPI Mode 0 (CPOL=0, CPHA=0)
- **Data Size**: 8-bit transfers
- **Clock Speed**: Up to 1MHz (device limit)
- **Chip Select**: Software controlled

### Pin Connections (STM32F429-Discovery)
- **MOSI**: PF9 (SPI5)
- **MISO**: PF8 (SPI5)
- **SCK**: PF7 (SPI5)
- **CS**: Software controlled (GPIO output)

### SPI Command Format
- **Read**: 0x80 | register_address
- **Write**: 0x00 | register_address

## Register Map

### Key Registers
- `0x00`: STATUS - Data status register
- `0x01-0x06`: OUT_X/Y/Z_MSB/LSB - Acceleration data
- `0x0D`: WHO_AM_I - Device ID (0x2A)
- `0x0E`: XYZ_DATA_CFG - Data configuration
- `0x2A`: CTRL_REG1 - Control register 1 (data rate, active/standby)
- `0x2B`: CTRL_REG2 - Control register 2
- `0x2D`: CTRL_REG4 - Interrupt enable
- `0x2E`: CTRL_REG5 - Interrupt routing

## Configuration Constants

### Output Data Rates
- `ACCEL_ODR_800HZ` - 800 Hz
- `ACCEL_ODR_400HZ` - 400 Hz
- `ACCEL_ODR_200HZ` - 200 Hz
- `ACCEL_ODR_100HZ` - 100 Hz (default)
- `ACCEL_ODR_50HZ` - 50 Hz
- `ACCEL_ODR_12_5HZ` - 12.5 Hz
- `ACCEL_ODR_6_25HZ` - 6.25 Hz
- `ACCEL_ODR_1_56HZ` - 1.56 Hz

### Measurement Ranges
- `ACCEL_RANGE_2G` - ±2g (default)
- `ACCEL_RANGE_4G` - ±4g
- `ACCEL_RANGE_8G` - ±8g

### Operating Modes
- `ACCEL_MODE_STANDBY` - Standby mode (low power)
- `ACCEL_MODE_ACTIVE` - Active mode (normal operation)
- `ACCEL_MODE_SLEEP` - Sleep mode

## Data Conversion

### Raw to G-Force Conversion
```c
// Sensitivity values (counts per g)
#define SENSITIVITY_2G     4096.0f    // 2^14 / 2
#define SENSITIVITY_4G     2048.0f    // 2^14 / 4
#define SENSITIVITY_8G     1024.0f    // 2^14 / 8

float acceleration_g = (float)raw_value / sensitivity;
```

### 14-bit Data Format
- **Range**: -8192 to +8191 counts
- **Resolution**: 14-bit signed integer
- **MSB/LSB**: Data is stored as MSB first, LSB second

## Error Handling

The driver provides comprehensive error handling:

- `ACCEL_OK` - Operation successful
- `ACCEL_ERROR` - General error
- `ACCEL_BUSY` - Device busy
- `ACCEL_TIMEOUT` - Operation timeout
- `ACCEL_INVALID_PARAM` - Invalid parameter
- `ACCEL_NOT_READY` - Device not ready

## Running Examples

To run the comprehensive examples:

```c
#include "accel_example.h"

// Run all accelerometer examples
ACCEL_RunExamples();
```

This will demonstrate:
1. Basic data reading
2. Configuration changes
3. Calibration procedures
4. Interrupt configuration
5. Self-test functionality

## Performance Considerations

### Power Consumption
- **Standby Mode**: ~6µA
- **Active Mode**: ~165µA at 100Hz
- **Sleep Mode**: Reduced power with periodic wake-ups

### Timing Considerations
- **Startup Time**: ~1ms from standby to active
- **Data Ready**: Varies with output data rate
- **Calibration**: ~1 second for automatic calibration

### SPI Timing
- **Clock Speed**: Maximum 1MHz for MMA8452Q
- **Chip Select**: Minimum 100ns high time between transfers
- **Data Setup**: 50ns minimum data setup time

## Troubleshooting

### Common Issues
1. **No Communication**: Check SPI wiring and power supply
2. **Incorrect Data**: Verify SPI mode and data format
3. **Self-Test Failure**: Check accelerometer mounting and power
4. **Interrupt Issues**: Verify interrupt pin connections

### Debug Tips
- Use oscilloscope to verify SPI signals
- Check device ID (should be 0x2A)
- Verify power supply voltage (1.95V to 3.6V)
- Test with different SPI clock speeds

## Integration Notes

### Dependencies
- STM32F4xx HAL Library
- SPI driver (`spi.h`, `spi.c`)
- Standard C libraries (`stdint.h`, `stdbool.h`, `math.h`)

### Thread Safety
The driver is not thread-safe. Use mutex protection for multi-threaded applications.

### Memory Usage
- **RAM**: ~50 bytes for static variables
- **Stack**: ~100 bytes for local variables during operation

---

For more detailed information, refer to the MMA8452Q datasheet and STM32F429 reference manual.
