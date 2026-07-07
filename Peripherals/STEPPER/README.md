# Stepper Motor Driver

This folder contains the driver for stepper motor control using GPIO pins and timer for precise timing on the STM32F429 Discovery board.

## Features

- Support for 4-wire bipolar stepper motors
- Multiple stepping modes (full step, half step, wave drive)
- Speed control in RPM
- Position tracking
- Acceleration/deceleration support
- Limit switch support
- Synchronous and continuous rotation modes

## Hardware Connections

The stepper motor requires 4 GPIO pins for coil control:

| Stepper Pin | STM32F429 Pin | Description |
|-------------|----------------|-------------|
| Coil A+    | PB12          | Coil A positive |
| Coil A-    | PB13          | Coil A negative |
| Coil B+    | PB14          | Coil B positive |
| Coil B-    | PB15          | Coil B negative |

Optional limit switches can be connected to any GPIO pins.

## Usage Example

```c
#include "stepper.h"

STEPPER_Handle_t hstep;
TIM_HandleTypeDef htim3;

// Initialize timer for timing (configure TIM3 externally)
TIM_Base_Init(&htim3);

// Get default pin configuration
STEPPER_Pins_t pins = STEPPER_GetDefaultPins();

// Initialize stepper motor
if (STEPPER_Init(&hstep, &htim3, &pins) == STEPPER_OK) {
    // Move 100 steps clockwise at 60 RPM
    STEPPER_MoveSteps(&hstep, 100, STEPPER_DIR_CW, 60);

    // Move to absolute position 500
    STEPPER_MoveToPosition(&hstep, 500, 120);

    // Start continuous rotation
    STEPPER_RotateContinuous(&hstep, STEPPER_DIR_CCW, 30);
}
```

## API Reference

### Initialization and Configuration

- `STEPPER_Init()` - Initialize stepper motor
- `STEPPER_DeInit()` - Deinitialize stepper motor
- `STEPPER_Config()` - Configure motor parameters

### Motion Control

- `STEPPER_MoveSteps()` - Move by relative steps
- `STEPPER_MoveToPosition()` - Move to absolute position
- `STEPPER_RotateContinuous()` - Continuous rotation
- `STEPPER_Stop()` - Stop motor
- `STEPPER_EmergencyStop()` - Emergency stop

### Status and Information

- `STEPPER_IsRunning()` - Check if motor is running
- `STEPPER_GetPosition()` - Get current position
- `STEPPER_SetPosition()` - Set current position
- `STEPPER_GetStatus()` - Get motor status

### Advanced Features

- `STEPPER_EnableLimitSwitches()` - Enable limit switches
- `STEPPER_Home()` - Home motor to limit switch
- `STEPPER_SetAcceleration()` - Set acceleration parameters

### Utility Functions

- `STEPPER_RPMToDelay()` - Convert RPM to step delay
- `STEPPER_DelayToRPM()` - Convert step delay to RPM
- `STEPPER_GetDefaultConfig()` - Get default configuration
- `STEPPER_GetDefaultPins()` - Get default pin configuration

## Stepping Modes

### Full Step (4 steps per revolution)
- Highest torque
- Simplest control
- 200 steps per revolution for standard motors

### Half Step (8 steps per revolution)
- Smoother motion
- Medium torque
- 400 steps per revolution

### Wave Drive (4 steps per revolution)
- Lowest power consumption
- Lower torque
- 200 steps per revolution

## Speed Control

- Speed range: 1-1000 RPM
- Step delay calculation: delay = (60,000,000) / (RPM × steps_per_rev)
- Acceleration/deceleration support for smooth motion

## Dependencies

- GPIO peripheral driver (`Peripherals/GPIO/`)
- TIM peripheral driver (`Peripherals/TIM/`)

## Notes

- Uses busy-wait delays for step timing (can be improved with timer interrupts)
- Position tracking is maintained in software
- Limit switch functionality requires external GPIO configuration
- Default pin configuration uses PB12-PB15
- Motor should be powered appropriately for the coil current requirements
