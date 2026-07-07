# IR Distance Sensor Driver

This peripheral provides IR distance measurement using sensors like Sharp GP2Y0A21YK, using ADC for analog distance reading.

## Features

- Support for multiple Sharp IR distance sensors (GP2Y0A21YK, GP2Y0A02YK, etc.)
- ADC-based analog distance measurement
- Piecewise linear calibration curves
- Configurable averaging and filtering
- Custom sensor calibration support

## Dependencies

- **ADC**: ADC peripheral for analog voltage reading
- **HAL**: STM32 HAL library

## Hardware Requirements

- IR distance sensor with analog output (Sharp GP2Y0A series)
- ADC channel connected to sensor output
- 3.3V power supply for sensor

## Supported Sensors

| Sensor | Range | Typical Use |
|--------|-------|-------------|
| GP2Y0A21YK | 10-80cm | Medium range |
| GP2Y0A02YK | 20-150cm | Long range |
| GP2Y0A41SK | 4-30cm | Short range |
| GP2Y0A51SK | 2-15cm | Very short range |

## API Reference

### Initialization

```c
IR_DISTANCE_Handle_t hird;
ADC_HandleTypeDef hadc1;

IR_DISTANCE_StatusTypeDef status = IR_DISTANCE_Init(&hird, &hadc1, ADC_CHANNEL_0, IR_DISTANCE_GP2Y0A21YK);
```

### Configuration

```c
IR_DISTANCE_Config_t config = IR_DISTANCE_GetDefaultConfig(IR_DISTANCE_GP2Y0A21YK);
config.averagingSamples = 10;  // More averaging for stability

IR_DISTANCE_Config(&hird, &config);
```

### Distance Measurement

```c
// Single measurement
uint16_t distance = IR_DISTANCE_MeasureDistance(&hird);

// Get last measurement
uint16_t distance = IR_DISTANCE_GetDistance(&hird);

// Get raw ADC value
uint16_t adcValue = IR_DISTANCE_GetAdcValue(&hird);
```

### Custom Calibration

```c
// Add calibration points
IR_DISTANCE_CalibratePoint(&hird, 100, 3580);  // 10cm = 3580 ADC
IR_DISTANCE_CalibratePoint(&hird, 200, 1900);  // 20cm = 1900 ADC
IR_DISTANCE_CalibratePoint(&hird, 300, 1250);  // 30cm = 1250 ADC

// Or set complete custom curve
IR_DISTANCE_CustomCurve_t customCurve = {
    .numPoints = 3,
    .points = {
        {100, 3580}, {200, 1900}, {300, 1250}
    }
};
IR_DISTANCE_SetCustomCurve(&hird, &customCurve);
```

### Utility Functions

```c
// Convert between ADC and distance
uint16_t distance = IR_DISTANCE_AdcToDistance(&hird, 2500);
uint16_t adcValue = IR_DISTANCE_DistanceToAdc(&hird, 150);

// Validate distance range
bool valid = IR_DISTANCE_IsValidDistance(&hird, 500);
```

## Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| sensorType | GP2Y0A21YK | Type of IR sensor |
| minDistance | varies | Minimum measurable distance (mm) |
| maxDistance | varies | Maximum measurable distance (mm) |
| averagingSamples | 5 | Number of ADC samples to average |
| measurementTimeout | 100ms | Maximum measurement time |
| voltageScale | 3.3V | ADC reference voltage |
| voltageOffset | 0.0V | ADC voltage offset |

## Calibration Curves

The driver uses piecewise linear interpolation with predefined curves:

### GP2Y0A21YK (10-80cm)
```
Distance: 100mm → ADC: 3580
Distance: 200mm → ADC: 1900
Distance: 300mm → ADC: 1250
Distance: 500mm → ADC: 780
Distance: 800mm → ADC: 480
```

### GP2Y0A02YK (20-150cm)
```
Distance: 200mm → ADC: 3200
Distance: 500mm → ADC: 1250
Distance: 1000mm → ADC: 650
Distance: 1500mm → ADC: 450
```

## Example Usage

```c
#include "ir_distance.h"

IR_DISTANCE_Handle_t hird;
ADC_HandleTypeDef hadc1;

void IR_Distance_Example(void)
{
    // Initialize sensor
    if (IR_DISTANCE_Init(&hird, &hadc1, ADC_CHANNEL_0, IR_DISTANCE_GP2Y0A21YK) == IR_DISTANCE_OK) {
        // Configure averaging
        IR_DISTANCE_Config_t config = IR_DISTANCE_GetDefaultConfig(IR_DISTANCE_GP2Y0A21YK);
        config.averagingSamples = 10;
        IR_DISTANCE_Config(&hird, &config);

        // Measure distance
        uint16_t distance = IR_DISTANCE_MeasureDistance(&hird);

        if (distance > 0) {
            printf("Distance: %d mm\n", distance);
        }
    }
}
```

## Custom Sensor Support

For sensors not in the predefined list:

```c
// Initialize as custom sensor
IR_DISTANCE_Init(&hird, &hadc1, ADC_CHANNEL_0, IR_DISTANCE_CUSTOM);

// Define calibration curve
IR_DISTANCE_CustomCurve_t curve = {
    .numPoints = 5,
    .points = {
        {50, 3800}, {100, 3200}, {150, 2500},
        {200, 1900}, {250, 1500}
    }
};

IR_DISTANCE_SetCustomCurve(&hird, &curve);
```

## Error Handling

The driver returns status codes:
- `IR_DISTANCE_OK`: Operation successful
- `IR_DISTANCE_INVALID_PARAM`: Invalid parameter
- `IR_DISTANCE_NOT_INITIALIZED`: Driver not initialized
- `IR_DISTANCE_OUT_OF_RANGE`: Distance outside sensor range

## Integration Notes

1. Ensure ADC is properly configured before calling `IR_DISTANCE_Init()`
2. IR sensors require clean power supply and may need decoupling capacitors
3. Calibration improves accuracy significantly
4. Averaging reduces noise but increases measurement time
5. Sensor output is nonlinear - calibration curves compensate for this

## File Structure

```
Peripherals/IR_DISTANCE/
├── ir_distance.h          # Header file with API declarations
├── ir_distance.c          # Implementation file
└── README.md             # This documentation
```
