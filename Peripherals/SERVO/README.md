# Servo Motor Driver

This peripheral provides PWM-based servo motor control for the STM32F429 Discovery board.

## Features

- PWM-based servo control using timer peripherals
- Configurable angle range (0-180 degrees)
- Pulse width calibration
- Multiple control functions (angle, pulse width, sweep)
- MSP initialization for GPIO and PWM setup

## Dependencies

- **TIM**: Timer peripheral for PWM generation
- **GPIO**: GPIO peripheral for PWM output pin
- **HAL**: STM32 HAL library

## Hardware Requirements

- Servo motor with PWM control (typically 50Hz, 1-2ms pulse width)
- Timer channel capable of PWM output
- GPIO pin connected to servo signal line

## API Reference

### Initialization

```c
SERVO_Handle_t hservo;
TIM_HandleTypeDef htim3;
SERVO_StatusTypeDef status;

// Initialize servo on TIM3 Channel 1, GPIO PB4
status = SERVO_Init(&hservo, &htim3, TIM_CHANNEL_1, GPIOB, GPIO_PIN_4);
```

### Configuration

```c
SERVO_Config_t config = SERVO_GetDefaultConfig();
config.minAngle = 0;
config.maxAngle = 180;
config.minPulseWidth = 500;  // 0.5ms
config.maxPulseWidth = 2500; // 2.5ms

SERVO_Config(&hservo, &config);
```

### Control Functions

```c
// Set angle directly
SERVO_SetAngle(&hservo, 90);  // Center position

// Set pulse width directly
SERVO_SetPulseWidth(&hservo, 1500);  // 1.5ms pulse

// Move to predefined positions
SERVO_MoveToMin(&hservo);
SERVO_MoveToMax(&hservo);
SERVO_MoveToCenter(&hservo);

// Sweep across range
SERVO_Sweep(&hservo, 50);  // 50ms delay between steps
```

### Calibration

```c
// Calibrate minimum and maximum angles
SERVO_CalibrateMin(&hservo, 0);
SERVO_CalibrateMax(&hservo, 180);

// Reset to defaults
SERVO_ResetCalibration(&hservo);
```

### Utility Functions

```c
// Convert between angle and pulse width
uint16_t pulse = SERVO_AngleToPulseWidth(90, &config);
uint16_t angle = SERVO_PulseWidthToAngle(1500, &config);

// Validate parameters
bool valid = SERVO_IsValidAngle(90, &config);
bool valid = SERVO_IsValidPulseWidth(1500, &config);
```

## Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| minAngle | 0 | Minimum servo angle (degrees) |
| maxAngle | 180 | Maximum servo angle (degrees) |
| minPulseWidth | 500 | Minimum pulse width (μs) |
| maxPulseWidth | 2500 | Maximum pulse width (μs) |
| defaultAngle | 90 | Default startup angle (degrees) |

## Timer Configuration

The driver configures timers for 50Hz PWM (20ms period):
- Prescaler: 89 (90MHz → 1MHz timer clock)
- Period: 19999 (1MHz → 50Hz)
- Pulse calculation: Direct microsecond mapping

## Example Usage

```c
#include "servo.h"

// Global handles
SERVO_Handle_t hservo;
TIM_HandleTypeDef htim3;

void SERVO_Example(void)
{
    // Initialize timer (assumed configured elsewhere)
    // MX_TIM3_Init();

    // Initialize servo
    if (SERVO_Init(&hservo, &htim3, TIM_CHANNEL_1, GPIOB, GPIO_PIN_4) == SERVO_OK) {
        // Move to center
        SERVO_MoveToCenter(&hservo);
        HAL_Delay(1000);

        // Sweep motion
        SERVO_Sweep(&hservo, 20);
    }
}
```

## Error Handling

The driver returns status codes:
- `SERVO_OK`: Operation successful
- `SERVO_INVALID_PARAM`: Invalid parameter
- `SERVO_NOT_INITIALIZED`: Driver not initialized
- `SERVO_OUT_OF_RANGE`: Value out of configured range

## Integration Notes

1. Ensure timer is properly initialized before calling `SERVO_Init()`
2. GPIO alternate function must match timer channel
3. PWM frequency is fixed at 50Hz for standard servo compatibility
4. Calibration may be needed for precise angle control

## File Structure

```
Peripherals/SERVO/
├── servo.h          # Header file with API declarations
├── servo.c          # Implementation file
└── README.md        # This documentation
```
