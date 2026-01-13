# Ultrasonic Distance Sensor Driver

This peripheral provides ultrasonic distance measurement using sensors like HC-SR04, using timer input capture for echo pulse measurement.

## Features

- HC-SR04 compatible ultrasonic distance measurement
- Timer input capture for precise echo timing
- Temperature compensation for speed of sound
- Configurable measurement parameters
- Non-blocking and blocking measurement modes

## Dependencies

- **TIM**: Timer peripheral for input capture and timing
- **GPIO**: GPIO peripheral for trigger and echo pins
- **HAL**: STM32 HAL library

## Hardware Requirements

- Ultrasonic sensor with TRIGGER and ECHO pins (HC-SR04 compatible)
- Timer channel capable of input capture
- GPIO pins for trigger output and echo input

## API Reference

### Initialization

```c
ULTRASONIC_Handle_t hultra;
TIM_HandleTypeDef htim3;
ULTRASONIC_Pins_t pins = {
    .triggerPort = GPIOB,
    .triggerPin = GPIO_PIN_0,
    .echoPort = GPIOB,
    .echoPin = GPIO_PIN_1
};

ULTRASONIC_StatusTypeDef status = ULTRASONIC_Init(&hultra, &htim3, TIM_CHANNEL_1, &pins);
```

### Configuration

```c
ULTRASONIC_Config_t config = ULTRASONIC_GetDefaultConfig();
config.temperature = 25;  // 25°C for better accuracy
config.maxDistance = 3000;  // 3m maximum range

ULTRASONIC_Config(&hultra, &config);
```

### Distance Measurement

```c
// Single measurement (blocking)
uint16_t distance = ULTRASONIC_MeasureDistance(&hultra);

// Non-blocking measurement
ULTRASONIC_StartMeasurement(&hultra);
while (!ULTRASONIC_IsMeasurementComplete(&hultra)) {
    // Wait or do other tasks
}
uint16_t distance = ULTRASONIC_GetDistance(&hultra);

// Wait for completion with timeout
ULTRASONIC_StartMeasurement(&hultra);
if (ULTRASONIC_WaitForMeasurement(&hultra, 100) == ULTRASONIC_OK) {
    uint16_t distance = ULTRASONIC_GetDistance(&hultra);
}
```

### Temperature Compensation

```c
// Set ambient temperature for accurate measurements
ULTRASONIC_SetTemperature(&hultra, 30);  // 30°C
```

## Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| triggerTimeout | 1000ms | Trigger pulse timeout |
| measurementTimeout | 100ms | Echo measurement timeout |
| minDistance | 20mm | Minimum measurable distance |
| maxDistance | 4000mm | Maximum measurable distance |
| temperature | 20°C | Ambient temperature for speed correction |

## Timer Configuration

The driver requires:
- Timer frequency: 1MHz (1μs resolution)
- Input capture channel for echo pulse measurement
- Interrupt capability for echo detection

## Speed of Sound Calculation

The driver compensates for temperature:
- Speed = 331.3 + (0.6 × temperature) m/s
- Distance = (echo_time × speed × 1000) / (2 × 1000000) mm

## Example Usage

```c
#include "ultrasonic.h"

ULTRASONIC_Handle_t hultra;
TIM_HandleTypeDef htim3;

void Ultrasonic_Example(void)
{
    // Initialize pins
    ULTRASONIC_Pins_t pins = {
        .triggerPort = GPIOB,
        .triggerPin = GPIO_PIN_0,
        .echoPort = GPIOB,
        .echoPin = GPIO_PIN_1
    };

    // Initialize sensor
    if (ULTRASONIC_Init(&hultra, &htim3, TIM_CHANNEL_1, &pins) == ULTRASONIC_OK) {
        // Configure temperature
        ULTRASONIC_SetTemperature(&hultra, 25);

        // Measure distance
        uint16_t distance = ULTRASONIC_MeasureDistance(&hultra);

        if (distance > 0) {
            printf("Distance: %d mm\n", distance);
        }
    }
}
```

## Interrupt Handling

The driver requires timer input capture interrupts. Add this to your timer ISR:

```c
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);

    // Call ultrasonic callback
    ULTRASONIC_TIM_IC_CaptureCallback(&hultra);
}
```

## Error Handling

The driver returns status codes:
- `ULTRASONIC_OK`: Operation successful
- `ULTRASONIC_BUSY`: Sensor is measuring
- `ULTRASONIC_TIMEOUT`: Measurement timeout
- `ULTRASONIC_INVALID_PARAM`: Invalid parameter
- `ULTRASONIC_NOT_INITIALIZED`: Driver not initialized

## Integration Notes

1. Ensure timer is properly configured before calling `ULTRASONIC_Init()`
2. GPIO alternate functions must match timer input capture channels
3. Temperature compensation improves accuracy significantly
4. Use non-blocking mode for real-time applications

## File Structure

```
Peripherals/ULTRASONIC/
├── ultrasonic.h          # Header file with API declarations
├── ultrasonic.c          # Implementation file
└── README.md            # This documentation
```
