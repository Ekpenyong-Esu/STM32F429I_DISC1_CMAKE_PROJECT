# GYRO Driver for L3GD20 Gyroscope

## Overview

This driver provides a complete interface for the L3GD20 3-axis digital gyroscope sensor on the STM32F429 Discovery board. The L3GD20 is a low-power gyroscope that measures angular velocity in three axes with selectable full-scale ranges and output data rates.

## Features

- **Full L3GD20 Support**: Complete register map and functionality
- **Multiple Configurations**: Configurable data rates, full-scale ranges, and power modes
- **SPI Communication**: High-speed SPI interface with proper timing
- **Data Conversion**: Raw data to degrees per second (dps) conversion
- **Temperature Sensing**: Built-in temperature measurement
- **Error Handling**: Comprehensive error codes and validation
- **HAL Integration**: Full STM32 HAL library compatibility

## Hardware Configuration

### STM32F429 Discovery Board Connections
- **SPI5_SCK**: PF7 (Serial Clock)
- **SPI5_MISO**: PF8 (Master In Slave Out)
- **SPI5_MOSI**: PF9 (Master Out Slave In)
- **CS**: PC1 (Chip Select)

### L3GD20 Specifications
- **Supply Voltage**: 2.16V to 3.6V
- **Interface**: SPI (up to 10 MHz)
- **Full-Scale Ranges**: ±250, ±500, ±2000 dps
- **Output Data Rates**: 95, 190, 380, 760 Hz
- **Resolution**: 16-bit

## API Reference

### Initialization Functions

```c
HAL_StatusTypeDef GYRO_Init(GYRO_Handle_t *handle, SPI_HandleTypeDef *hspi, 
                           GPIO_TypeDef *csPort, uint16_t csPin, 
                           GYRO_Config_t *config);
HAL_StatusTypeDef GYRO_DeInit(GYRO_Handle_t *handle);
```

### Data Reading Functions

```c
HAL_StatusTypeDef GYRO_ReadData(GYRO_Handle_t *handle, GYRO_Data_t *data);
HAL_StatusTypeDef GYRO_ReadAxis(GYRO_Handle_t *handle, uint8_t axis, int16_t *value);
HAL_StatusTypeDef GYRO_ReadTemperature(GYRO_Handle_t *handle, int8_t *temperature);
HAL_StatusTypeDef GYRO_IsDataReady(GYRO_Handle_t *handle, bool *dataReady);
```

### Configuration Functions

```c
HAL_StatusTypeDef GYRO_SetFullScale(GYRO_Handle_t *handle, uint8_t fullScale);
HAL_StatusTypeDef GYRO_SetOutputDataRate(GYRO_Handle_t *handle, uint8_t dataRate);
HAL_StatusTypeDef GYRO_SetAxes(GYRO_Handle_t *handle, uint8_t axes);
HAL_StatusTypeDef GYRO_ConfigInterrupt(GYRO_Handle_t *handle, uint8_t config);
```

### Utility Functions

```c
HAL_StatusTypeDef GYRO_GetDeviceID(GYRO_Handle_t *handle, uint8_t *deviceId);
float GYRO_ConvertToDPS(int16_t rawValue, uint8_t fullScale);
uint32_t GYRO_GetError(GYRO_Handle_t *handle);
```

## Configuration Options

### Power Modes
- `GYRO_POWER_DOWN`: Power-down mode (lowest power consumption)
- `GYRO_POWER_NORMAL`: Normal mode (active sensing)

### Output Data Rates
- `GYRO_ODR_95HZ_BW_12_5`: 95 Hz, 12.5 Hz bandwidth
- `GYRO_ODR_190HZ_BW_25`: 190 Hz, 25 Hz bandwidth
- `GYRO_ODR_380HZ_BW_50`: 380 Hz, 50 Hz bandwidth
- `GYRO_ODR_760HZ_BW_100`: 760 Hz, 100 Hz bandwidth

### Full-Scale Ranges
- `GYRO_FULLSCALE_250`: ±250 dps (8.75 mdps/LSB)
- `GYRO_FULLSCALE_500`: ±500 dps (17.5 mdps/LSB)
- `GYRO_FULLSCALE_2000`: ±2000 dps (70 mdps/LSB)

### Axis Control
- `GYRO_X_ENABLE`: Enable X-axis
- `GYRO_Y_ENABLE`: Enable Y-axis
- `GYRO_Z_ENABLE`: Enable Z-axis
- `GYRO_XYZ_ENABLE`: Enable all axes

## Usage Examples

### Basic Initialization

```c
#include "gyro.h"

GYRO_Handle_t hgyro;
SPI_HandleTypeDef hspi5;

// Configure GYRO settings
GYRO_Config_t config = {
    .powerMode = GYRO_POWER_NORMAL,
    .outputDataRate = GYRO_ODR_95HZ_BW_12_5,
    .bandwidth = GYRO_BANDWIDTH_1,
    .fullScale = GYRO_FULLSCALE_250,
    .blockDataUpdate = GYRO_BDU_CONTINUOUS,
    .endianness = GYRO_BLE_MSB,
    .axes = GYRO_XYZ_ENABLE
};

// Initialize GYRO
if (GYRO_Init(&hgyro, &hspi5, GPIOC, GPIO_PIN_1, &config) == HAL_OK) {
    printf("GYRO initialized successfully!\\n");
}
```

### Reading Gyroscope Data

```c
GYRO_Data_t gyroData;

// Check if new data is available
bool dataReady;
if (GYRO_IsDataReady(&hgyro, &dataReady) == HAL_OK && dataReady) {
    // Read gyroscope data
    if (GYRO_ReadData(&hgyro, &gyroData) == HAL_OK) {
        // Convert to degrees per second
        float x_dps = GYRO_ConvertToDPS(gyroData.x, GYRO_FULLSCALE_250);
        float y_dps = GYRO_ConvertToDPS(gyroData.y, GYRO_FULLSCALE_250);
        float z_dps = GYRO_ConvertToDPS(gyroData.z, GYRO_FULLSCALE_250);
        
        printf("Angular velocity: X=%.2f, Y=%.2f, Z=%.2f dps\\n", 
               x_dps, y_dps, z_dps);
    }
}
```

### Temperature Reading

```c
int8_t temperature;
if (GYRO_ReadTemperature(&hgyro, &temperature) == HAL_OK) {
    printf("Temperature: %d°C\\n", temperature);
}
```

### Configuration Changes

```c
// Change full-scale range to ±500 dps
GYRO_SetFullScale(&hgyro, GYRO_FULLSCALE_500);

// Change output data rate to 190 Hz
GYRO_SetOutputDataRate(&hgyro, GYRO_ODR_190HZ_BW_25);

// Enable only X and Y axes
GYRO_SetAxes(&hgyro, GYRO_X_ENABLE | GYRO_Y_ENABLE);
```

## Error Handling

The driver provides comprehensive error codes:

- `GYRO_ERROR_NONE`: No error
- `GYRO_ERROR_INIT`: Initialization error
- `GYRO_ERROR_SPI`: SPI communication error
- `GYRO_ERROR_DEVICE_ID`: Device ID mismatch
- `GYRO_ERROR_CONFIG`: Configuration error
- `GYRO_ERROR_READ`: Read operation error
- `GYRO_ERROR_WRITE`: Write operation error

Example error handling:

```c
if (GYRO_ReadData(&hgyro, &gyroData) != HAL_OK) {
    uint32_t error = GYRO_GetError(&hgyro);
    printf("GYRO error: 0x%08lX\\n", error);
}
```

## Performance Considerations

### SPI Configuration
- **Clock Speed**: Up to 10 MHz supported
- **Mode**: SPI Mode 3 (CPOL=1, CPHA=1)
- **Data Size**: 8-bit
- **Bit Order**: MSB first

### Timing Requirements
- **Power-up Time**: 5ms typical
- **Data Ready**: Dependent on ODR setting
- **SPI Timeout**: 1000ms default

### Memory Usage
- **Handle Size**: ~32 bytes
- **Stack Usage**: Minimal (local variables only)
- **Flash Usage**: ~2KB (driver + examples)

## Integration with STM32CubeMX

1. **Enable SPI5**: Configure as Master, Full-Duplex
2. **Configure GPIO**: Set PF7/PF8/PF9 as SPI5 AF, PC1 as GPIO Output
3. **Clock Configuration**: Ensure adequate SPI clock frequency
4. **Include Files**: Add gyro.h and gyro.c to project

## Troubleshooting

### Common Issues

1. **Device ID Mismatch**
   - Check SPI connections
   - Verify chip select polarity
   - Ensure proper power supply

2. **No Data Ready**
   - Check output data rate configuration
   - Verify power mode (should be normal)
   - Ensure axes are enabled

3. **SPI Communication Errors**
   - Check SPI clock polarity and phase
   - Verify GPIO alternate function settings
   - Check for proper pull-up/pull-down resistors

### Debug Tips

```c
// Check device ID
uint8_t deviceId;
GYRO_GetDeviceID(&hgyro, &deviceId);
printf("Device ID: 0x%02X (expected: 0x%02X)\\n", deviceId, L3GD20_DEVICE_ID);

// Monitor error codes
uint32_t error = GYRO_GetError(&hgyro);
if (error != GYRO_ERROR_NONE) {
    printf("GYRO Error: 0x%08lX\\n", error);
}
```

## Dependencies

- STM32F4xx HAL Library
- SPI HAL module
- GPIO HAL module
- Standard C library (for data types)

## License

This driver is provided under the STM32 software license terms.

## Version History

- **v1.0**: Initial release with full L3GD20 support
- Complete register map implementation
- Comprehensive examples and documentation
- HAL library integration
