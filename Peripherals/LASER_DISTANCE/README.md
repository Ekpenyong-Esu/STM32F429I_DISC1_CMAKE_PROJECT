# Laser Distance Sensor Driver

This peripheral provides laser distance measurement using sensors like VL53L0X, using I2C interface for communication.

## Features

- Support for ST VL53L0X laser distance sensor
- I2C communication interface
- Single-shot and continuous measurement modes
- Calibration and offset compensation
- Ambient light and signal rate monitoring

## Dependencies

- **I2C**: I2C peripheral for sensor communication
- **HAL**: STM32 HAL library

## Hardware Requirements

- Laser distance sensor with I2C interface (VL53L0X, VL53L1X)
- I2C bus connection
- 2.8V-3.3V power supply for sensor

## Supported Sensors

| Sensor | Range | Interface | Features |
|--------|-------|-----------|----------|
| VL53L0X | 30-2000mm | I2C | Ambient light sensing, offset calibration |
| VL53L1X | 40-4000mm | I2C | Multi-zone sensing, faster measurement |
| TFmini | 30-12000mm | UART | Long range, industrial applications |

## API Reference

### Initialization

```c
LASER_DISTANCE_Handle_t hlaser;
I2C_HandleTypeDef hi2c1;

LASER_DISTANCE_StatusTypeDef status = LASER_DISTANCE_Init(&hlaser, &hi2c1, LASER_DISTANCE_VL53L0X);
```

### Configuration

```c
LASER_DISTANCE_Config_t config = LASER_DISTANCE_GetDefaultConfig(LASER_DISTANCE_VL53L0X);
config.longRangeMode = true;  // Enable long range mode
config.averagingSamples = 10; // More averaging for stability

LASER_DISTANCE_Config(&hlaser, &config);
```

### Single Measurements

```c
// Single distance measurement
uint16_t distance = LASER_DISTANCE_MeasureDistance(&hlaser);

// Get complete measurement data
LASER_DISTANCE_Measurement_t measurement;
LASER_DISTANCE_GetMeasurement(&hlaser, &measurement);

printf("Distance: %d mm, Signal: %d, Ambient: %d\n",
       measurement.distance,
       measurement.signalRate,
       measurement.ambientRate);
```

### Continuous Measurements

```c
// Start continuous mode
LASER_DISTANCE_StartContinuous(&hlaser);

// Check for new measurements
while (true) {
    if (LASER_DISTANCE_IsMeasurementReady(&hlaser)) {
        uint16_t distance = LASER_DISTANCE_GetDistance(&hlaser);
        printf("Distance: %d mm\n", distance);
    }
    HAL_Delay(100);
}

// Stop continuous mode
LASER_DISTANCE_StopContinuous(&hlaser);
```

### Calibration and Setup

```c
// Change I2C address (VL53L0X only)
LASER_DISTANCE_ChangeAddress(&hlaser, 0x54);

// Perform calibration
LASER_DISTANCE_Calibrate(&hlaser);
```

### Utility Functions

```c
// Validate distance range
bool valid = LASER_DISTANCE_IsValidDistance(&hlaser, 1500);

// Get status string
const char* statusStr = LASER_DISTANCE_GetStatusString(status);
```

## Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| sensorType | VL53L0X | Type of laser distance sensor |
| minDistance | varies | Minimum measurable distance (mm) |
| maxDistance | varies | Maximum measurable distance (mm) |
| averagingSamples | 5 | Number of samples for averaging |
| measurementTimeout | 500ms | Maximum measurement time |
| i2cAddress | 0x52 | I2C slave address |
| longRangeMode | false | Enable long range mode |
| highAccuracyMode | false | Enable high accuracy mode |

## VL53L0X Specific Features

### Measurement Data Structure
```c
typedef struct {
    uint16_t distance;       // Measured distance in mm
    uint16_t ambientRate;    // Ambient light rate
    uint16_t signalRate;     // Return signal rate
    uint8_t rangeStatus;     // Range status (0=valid)
    uint32_t timestamp;      // Measurement timestamp
} LASER_DISTANCE_Measurement_t;
```

### Range Status Values
- 0: Valid measurement
- 1: Sigma fail
- 2: Signal fail
- 3: Out of range
- 4: Hardware fail

## Example Usage

```c
#include "laser_distance.h"

LASER_DISTANCE_Handle_t hlaser;
I2C_HandleTypeDef hi2c1;

void LaserDistance_Example(void)
{
    // Initialize sensor
    if (LASER_DISTANCE_Init(&hlaser, &hi2c1, LASER_DISTANCE_VL53L0X) == LASER_DISTANCE_OK) {
        // Configure for long range
        LASER_DISTANCE_Config_t config = LASER_DISTANCE_GetDefaultConfig(LASER_DISTANCE_VL53L0X);
        config.longRangeMode = true;
        LASER_DISTANCE_Config(&hlaser, &config);

        // Perform calibration
        LASER_DISTANCE_Calibrate(&hlaser);

        // Single measurement
        uint16_t distance = LASER_DISTANCE_MeasureDistance(&hlaser);

        if (distance > 0) {
            printf("Distance: %d mm\n", distance);
        }
    }
}
```

## I2C Address Management

```c
// Default address: 0x52
// Change to alternative address
LASER_DISTANCE_ChangeAddress(&hlaser, 0x54);

// Multiple sensors on same bus
LASER_DISTANCE_Handle_t hlaser1, hlaser2;
LASER_DISTANCE_Init(&hlaser1, &hi2c1, LASER_DISTANCE_VL53L0X);  // Address 0x52
LASER_DISTANCE_ChangeAddress(&hlaser1, 0x54);                  // Change to 0x54
LASER_DISTANCE_Init(&hlaser2, &hi2c1, LASER_DISTANCE_VL53L0X);  // Address 0x52
```

## Error Handling

The driver returns status codes:
- `LASER_DISTANCE_OK`: Operation successful
- `LASER_DISTANCE_I2C_ERROR`: I2C communication error
- `LASER_DISTANCE_TIMEOUT`: Measurement timeout
- `LASER_DISTANCE_OUT_OF_RANGE`: Distance outside sensor range
- `LASER_DISTANCE_INVALID_PARAM`: Invalid parameter
- `LASER_DISTANCE_NOT_INITIALIZED`: Driver not initialized

## Integration Notes

1. Ensure I2C is properly configured before calling `LASER_DISTANCE_Init()`
2. VL53L0X requires specific initialization sequence
3. Calibration improves accuracy significantly
4. Long range mode reduces maximum distance but improves close range accuracy
5. Continuous mode provides faster measurements but uses more power

## Sensor-Specific Notes

### VL53L0X
- Requires 2.8V power supply
- I2C clock up to 400kHz
- Calibration recommended for best accuracy
- Ambient light can affect measurements

### VL53L1X
- Improved performance over VL53L0X
- Multi-zone sensing capability
- Better ambient light rejection
- Not fully implemented in this driver yet

### TFmini
- Uses UART interface (not I2C)
- Very long range capability
- Industrial grade sensor
- Not implemented in this driver (requires UART)

## File Structure

```
Peripherals/LASER_DISTANCE/
├── laser_distance.h          # Header file with API declarations
├── laser_distance.c          # Implementation file
└── README.md                # This documentation
```
