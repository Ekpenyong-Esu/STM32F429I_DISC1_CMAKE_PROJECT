# GPIO (General Purpose Input/Output) Driver

This directory contains the GPIO driver implementation for STM32F429 microcontroller, following STM32 HAL library best practices. The GPIO peripheral provides digital input/output functionality for controlling LEDs, reading buttons, and interfacing with external devices.

## Files

- `gpio.h` - Header file with function prototypes and data structures
- `gpio.c` - Complete GPIO implementation including initialization, LED control, button handling, and interrupt management
- `gpio_example.h` - Example usage header file
- `gpio_example.c` - Comprehensive example functions
- `README.md` - This documentation file

## Key Features

- **HAL Best Practices Compliance**: All functions follow STM32 HAL naming conventions and patterns
- **Consolidated Implementation**: Single `gpio.c` file contains all GPIO functionality
- **LED Control**: Easy-to-use functions for controlling LD3 and LD4 LEDs
- **Button Handling**: Support for both polling and interrupt-based button reading
- **Debouncing**: Software debouncing for reliable button press detection
- **Interrupt Support**: EXTI interrupt handling for PA0 button
- **Press Counting**: Shared button press counting across interrupt and polling modes
- **Comprehensive Examples**: Multiple example functions demonstrating usage

## API Functions

### Initialization Functions
- `GPIO_Init()` - Initialize all GPIO pins used in the application
- `GPIO_PA0_Button_Init()` - Initialize PA0 button for interrupt or polling mode
- `initialize_gpio_interrupts()` - Initialize GPIO interrupts

### LED Control Functions
- `GPIO_LED_LD3_Set()` - Set LD3 LED state (ON/OFF)
- `GPIO_LED_LD4_Set()` - Set LD4 LED state (ON/OFF)
- `GPIO_LED_LD3_Toggle()` - Toggle LD3 LED state
- `GPIO_LED_LD4_Toggle()` - Toggle LD4 LED state

### Button Functions
- `GPIO_Button_B1_GetState()` - Get current state of B1 button
- `GPIO_Button_B1_IsPressed()` - Check if button is pressed (with debouncing)
- `GPIO_Button_Callback()` - Button interrupt callback handler

## Usage Examples

### Basic LED Control

```c
#include "gpio.h"

// Initialize GPIO
GPIO_Init();

// Turn on LD3
GPIO_LED_LD3_Set(GPIO_PIN_SET);

// Toggle LD4
GPIO_LED_LD4_Toggle();

// Turn off both LEDs
GPIO_LED_LD3_Set(GPIO_PIN_RESET);
GPIO_LED_LD4_Set(GPIO_PIN_RESET);
```

### Button Polling

```c
#include "gpio.h"

// Initialize GPIO
GPIO_Init();

// Configure button for polling
GPIO_PA0_Button_Init(POLLING_MODE);

// Check button state
if (GPIO_Button_B1_IsPressed(50)) {  // 50ms debounce
    // Button was pressed
    GPIO_LED_LD3_Toggle();
}
```

### Button Interrupts

```c
#include "gpio.h"

// Initialize GPIO interrupts
initialize_gpio_interrupts();

// The callback function will be called automatically
void GPIO_Button_Callback(void) {
    GPIO_LED_LD3_Toggle();
}
```

### Running Examples

```c
#include "gpio_example.h"

// Run all GPIO examples
HAL_StatusTypeDef status = GPIO_Example();

// Run specific examples
status = GPIO_LED_Example();
status = GPIO_Button_Example();
status = GPIO_Interrupt_Example();
```

## Pin Configuration

### LEDs
- **LD3**: PG13 (Green LED)
- **LD4**: PG14 (Red LED)

### Buttons
- **B1**: PA0 (User button with interrupt capability)

### Other Pins
The GPIO driver also configures pins for:
- SPI communication (MEMS sensor)
- LCD control signals
- USB OTG functionality
- Audio codec control
- SDMMC interface
- Ethernet and camera interfaces

## Interrupt Configuration

The driver supports EXTI interrupts on PA0 (B1 button):
- **Interrupt Mode**: Button press triggers interrupt
- **Polling Mode**: Button state is read manually

### Interrupt Priority
- EXTI0_IRQn: Priority 5, Sub-priority 0

## Best Practices Implemented

1. **HAL Integration**: Uses HAL functions for GPIO operations
2. **Error Handling**: Proper parameter validation and error checking
3. **Debouncing**: Software debouncing for reliable button detection
4. **Documentation**: Complete function and parameter documentation
5. **Modular Design**: Separate functions for different GPIO operations
6. **Interrupt Safety**: Proper interrupt handling and callback management

## Hardware Configuration

### GPIO Ports Used
- **GPIOA**: Buttons, audio, SPI, UART
- **GPIOB**: USB, SDMMC, LCD
- **GPIOC**: SPI, USB, LCD, RCC
- **GPIOD**: LCD, SDMMC
- **GPIOE**: LCD, SDMMC
- **GPIOF**: FMC, SPI, LCD
- **GPIOG**: LEDs, FMC, LCD
- **GPIOH**: RCC oscillators

### Clock Configuration
All required GPIO peripheral clocks are enabled in `GPIO_Init()`:
- `__HAL_RCC_GPIOA_CLK_ENABLE()`
- `__HAL_RCC_GPIOB_CLK_ENABLE()`
- `__HAL_RCC_GPIOC_CLK_ENABLE()`
- `__HAL_RCC_GPIOD_CLK_ENABLE()`
- `__HAL_RCC_GPIOE_CLK_ENABLE()`
- `__HAL_RCC_GPIOF_CLK_ENABLE()`
- `__HAL_RCC_GPIOG_CLK_ENABLE()`
- `__HAL_RCC_GPIOH_CLK_ENABLE()`

## Example Applications

### LED Blinking
```c
while (1) {
    GPIO_LED_LD3_Toggle();
    HAL_Delay(500);
}
```

### Button-Controlled LED
```c
while (1) {
    if (GPIO_Button_B1_IsPressed(50)) {
        GPIO_LED_LD4_Toggle();
    }
    HAL_Delay(10);
}
```

### Interrupt-Driven LED Toggle
```c
// Initialize interrupts
initialize_gpio_interrupts();

// Main loop - LED toggling handled by interrupt
while (1) {
    // Other application code
    HAL_Delay(100);
}
```

## Dependencies

- STM32F4xx HAL Library
- CMSIS Core headers
- `main.h` for pin definitions

## Notes

- This driver is specifically designed for STM32F429 Discovery board
- GPIO initialization configures all pins used in the application
- Button debouncing prevents false triggers from contact bounce
- Interrupt mode provides low-latency response to button presses
- All functions are thread-safe for use with FreeRTOS

## Error Handling

The driver includes proper error handling:
- **Parameter Validation**: Input parameters are validated
- **HAL Status Returns**: Functions return HAL_StatusTypeDef
- **Interrupt Safety**: Interrupt handlers are designed to be safe

## Testing

The `gpio_example.c` file provides comprehensive tests:
- LED control verification
- Button polling functionality
- Interrupt handling validation
- Debouncing effectiveness testing

Run the examples to verify GPIO functionality on your hardware.
