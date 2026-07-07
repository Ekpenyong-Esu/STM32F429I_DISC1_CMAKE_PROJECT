# Simplified LED Driver for STM32F429

This directory contains both the original comprehensive LED driver and a new **simplified version** that maintains essential functionality while being much easier to use and understand.

## Files Overview

### Original (Complex) Version
- `led.h` / `led.c` - Full-featured driver with extensive functionality (891 lines)
- Advanced features: PWM brightness control, breathing effects, custom patterns, multi-LED coordination

### Simplified Version
- `led_simple.h` / `led_simple.c` - Streamlined driver with core features only (~315 lines total)
- `led_simple_example.c` - Simple usage examples
- `README_LED_SIMPLE.md` - This documentation

## Why Use the Simplified Version?

The original LED driver includes many complex features that most projects don't need:

**Original Driver Complexity:**
- 891 lines in implementation file
- 355 lines in header file
- PWM brightness control with timer configuration
- Complex pattern system with custom steps
- Breathing and pulsing effects with sine wave calculations
- Multi-LED coordination functions
- Knight rider effects
- Mathematical calculations for lighting effects
- Complex state machines for pattern management

**Simplified Driver Benefits:**
- Only ~315 lines total (65% reduction)
- Essential functionality only:
  - Basic on/off control
  - Simple blinking with configurable period
  - Toggle functionality
  - State monitoring
- Easy to understand and modify
- No external timer dependencies
- Much faster compilation

## Quick Start

### 1. Basic Usage
```c
#include "led_simple.h"

LedHandle_t greenLed;

// Initialize green LED on STM32F429 Discovery
Led_InitGreen(&greenLed);

// Turn LED on
Led_On(&greenLed);

// Turn LED off  
Led_Off(&greenLed);

// Toggle LED
Led_Toggle(&greenLed);
```

### 2. Blinking LEDs
```c
LedHandle_t redLed;
Led_InitRed(&redLed);

// Start blinking (500ms period)
Led_StartBlink(&redLed, 500);

// In your main loop
while (1) {
    Led_Update(&redLed);  // Call regularly for blinking to work
    HAL_Delay(10);
}

// Stop blinking
Led_StopBlink(&redLed);
```

### 3. Custom Configuration
```c
LedConfig_t config = {
    .port = GPIOD,
    .pin = GPIO_PIN_5,
    .activeLow = false  // Active high LED
};

LedHandle_t customLed;
Led_InitCustom(&customLed, &config);
```

## API Reference

### Data Types
- `LedState_t` - LED_OFF or LED_ON
- `LedConfig_t` - Configuration structure
- `LedHandle_t` - LED instance handle

### Core Functions
- `Led_Init()` - Initialize with default settings (active low)
- `Led_InitCustom()` - Initialize with custom configuration
- `Led_On()` - Turn LED on
- `Led_Off()` - Turn LED off
- `Led_Toggle()` - Toggle current state
- `Led_SetState()` - Set to specific state
- `Led_GetState()` - Get current state
- `Led_IsOn()` - Check if LED is on

### Blinking Functions
- `Led_StartBlink()` - Start blinking with specified period
- `Led_StopBlink()` - Stop blinking
- `Led_Update()` - Update state (call regularly for blinking)
- `Led_IsBlinking()` - Check if currently blinking

### Convenience Functions
- `Led_InitGreen()` - Initialize STM32F429 Discovery green LED
- `Led_InitRed()` - Initialize STM32F429 Discovery red LED

## Default Configuration

- **Active Level**: Active low (LED on when pin is low)
- **GPIO Mode**: Push-pull output
- **Speed**: Low frequency
- **No PWM**: Simple digital on/off only

## Hardware Requirements

- Any GPIO pin on STM32F429
- LED connected between GPIO pin and VCC (for active low) or GND (for active high)
- Current limiting resistor as appropriate for your LED

## STM32F429 Discovery Board

The driver includes convenience functions for the on-board LEDs:
- Green LED: PG13 (active low)
- Red LED: PG14 (active low)

## Migration from Original Driver

| Original Driver | Simplified Driver |
|----------------|------------------|
| `LED_Init()` | `Led_Init()` |
| `LED_On()` | `Led_On()` |
| `LED_Off()` | `Led_Off()` |
| `LED_Toggle()` | `Led_Toggle()` |
| `LED_StartBlink()` | `Led_StartBlink()` |
| `LED_Update()` | `Led_Update()` |

**Not included in simplified version:**
- PWM brightness control
- Breathing and pulsing effects
- Custom pattern sequences
- Multi-LED coordination
- Knight rider effects
- Complex timing calculations
- HAL_StatusTypeDef returns (uses bool instead)

## When to Use Which Version?

### Use Simplified Version When:
- You need basic LED control
- Code size matters
- Simple blinking is sufficient
- Learning embedded programming
- Prototyping applications
- No PWM requirements

### Use Original Version When:
- You need brightness control (PWM)
- Complex lighting effects required
- Breathing or pulsing patterns needed
- Multi-LED coordination
- Advanced pattern sequences
- Professional lighting applications

## Example Usage Patterns

### Status Indicator
```c
LedHandle_t statusLed;
Led_InitGreen(&statusLed);

if (systemOK) {
    Led_On(&statusLed);
} else {
    Led_StartBlink(&statusLed, LED_BLINK_FAST);
}
```

### Heartbeat Monitor
```c
// In main loop
static uint32_t lastHeartbeat = 0;
if (HAL_GetTick() - lastHeartbeat > 1000) {
    Led_Toggle(&heartbeatLed);
    lastHeartbeat = HAL_GetTick();
}
```
