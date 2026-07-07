# MEMS Sensor Driver for STM32F429 Discovery Board

This directory contains a comprehensive MEMS (Micro-Electro-Mechanical Systems) sensor driver implementation for the STM32F429 Discovery board, specifically designed for the L3GD20 3-axis digital gyroscope.

**Note:** The older `Peripherals/GYRO` driver was removed — use this `MEMS` driver for both the on-board and external gyroscopes (use `MEMS_SetCS()` to specify an external CS pin).

## Overview

The MEMS driver provides a complete interface for controlling and reading data from the L3GD20 gyroscope sensor connected via SPI5 on the STM32F429 Discovery board. The driver includes features for initialization, calibration, data reading, power management, and interrupt handling.

## Hardware Configuration

### L3GD20 Gyroscope Sensor
- **Connection**: SPI5 interface
- **Full Scale Ranges**: ±250, ±500, ±2000 dps (degrees per second)
- **Output Data Rates**: 95Hz, 190Hz, 380Hz, 760Hz
- **Resolution**: 16-bit
- **Operating Voltage**: 2.16V to 3.6V

### Pin Configuration
| Function | Pin | GPIO Port | Description |
|----------|-----|-----------|-------------|
| SPI5_SCK | PF7 | GPIOF | Serial Clock |
| SPI5_MISO | PF8 | GPIOF | Master In Slave Out |
| SPI5_MOSI | PF9 | GPIOF | Master Out Slave In |
| CS | PC1 | GPIOC | Chip Select |
| INT1 | PA1 | GPIOA | Interrupt 1 |
| INT2 | PA2 | GPIOA | Interrupt 2 |

## Files Structure

```
MEMS/
├── mems.h              # Main driver header file
├── mems.c              # Main driver implementation
├── mems_example.h      # Examples header file
├── mems_example.c      # Examples implementation
└── README.md           # This documentation file
```

## Features

### Core Features
- ✅ Device initialization and configuration
- ✅ Gyroscope data reading (raw and engineering units)
- ✅ Temperature sensor reading
- ✅ Automatic calibration with offset compensation
- ✅ Power management (normal/power-down modes)
- ✅ Interrupt configuration and handling
- ✅ Self-test functionality
- ✅ Multiple full-scale ranges and data rates
- ✅ Error handling and status reporting

### Advanced Features
- ✅ Motion detection algorithms
- ✅ Statistical data analysis
- ✅ Performance testing utilities
- ✅ Comprehensive example implementations
- ✅ Device information and status monitoring

## API Reference

### Initialization Functions
```c
MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, SPI_HandleTypeDef *hspi);
MEMS_StatusTypeDef MEMS_DeInit(MEMS_HandleTypeDef *hmems);
```

### Configuration Functions
```c
MEMS_StatusTypeDef MEMS_GyroConfig(MEMS_HandleTypeDef *hmems, MEMS_GyroConfigTypeDef *config);
MEMS_StatusTypeDef MEMS_ConfigureInterrupt(MEMS_HandleTypeDef *hmems, MEMS_InterruptConfigTypeDef *config);
MEMS_StatusTypeDef MEMS_SetPowerMode(MEMS_HandleTypeDef *hmems, bool power_down);
```

### Data Reading Functions
```c
MEMS_StatusTypeDef MEMS_GyroReadRaw(MEMS_HandleTypeDef *hmems, MEMS_AxesRawTypeDef *axes);
MEMS_StatusTypeDef MEMS_GyroRead(MEMS_HandleTypeDef *hmems, MEMS_AxesTypeDef *axes);
MEMS_StatusTypeDef MEMS_ReadTemperature(MEMS_HandleTypeDef *hmems, float *temperature);
```

### Calibration Functions
```c
MEMS_StatusTypeDef MEMS_CalibrateGyroscope(MEMS_HandleTypeDef *hmems, uint16_t samples);
```

### Utility Functions
```c
MEMS_StatusTypeDef MEMS_GetDeviceInfo(MEMS_HandleTypeDef *hmems, MEMS_DeviceInfoTypeDef *info);
MEMS_StatusTypeDef MEMS_SelfTest(MEMS_HandleTypeDef *hmems, bool *result);
float MEMS_ConvertToDPS(int16_t raw_data, MEMS_GyroFullScaleTypeDef full_scale);
```

## Usage Examples

### Basic Initialization and Reading

**Using an external gyroscope (custom CS pin)**

If your gyroscope is not the on-board device and you wired it to a custom CS pin, call `MEMS_SetCS()` before `MEMS_Init()` to configure the chip-select pin for the `MEMS_HandleTypeDef`:

```c
// Example: external device CS on PA4
MEMS_SetCS(&hmems, GPIOA, GPIO_PIN_4);
if (MEMS_Init(&hmems, &hspi5) == MEMS_OK) {
    // proceed as normal
}
```
```c
#include "mems.h"

MEMS_HandleTypeDef hmems;
SPI_HandleTypeDef hspi5;
MEMS_AxesTypeDef gyro_data;

// Initialize MEMS sensor
if (MEMS_Init(&hmems, &hspi5) == MEMS_OK) {
    // Read gyroscope data
    if (MEMS_GyroRead(&hmems, &gyro_data) == MEMS_OK) {
        printf("Gyro X: %.2f dps\n", gyro_data.X);
        printf("Gyro Y: %.2f dps\n", gyro_data.Y);
        printf("Gyro Z: %.2f dps\n", gyro_data.Z);
    }
}
```

### Configuration Example
```c
MEMS_GyroConfigTypeDef config = {
    .OutputDataRate = MEMS_GYRO_ODR_190Hz,
    .FullScale = MEMS_GYRO_FULLSCALE_500,
    .Bandwidth = MEMS_GYRO_BANDWIDTH_2,
    .XAxisEnable = true,
    .YAxisEnable = true,
    .ZAxisEnable = true,
    .PowerDownMode = false
};

MEMS_GyroConfig(&hmems, &config);
```

### Calibration Example
```c
// Perform calibration with 100 samples
printf("Keep device stationary for calibration...\n");
if (MEMS_CalibrateGyroscope(&hmems, 100) == MEMS_OK) {
    printf("Calibration completed successfully\n");
    printf("X offset: %.3f dps\n", hmems.CalibrationOffset.X);
    printf("Y offset: %.3f dps\n", hmems.CalibrationOffset.Y);
    printf("Z offset: %.3f dps\n", hmems.CalibrationOffset.Z);
}
```

## Integration Guide

### 1. Hardware Setup
Ensure that the STM32F429 Discovery board is properly connected and powered. The L3GD20 sensor is already mounted on the board and connected to the SPI5 interface.

### 2. Include Headers
```c
#include "mems.h"
#include "mems_example.h"  // For examples
```

### 3. Initialize SPI5
The SPI5 peripheral must be configured before initializing the MEMS driver. This is typically done in the main initialization routine:

```c
// SPI5 must be configured by the application using the shared SPI driver (Peripherals/SPI). For example call `SPI_Init()` or `SPI_Init_Custom()` before `MEMS_Init()`.
void MX_SPI5_Init(void) {
    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi5.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi5);
}
```

### 4. Application Integration
```c
int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    MEMS_HandleTypeDef hmems;
    SPI_HandleTypeDef hspi5;
    
    // Initialize MEMS sensor
    if (MEMS_Init(&hmems, &hspi5) == MEMS_OK) {
        // Run basic example
        MEMS_ExampleResultTypeDef result = MEMS_Example_Basic(&hmems, &hspi5);
        MEMS_Example_PrintResult(&result);
        
        // Your application code here
        while (1) {
            MEMS_AxesTypeDef gyro;
            if (MEMS_GyroRead(&hmems, &gyro) == MEMS_OK) {
                // Process gyroscope data
            }
            HAL_Delay(10);
        }
    }
    
    return 0;
}
```

## Error Handling

The driver uses comprehensive error codes for robust error handling:

```c
typedef enum {
    MEMS_OK = 0,                // Operation completed successfully
    MEMS_ERROR,                 // General error occurred
    MEMS_BUSY,                  // MEMS device is busy
    MEMS_TIMEOUT,               // Operation timed out
    MEMS_INVALID_PARAM,         // Invalid parameter provided
    MEMS_NOT_INITIALIZED,       // Device not initialized
    MEMS_COMMUNICATION_ERROR,   // SPI communication error
    MEMS_DEVICE_NOT_FOUND      // Device not detected
} MEMS_StatusTypeDef;
```

### Error Handling Example
```c
MEMS_StatusTypeDef status = MEMS_GyroRead(&hmems, &gyro_data);
switch (status) {
    case MEMS_OK:
        // Process data
        break;
    case MEMS_NOT_INITIALIZED:
        printf("ERROR: MEMS not initialized\n");
        break;
    case MEMS_COMMUNICATION_ERROR:
        printf("ERROR: SPI communication failed\n");
        break;
    default:
        printf("ERROR: Unknown error (%d)\n", status);
        break;
}
```

## Performance Considerations

### Timing Requirements
- **Initialization Time**: ~10ms
- **Reading Time**: ~1ms per axis set
- **Calibration Time**: ~2-5 seconds (depending on sample count)
- **Maximum Data Rate**: Up to 760Hz (limited by SPI speed)

### Memory Usage
- **Driver Structure**: ~100 bytes
- **Stack Usage**: ~50 bytes per function call
- **No dynamic memory allocation**

### Power Consumption
- **Normal Mode**: ~6.1mA
- **Power-Down Mode**: ~5µA
- Use power-down mode when not actively reading data

## Troubleshooting

### Common Issues

1. **Device Not Found (MEMS_DEVICE_NOT_FOUND)**
   - Check SPI connections
   - Verify power supply
   - Ensure correct pin configuration

2. **Communication Errors**
   - Verify SPI clock frequency (max 10MHz)
   - Check CS pin control
   - Ensure proper SPI mode configuration

3. **Noisy Data**
   - Perform calibration
   - Check for mechanical vibrations
   - Use appropriate full-scale range

4. **Calibration Issues**
   - Ensure device is stationary during calibration
   - Use sufficient number of samples (>100)
   - Allow settling time before calibration

### Debug Tips
```c
// Enable debug output
#define MEMS_DEBUG_ENABLE

// Check device presence
MEMS_DeviceInfoTypeDef info;
if (MEMS_GetDeviceInfo(&hmems, &info) == MEMS_OK) {
    printf("Device: %s, WHO_AM_I: 0x%02X\n", info.DeviceName, info.WhoAmI);
}

// Perform self-test
bool test_result;
if (MEMS_SelfTest(&hmems, &test_result) == MEMS_OK) {
    printf("Self-test: %s\n", test_result ? "PASS" : "FAIL");
}
```

## Dependencies

### HAL Libraries
- `stm32f4xx_hal.h`
- `stm32f4xx_hal_spi.h`
- `stm32f4xx_hal_gpio.h`

### Standard Libraries
- `<stdint.h>`
- `<stdbool.h>`
- `<string.h>`
- `<math.h>` (for magnitude calculations)

## Examples

The `mems_example.c` file provides comprehensive examples:

1. **Basic Example**: Initialization and basic reading
2. **Continuous Reading**: Data collection over time
3. **Calibration**: Offset compensation
4. **Temperature Monitoring**: Temperature sensor usage
5. **Motion Detection**: Movement detection algorithms
6. **Self-Test**: Built-in diagnostic
7. **Power Management**: Power mode control
8. **Performance Test**: Speed and reliability testing

Run examples with:
```c
MEMS_ExampleResultTypeDef result = MEMS_Example_Basic(&hmems, &hspi5);
MEMS_Example_PrintResult(&result);
```

## License

This MEMS driver is provided as part of the STM32F429 Discovery board peripheral driver collection.

## Version History

- **v1.0** (2025-09-04): Initial release
  - L3GD20 gyroscope support
  - Complete API implementation
  - Comprehensive examples
  - Full documentation

## Support

For technical support or questions about this driver, please refer to:
- STM32F429 Reference Manual
- L3GD20 Datasheet
- STM32F429 Discovery Board User Manual
