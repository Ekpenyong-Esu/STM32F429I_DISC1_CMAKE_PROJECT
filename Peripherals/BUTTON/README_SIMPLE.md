# Simplified Button Driver for STM32F429

This directory contains both the original comprehensive button driver and a new **simplified version** that maintains essential functionality while being much easier to use and understand.

## Files Overview

### Original (Complex) Version
- `button.h` / `button.c` - Full-featured driver with extensive functionality
- `README.md` - Complete documentation for original driver

### Simplified Version
- `button_simple.h` / `button_simple.c` - Streamlined driver with core features only
- `button_simple_example.c` - Simple usage examples
- `README_SIMPLE.md` - This documentation

## Why Use the Simplified Version?

The original button driver, while comprehensive, includes many features that most projects don't need:

**Original Driver Complexity:**
- 874 lines of code in .c file
- 384 lines in header file  
- Complex state machines with 6 internal states
- Double-click detection
- Long press detection with configurable timing
- Statistics tracking
- Extensive error handling with 6 different error types
- Event callback system
- EXTI interrupt support
- Multiple timing parameters to configure

**Simplified Driver Benefits:**
- Only ~200 lines of code total
- Easy to understand and modify
- Essential functionality only:
  - Basic button reading with debouncing
  - Press/release detection
  - Edge detection (just pressed/released)
  - Configurable active level and debounce time
- Much faster compilation and smaller code footprint

## Quick Start

### 1. Basic Usage (Default Settings)
```c
#include "button_simple.h"

ButtonHandle_t button;

// Initialize button on PA0 (STM32F429 Discovery user button)
Button_Init(&button, GPIOA, GPIO_PIN_0);

// In your main loop
while (1) {
    if (Button_WasPressed(&button)) {
        // Button was just pressed
        printf("Button pressed!\n");
    }
    
    if (Button_IsPressed(&button)) {
        // Button is currently held down
        // Do something while pressed
    }
    
    HAL_Delay(10);
}
```

### 2. Custom Configuration
```c
ButtonConfig_t config = {
    .port = GPIOB,
    .pin = GPIO_PIN_1,
    .activeLow = false,      // Active high button
    .debounceMs = 30         // 30ms debounce
};

ButtonHandle_t button;
Button_InitCustom(&button, &config);
```

## API Reference

### Data Types
- `ButtonState_t` - BUTTON_RELEASED or BUTTON_PRESSED
- `ButtonConfig_t` - Configuration structure
- `ButtonHandle_t` - Button instance handle

### Functions
- `Button_Init()` - Initialize with default settings (active low, 50ms debounce)
- `Button_InitCustom()` - Initialize with custom configuration
- `Button_Read()` - Read current state (with debouncing)
- `Button_IsPressed()` - Check if currently pressed
- `Button_WasPressed()` - Check if just pressed (edge detection)
- `Button_WasReleased()` - Check if just released (edge detection)
- `Button_ReadRaw()` - Read without debouncing

## Default Configuration

- **Active Level**: Active low (pressed when pin reads 0V)
- **Pull Resistor**: Pull-up for active low, pull-down for active high
- **Debounce Time**: 50ms
- **No interrupts**: Polling only for simplicity

## Hardware Requirements

- Any GPIO pin on STM32F429
- External button connected between GPIO pin and GND (for active low)
- Internal pull-up resistor is automatically enabled

## Migration from Original Driver

If you're currently using the complex driver, here's how the functions map:

| Original Driver | Simplified Driver |
|----------------|------------------|
| `BUTTON_Init()` | `Button_Init()` |
| `BUTTON_IsPressed()` | `Button_IsPressed()` |
| `BUTTON_Process()` | Called automatically in `Button_Read()` |
| `BUTTON_GetState()` | `Button_Read()` |

**Not included in simplified version:**
- Double-click detection
- Long press detection  
- Event callbacks
- Statistics
- Error codes (functions return bool instead)
- Interrupt support

## When to Use Which Version?

### Use Simplified Version When:
- You need basic button functionality
- Code size and complexity matter
- Learning or prototyping
- Most typical applications

### Use Original Version When:
- You need double-click or long press detection
- You want interrupt-driven operation
- You need statistics or advanced error handling
- Building a commercial product with comprehensive requirements
