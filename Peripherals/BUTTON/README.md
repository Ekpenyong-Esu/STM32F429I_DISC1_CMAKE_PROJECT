# Button Driver for STM32F429

This directory contains a comprehensive button driver implementation for the STM32F429 microcontroller, supporting GPIO-based button input with advanced features like debouncing, event detection, and interrupt handling.

## Features

- **Flexible Configuration**: Support for different GPIO pins, pull resistors, and active levels
- **Debouncing**: Hardware and software debouncing with configurable timing
- **Event Detection**: Click, double-click, long press, and release events
- **Interrupt Support**: Optional EXTI interrupt handling for low-power operation
- **Polling Mode**: Alternative polling-based operation for simple applications
- **State Machine**: Robust state machine for reliable button state tracking
- **Callback System**: Event-driven programming with user callbacks
- **Statistics**: Usage statistics and performance monitoring
- **Comprehensive Examples**: Multiple example functions demonstrating all features

## Files

- `button.h` - Main header file with function prototypes and type definitions
- `button.c` - Core button driver implementation
- `button_example.c` - Example functions demonstrating usage
- `README.md` - Complete documentation and usage guide

## Supported Button Types

- **Momentary Buttons**: Standard push buttons
- **Active Low/High**: Configurable active level (pressed when low or high)
- **Pull-up/Pull-down**: Internal resistor configuration
- **Interrupt/Polling**: Both interrupt-driven and polling modes

## Button Events

The driver detects the following button events:

- `BUTTON_EVENT_PRESSED` - Button pressed (after debounce)
- `BUTTON_EVENT_RELEASED` - Button released
- `BUTTON_EVENT_CLICK` - Single click (press + release)
- `BUTTON_EVENT_DOUBLE_CLICK` - Double click (two quick presses)
- `BUTTON_EVENT_LONG_PRESS` - Long press started
- `BUTTON_EVENT_LONG_RELEASE` - Long press released

## Usage Examples

### Basic Initialization
```c
#include "button.h"

BUTTON_HandleTypeDef buttonHandle;

// Initialize with default settings
BUTTON_StatusTypeDef status = BUTTON_Init(&buttonHandle, GPIOA, GPIO_PIN_0);
if (status != BUTTON_OK) {
    // Handle error
}
```

### Custom Configuration
```c
BUTTON_ConfigTypeDef config = {
    .Port = GPIOA,
    .Pin = GPIO_PIN_0,
    .Pull = BUTTON_PULL_UP,
    .ActiveLevel = BUTTON_ACTIVE_LOW,
    .EnableInterrupt = true,
    .DebounceTime = 50,      // 50ms debounce
    .LongPressTime = 1000,   // 1 second long press
    .DoubleClickTime = 300   // 300ms double click window
};

BUTTON_StatusTypeDef status = BUTTON_Init_Custom(&buttonHandle, &config);
```

### Event-Driven Usage
```c
// Define callback function
void ButtonCallback(BUTTON_EventTypeDef event) {
    switch (event) {
        case BUTTON_EVENT_CLICK:
            printf("Button clicked!\n");
            break;
        case BUTTON_EVENT_DOUBLE_CLICK:
            printf("Button double-clicked!\n");
            break;
        case BUTTON_EVENT_LONG_PRESS:
            printf("Button long pressed!\n");
            break;
    }
}

// Register callback
BUTTON_RegisterCallback(&buttonHandle, ButtonCallback);
```

### Polling Mode
```c
// In main loop
while (1) {
    // Process button state
    BUTTON_Process(&buttonHandle);

    // Check for events
    BUTTON_EventTypeDef event;
    BUTTON_GetLastEvent(&buttonHandle, &event);

    if (event == BUTTON_EVENT_CLICK) {
        // Handle click
    }

    HAL_Delay(10); // Small delay
}
```

## Configuration Options

### Timing Parameters
- **Debounce Time**: 10-200ms (default: 50ms)
- **Long Press Time**: 500-5000ms (default: 1000ms)
- **Double Click Time**: 100-1000ms (default: 300ms)

### GPIO Configuration
- **Port**: Any GPIO port (GPIOA-GPIOE)
- **Pin**: Any GPIO pin
- **Pull**: None, Pull-up, or Pull-down
- **Active Level**: Low (pressed = 0) or High (pressed = 1)

### Operation Mode
- **Interrupt Mode**: Uses EXTI interrupts (lower power)
- **Polling Mode**: Manual processing in main loop

## Pin Configuration

### STM32F429 Discovery Board
```c
// User button (B1) - PA0
#define BUTTON_USER_PORT    GPIOA
#define BUTTON_USER_PIN     GPIO_PIN_0
#define BUTTON_USER_IRQn    EXTI0_IRQn
```

### Custom Button Setup
```c
// Example: Button on PC13
#define BUTTON_CUSTOM_PORT  GPIOC
#define BUTTON_CUSTOM_PIN   GPIO_PIN_13
```

## State Machine

The button driver uses a robust state machine:

```
IDLE ────────────── Press ───────────► DEBOUNCE
│                                           │
│              Not pressed during debounce  │
└───────────────────────────────────────────┘
                        │
                        ▼
              Pressed during debounce ───► PRESSED
                        │
                        ▼
           Not pressed within long press time ───► WAIT_DOUBLE_CLICK
                        │
                        ▼
              Pressed within double click window ───► DOUBLE_CLICK
                        │
                        ▼
           Long press time exceeded ───► LONG_PRESS
                        │
                        ▼
                    Released ───► LONG_RELEASE
```

## Interrupt Handling

### EXTI Configuration
When interrupts are enabled, the driver configures EXTI lines:

```c
// Automatic EXTI setup (handled by driver)
EXTI_ConfigTypeDef extiConfig = {
    .Line = EXTI_LINE,
    .Mode = EXTI_MODE_INTERRUPT,
    .Trigger = EXTI_TRIGGER_RISING_FALLING,
    .GPIOSel = GPIO_PORT_SOURCE
};
```

### Interrupt Handler
```c
// In stm32f4xx_it.c
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

// EXTI callback (registered by driver)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        // Button interrupt occurred
        BUTTON_Process(&buttonHandle);
    }
}
```

## Debouncing

The driver implements multi-level debouncing:

1. **Hardware Debouncing**: External RC circuit (recommended)
2. **Software Debouncing**: Timer-based filtering
3. **State Machine**: Prevents false transitions

### Debounce Algorithm
```c
if (button_pressed) {
    if (debounce_timer == 0) {
        start_debounce_timer();
    } else if (debounce_timer >= DEBOUNCE_TIME) {
        button_state = PRESSED;
        reset_debounce_timer();
    }
}
```

## Event Detection

### Click Detection
- Press + Release within normal time
- No long press detected

### Double Click Detection
- Two clicks within double-click window
- Each click must be valid (not long press)

### Long Press Detection
- Press duration exceeds long press threshold
- Separate events for start and release

## Performance Considerations

- **Memory Usage**: ~100 bytes per button instance
- **CPU Usage**: Minimal in interrupt mode, ~1-2% in polling mode
- **Response Time**: Debounce time + processing time
- **Power Consumption**: Lower in interrupt mode

## Error Handling

The driver provides comprehensive error handling:

- `BUTTON_OK` - Operation successful
- `BUTTON_ERROR` - General error
- `BUTTON_BUSY` - Driver busy
- `BUTTON_TIMEOUT` - Operation timeout
- `BUTTON_INVALID_PARAM` - Invalid parameter
- `BUTTON_NOT_INITIALIZED` - Button not initialized

## Example Functions

The `button_example.c` file contains several example functions:

- `BUTTON_Example_BasicInit()` - Basic initialization
- `BUTTON_Example_CustomConfig()` - Custom configuration
- `BUTTON_Example_PollingMode()` - Polling mode operation
- `BUTTON_Example_TimingConfig()` - Timing parameter configuration
- `BUTTON_Example_EventDemo()` - Event demonstration
- `BUTTON_Example_StateMonitor()` - State monitoring
- `BUTTON_Example_RunAll()` - Complete demonstration

## Integration with Main Application

Add the following to your main.c:

```c
#include "button.h"
#include "button_example.h"

// Global button handle
BUTTON_HandleTypeDef userButton;

// In main function
int main(void) {
    // Initialize HAL and system clocks
    HAL_Init();
    SystemClock_Config();

    // Initialize button
    BUTTON_Init(&userButton, GPIOA, GPIO_PIN_0);

    // Register callback
    BUTTON_RegisterCallback(&userButton, ButtonCallback);

    // Main loop
    while (1) {
        // Process button in polling mode (if not using interrupts)
        BUTTON_Process(&userButton);

        // Other application code
    }
}

// Button callback function
void ButtonCallback(BUTTON_EventTypeDef event) {
    switch (event) {
        case BUTTON_EVENT_CLICK:
            // Handle click
            break;
        case BUTTON_EVENT_DOUBLE_CLICK:
            // Handle double click
            break;
        case BUTTON_EVENT_LONG_PRESS:
            // Handle long press
            break;
    }
}
```

## Hardware Considerations

### Button Circuit
```
VCC ───┬─── Button ─── GND
       │
       └─────── GPIO Pin (with pull-up)
```

### External Debouncing
```
VCC ───┬─── Button ───┬─── GPIO Pin
       │              │
       ├─[10kΩ]───────┘
       │
       └─[0.1µF]─── GND
```

## Troubleshooting

### Common Issues
1. **Bouncing**: Increase debounce time or add hardware debouncing
2. **False Events**: Check active level configuration
3. **No Events**: Verify GPIO pin and pull resistor settings
4. **Interrupt Issues**: Check EXTI configuration and NVIC settings

### Debug Information
Use these functions for debugging:
- `BUTTON_GetState()` - Check current button state
- `BUTTON_GetLastEvent()` - Check last detected event
- `BUTTON_GetStatus()` - Check driver status
- `BUTTON_GetStatistics()` - Monitor usage statistics

## Future Enhancements

- Support for multiple buttons with single handle
- Analog button support (resistive ladder)
- Capacitive touch button support
- Button matrix support
- Advanced gesture recognition
- Power management integration

## Dependencies

- STM32F4xx HAL library
- CMSIS core
- GPIO and EXTI peripherals

## License

This button driver is provided as-is for educational and development purposes.
