# STMPE811 Touchscreen Driver

This driver provides an interface for the STMPE811 resistive touchscreen controller found on the STM32F429I Discovery board.

## Hardware Overview

The STM32F429I Discovery board includes:
- **STMPE811** resistive touchscreen controller
- **240x320 pixel** resistive touchscreen display
- **I2C3** interface for communication
- **PA15** interrupt pin for touch detection

## Features

- ✅ Single-point touch detection
- ✅ Pressure sensitivity measurement
- ✅ Gesture recognition (tap, swipe, etc.)
- ✅ Calibration support
- ✅ Interrupt-driven operation
- ✅ FIFO buffer support
- ✅ Hardware filtering and averaging

## Pin Configuration

| Function | Pin | GPIO Port | Alternative Function |
|----------|-----|-----------|---------------------|
| I2C3_SCL | PA8 | GPIOA | AF4 |
| I2C3_SDA | PC9 | GPIOC | AF4 |
| Touch INT | PA15 | GPIOA | GPIO Input |

## Quick Start

### 1. Basic Initialization

```c
#include "touchscreen.h"

// Declare touchscreen handle
TS_HandleTypeDef hts;
I2C_HandleTypeDef hi2c3;

// Initialize I2C3 (in your main.c or init function)
void MX_I2C3_Init(void)
{
    hi2c3.Instance = I2C3;
    hi2c3.Init.ClockSpeed = 100000;
    hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c3);
}

// Initialize touchscreen
TS_StatusTypeDef status = TS_Init(&hts, &hi2c3);
if (status == TS_OK) {
    printf("Touchscreen initialized successfully\\n");
}
```

### 2. Touch Detection

```c
TS_TouchDataTypeDef touchData;

// Poll for touch data
if (TS_GetTouchData(&hts, &touchData) == TS_OK) {
    if (touchData.TouchCount > 0) {
        uint16_t x = touchData.Points[0].X;
        uint16_t y = touchData.Points[0].Y;
        uint16_t pressure = touchData.Points[0].Z;
        
        printf("Touch at (%d, %d) with pressure %d\\n", x, y, pressure);
    }
}
```

### 3. Interrupt-Driven Operation

```c
// Enable interrupts
TS_EnableIT(&hts);

// Set callbacks
hts.TouchCallback = MyTouchCallback;
hts.ReleaseCallback = MyReleaseCallback;
hts.GestureCallback = MyGestureCallback;

// In your interrupt handler (stm32f4xx_it.c)
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(TS_INT_PIN);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == TS_INT_PIN) {
        TS_IRQHandler(&hts);
    }
}

// Callback functions
void MyTouchCallback(void) {
    printf("Touch detected!\\n");
}

void MyGestureCallback(TS_GestureTypeDef gesture) {
    switch (gesture) {
        case TS_GESTURE_TAP:
            printf("Tap gesture\\n");
            break;
        case TS_GESTURE_SWIPE_UP:
            printf("Swipe up gesture\\n");
            break;
        // Handle other gestures...
    }
}
```

### 4. Calibration

```c
TS_CalibrationTypeDef calibration;

// Perform calibration (user must touch specific points)
if (TS_Calibrate(&hts, &calibration) == TS_OK) {
    printf("Calibration completed successfully\\n");
}

// Or set manual calibration values
calibration.MinX = 300;
calibration.MaxX = 3700;
calibration.MinY = 300;
calibration.MaxY = 3700;
calibration.ScaleX = (float)TS_DISPLAY_WIDTH / (calibration.MaxX - calibration.MinX);
calibration.ScaleY = (float)TS_DISPLAY_HEIGHT / (calibration.MaxY - calibration.MinY);
calibration.OffsetX = -calibration.MinX;
calibration.OffsetY = -calibration.MinY;
calibration.IsCalibrated = true;

TS_SetCalibration(&hts, &calibration);
```

## API Reference

### Initialization Functions

- `TS_Init(hts, hi2c)` - Initialize touchscreen controller
- `TS_DeInit(hts)` - Deinitialize touchscreen controller
- `TS_Reset(hts)` - Reset touchscreen controller

### Touch Data Functions

- `TS_GetTouchData(hts, data)` - Get current touch data
- `TS_IsTouched(hts)` - Check if screen is currently touched
- `TS_GetGesture(hts, gesture)` - Get detected gesture

### Configuration Functions

- `TS_SetConfig(hts, config)` - Set touchscreen configuration
- `TS_GetConfig(hts, config)` - Get current configuration
- `TS_SetPressureThreshold(hts, threshold)` - Set pressure threshold

### Calibration Functions

- `TS_Calibrate(hts, calibration)` - Perform interactive calibration
- `TS_SetCalibration(hts, calibration)` - Set calibration data
- `TS_GetCalibration(hts, calibration)` - Get current calibration

### Interrupt Functions

- `TS_EnableIT(hts)` - Enable touchscreen interrupts
- `TS_DisableIT(hts)` - Disable touchscreen interrupts
- `TS_IRQHandler(hts)` - Handle touchscreen interrupt

## Configuration Options

### Sample Time Settings
- `STMPE811_TSC_CFG_1_SAMPLE` - 1 sample per measurement
- `STMPE811_TSC_CFG_2_SAMPLE` - 2 samples averaged
- `STMPE811_TSC_CFG_4_SAMPLE` - 4 samples averaged
- `STMPE811_TSC_CFG_8_SAMPLE` - 8 samples averaged

### Touch Detection Delay
- `STMPE811_TSC_CFG_DELAY_10US` to `STMPE811_TSC_CFG_DELAY_50MS`

### Panel Driver Settling Time
- `STMPE811_TSC_CFG_SETTLE_10US` to `STMPE811_TSC_CFG_SETTLE_100MS`

## Gestures Supported

- `TS_GESTURE_TAP` - Single tap
- `TS_GESTURE_DOUBLE_TAP` - Double tap
- `TS_GESTURE_LONG_PRESS` - Long press (hold)
- `TS_GESTURE_SWIPE_UP` - Swipe up
- `TS_GESTURE_SWIPE_DOWN` - Swipe down
- `TS_GESTURE_SWIPE_LEFT` - Swipe left
- `TS_GESTURE_SWIPE_RIGHT` - Swipe right

## Examples

The `touchscreen_example.c` file provides comprehensive examples:

1. **Basic Touch Test** - Simple touch detection
2. **Touch Drawing** - Drawing application
3. **Calibration Test** - Interactive calibration
4. **Gesture Detection** - Gesture recognition demo
5. **Simple Menu** - Touch-based menu navigation
6. **Diagnostic Test** - Hardware diagnostics

## Troubleshooting

### Common Issues

1. **No Touch Detection**
   - Check I2C3 connections (PA8, PC9)
   - Verify device ID with `TS_ReadRegister(STMPE811_CHIP_ID, &id)`
   - Ensure proper power supply

2. **Inaccurate Touch Position**
   - Perform calibration with `TS_Calibrate()`
   - Check pressure threshold settings
   - Verify display coordinate mapping

3. **Missed Touches**
   - Enable interrupts for better responsiveness
   - Adjust pressure threshold
   - Check sampling configuration

4. **I2C Communication Errors**
   - Verify I2C3 clock configuration
   - Check pull-up resistors on I2C lines
   - Ensure correct device address (0x82)

### Debug Information

Enable debug output to monitor touchscreen operation:

```c
// In your configuration
#define TS_DEBUG_ENABLE 1

// This will print I2C transactions and touch events
```

## Hardware Specifications

- **Resolution**: 4096 x 4096 (12-bit ADC)
- **Display Mapping**: 240 x 320 pixels
- **Pressure Levels**: 4096 levels (12-bit)
- **Sample Rate**: Up to 80 Hz (configurable)
- **Power Supply**: 3.3V
- **Interface**: I2C (up to 400 kHz)

## Notes

- The STMPE811 is a resistive touchscreen controller, not capacitive
- Single-touch only (multi-touch not supported by hardware)
- Pressure sensitivity available for advanced applications
- Gesture recognition is implemented in software
- Calibration is recommended for accurate touch positioning
