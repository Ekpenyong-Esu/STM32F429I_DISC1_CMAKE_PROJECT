# LED Driver for STM32F429 Discovery Board

This driver provides comprehensive control for LEDs on the STM32F429 Discovery board, including basic on/off operations, brightness control via PWM, various blinking patterns, breathing effects, and coordinated multi-LED operations.

## Features

- **Basic LED Control**: On/off operations and toggle functionality
- **Brightness Control**: PWM-based brightness adjustment (0-100%)
- **Blinking Patterns**: Multiple predefined patterns (slow, fast, double, triple, heartbeat)
- **Breathing Effect**: Smooth sine wave brightness modulation
- **Custom Patterns**: User-defined sequences with timing control
- **Multi-LED Support**: Coordinated effects across multiple LEDs
- **Error Handling**: Comprehensive error detection and reporting
- **Performance Optimized**: Efficient update mechanisms for real-time operation

## Hardware Configuration

### STM32F429 Discovery Board LEDs

| LED   | GPIO Port | GPIO Pin | Color | Location          |
|-------|-----------|----------|-------|-------------------|
| LED3  | GPIOG     | Pin 13   | Green | Top-left corner   |
| LED4  | GPIOG     | Pin 14   | Red   | Top-right corner  |

### GPIO Configuration

```c
// Enable GPIO clock (usually done in system initialization)
__HAL_RCC_GPIOG_CLK_ENABLE();

// GPIO is configured automatically by the driver
// - Mode: GPIO_MODE_OUTPUT_PP (Push-Pull)
// - Pull: GPIO_NOPULL
// - Speed: GPIO_SPEED_FREQ_LOW
```

### PWM Configuration (Optional)

For brightness control, connect LEDs to timer PWM channels:

```c
// Example timer configuration for PWM
TIM_HandleTypeDef htim3;

// Timer initialization (handle this in your main initialization)
htim3.Instance = TIM3;
htim3.Init.Prescaler = 83;  // For 1MHz timer frequency
htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
htim3.Init.Period = 999;    // For 1kHz PWM frequency
htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
HAL_TIM_PWM_Init(&htim3);
```

## Quick Start

### 1. Basic Usage

```c
#include "led.h"

int main(void) {
    // System initialization...
    HAL_Init();
    SystemClock_Config();
    
    // Initialize LED
    LED_Handle_t greenLED;
    LED_Config_t config;
    
    // Configure green LED
    config.port = GPIOG;
    config.pin = GPIO_PIN_13;
    config.invertLogic = false;
    config.pwmEnabled = false;
    config.htim = NULL;
    config.channel = 0;
    
    LED_Init(&greenLED, &config);
    
    // Turn LED on
    LED_On(&greenLED);
    HAL_Delay(1000);
    
    // Turn LED off
    LED_Off(&greenLED);
    
    while (1) {
        // Main loop
    }
}
```

### 2. Using Multiple LEDs

```c
#include "led.h"

int main(void) {
    // System initialization...
    
    // Initialize all LEDs
    LED_Handle_t ledHandles[LED_COUNT];
    LED_InitAll(ledHandles);
    
    // Turn all LEDs on
    LED_AllOn(ledHandles);
    HAL_Delay(1000);
    
    // Turn all LEDs off
    LED_AllOff(ledHandles);
    
    while (1) {
        // Update all LEDs (call periodically for patterns)
        LED_UpdateAll(ledHandles);
        HAL_Delay(10);
    }
}
```

### 3. Blinking Patterns

```c
// Set predefined pattern
LED_SetPattern(&greenLED, LED_PATTERN_FAST_BLINK);

// Custom blink timing
LED_StartBlink(&greenLED, 1000, 50); // 1000ms period, 50% duty cycle

// In main loop
while (1) {
    LED_Update(&greenLED);
    HAL_Delay(10);
}
```

### 4. PWM Brightness Control

```c
// Configure LED with PWM support
LED_Config_t config;
config.port = GPIOG;
config.pin = GPIO_PIN_13;
config.pwmEnabled = true;
config.htim = &htim3;
config.channel = TIM_CHANNEL_1;

LED_Init(&greenLED, &config);

// Set brightness levels
LED_SetBrightness(&greenLED, 25);   // 25% brightness
LED_On(&greenLED);

// Fade effect
for (uint8_t brightness = 0; brightness <= 100; brightness += 5) {
    LED_SetBrightness(&greenLED, brightness);
    LED_On(&greenLED);
    HAL_Delay(100);
}
```

### 5. Custom Patterns

```c
// Define custom SOS pattern
LED_PatternStep_t sosSteps[] = {
    // S - short blinks
    {100, 200}, {0, 200}, {100, 200}, {0, 200}, {100, 200}, {0, 400},
    // O - long blinks  
    {100, 600}, {0, 200}, {100, 600}, {0, 200}, {100, 600}, {0, 400},
    // S - short blinks
    {100, 200}, {0, 200}, {100, 200}, {0, 200}, {100, 200}, {0, 2000}
};

LED_CustomPattern_t sosPattern = {
    .steps = sosSteps,
    .stepCount = sizeof(sosSteps) / sizeof(LED_PatternStep_t),
    .repeat = true
};

LED_SetCustomPattern(&greenLED, &sosPattern);

// Update in main loop
while (1) {
    LED_Update(&greenLED);
    HAL_Delay(10);
}
```

## API Reference

### Data Structures

#### LED_Config_t
Configuration structure for LED initialization.

```c
typedef struct {
    GPIO_TypeDef *port;        // GPIO port (e.g., GPIOG)
    uint16_t pin;              // GPIO pin (e.g., GPIO_PIN_13)
    bool invertLogic;          // true if LED is active low
    bool pwmEnabled;           // Enable PWM brightness control
    TIM_HandleTypeDef *htim;   // Timer handle for PWM (if enabled)
    uint32_t channel;          // Timer channel for PWM (if enabled)
} LED_Config_t;
```

#### LED_Handle_t
LED driver handle structure (internal - do not modify directly).

#### LED_PatternStep_t
Single step in a custom pattern.

```c
typedef struct {
    uint8_t brightness;        // Brightness level (0-100)
    uint16_t duration;         // Step duration in milliseconds
} LED_PatternStep_t;
```

#### LED_CustomPattern_t
Custom pattern definition.

```c
typedef struct {
    LED_PatternStep_t *steps;  // Array of pattern steps
    uint16_t stepCount;        // Number of steps
    bool repeat;               // true to repeat pattern
} LED_CustomPattern_t;
```

### Enumerations

#### LED_State_t
```c
typedef enum {
    LED_STATE_OFF,             // LED is off
    LED_STATE_ON,              // LED is on
    LED_STATE_BLINKING,        // LED is blinking
    LED_STATE_BREATHING,       // LED is breathing
    LED_STATE_PULSING,         // LED is pulsing (custom pattern)
    LED_STATE_FADING           // LED is fading
} LED_State_t;
```

#### LED_Pattern_t
```c
typedef enum {
    LED_PATTERN_OFF,           // Off
    LED_PATTERN_ON,            // Solid on
    LED_PATTERN_SLOW_BLINK,    // Slow blink (1 Hz)
    LED_PATTERN_FAST_BLINK,    // Fast blink (5 Hz)
    LED_PATTERN_DOUBLE_BLINK,  // Double blink
    LED_PATTERN_TRIPLE_BLINK,  // Triple blink
    LED_PATTERN_HEARTBEAT,     // Heartbeat pattern
    LED_PATTERN_BREATHING,     // Breathing effect
    LED_PATTERN_CUSTOM         // Custom pattern
} LED_Pattern_t;
```

#### LED_ID_t
```c
typedef enum {
    LED_GREEN = 0,             // Green LED (PG13)
    LED_RED = 1                // Red LED (PG14)
} LED_ID_t;
```

### Core Functions

#### Initialization
```c
HAL_StatusTypeDef LED_Init(LED_Handle_t *handle, LED_Config_t *config);
HAL_StatusTypeDef LED_DeInit(LED_Handle_t *handle);
```

#### Basic Control
```c
HAL_StatusTypeDef LED_On(LED_Handle_t *handle);
HAL_StatusTypeDef LED_Off(LED_Handle_t *handle);
HAL_StatusTypeDef LED_Toggle(LED_Handle_t *handle);
```

#### Brightness Control
```c
HAL_StatusTypeDef LED_SetBrightness(LED_Handle_t *handle, uint8_t brightness);
uint8_t LED_GetBrightness(LED_Handle_t *handle);
```

#### Pattern Control
```c
HAL_StatusTypeDef LED_SetPattern(LED_Handle_t *handle, LED_Pattern_t pattern);
HAL_StatusTypeDef LED_SetCustomPattern(LED_Handle_t *handle, LED_CustomPattern_t *customPattern);
HAL_StatusTypeDef LED_StartBlink(LED_Handle_t *handle, uint16_t periodMs, uint8_t dutyCycle);
HAL_StatusTypeDef LED_StopBlink(LED_Handle_t *handle);
HAL_StatusTypeDef LED_StartBreathing(LED_Handle_t *handle, uint16_t periodMs);
```

#### State Management
```c
HAL_StatusTypeDef LED_Update(LED_Handle_t *handle);
LED_State_t LED_GetState(LED_Handle_t *handle);
bool LED_IsOn(LED_Handle_t *handle);
```

#### Error Handling
```c
uint32_t LED_GetError(LED_Handle_t *handle);
HAL_StatusTypeDef LED_ClearError(LED_Handle_t *handle);
```

### Multi-LED Functions

```c
HAL_StatusTypeDef LED_InitAll(LED_Handle_t handles[LED_COUNT]);
HAL_StatusTypeDef LED_AllOn(LED_Handle_t handles[LED_COUNT]);
HAL_StatusTypeDef LED_AllOff(LED_Handle_t handles[LED_COUNT]);
HAL_StatusTypeDef LED_SetAllPattern(LED_Handle_t handles[LED_COUNT], LED_Pattern_t pattern);
HAL_StatusTypeDef LED_UpdateAll(LED_Handle_t handles[LED_COUNT]);
HAL_StatusTypeDef LED_AlternatingBlink(LED_Handle_t handles[LED_COUNT], uint16_t periodMs);
HAL_StatusTypeDef LED_KnightRider(LED_Handle_t handles[LED_COUNT], uint16_t periodMs);
```

### Utility Functions

```c
HAL_StatusTypeDef LED_GetConfigFromID(LED_ID_t ledId, LED_Config_t *config);
uint8_t LED_CalculateBreathingBrightness(uint32_t time, uint16_t period);
uint8_t LED_CalculateHeartbeatBrightness(uint32_t time, uint16_t period);
```

## Examples

### Running Examples

```c
#include "led_example.h"

int main(void) {
    // System initialization...
    
    // Initialize examples
    LED_ExamplesInit();
    
    // Run specific examples
    LED_BasicExample();
    LED_BlinkingExample();
    LED_BreathingExample();
    LED_MultiLEDExample();
    LED_CustomPatternExample();
    LED_BrightnessExample();
    LED_StatusIndicatorExample();
    LED_InteractiveDemo();
    LED_ErrorIndicationExample();
    LED_PerformanceTestExample();
    
    // Cleanup
    LED_ExamplesCleanup();
    
    while (1) {
        // Main application loop
    }
}
```

### Example Descriptions

1. **Basic Example**: Simple on/off operations and toggle functionality
2. **Blinking Example**: Various blinking patterns and custom timing
3. **Breathing Example**: Smooth breathing effects using PWM
4. **Multi-LED Example**: Coordinated effects with multiple LEDs
5. **Custom Pattern Example**: Creating SOS and heartbeat patterns
6. **Brightness Example**: PWM brightness control and fading effects
7. **Status Indicator Example**: Using LEDs for system status indication
8. **Interactive Demo**: User-controlled LED operations
9. **Error Indication Example**: Error handling and indication patterns
10. **Performance Test Example**: Timing and performance validation

## Integration Guide

### 1. Add Files to Project

Copy these files to your project:
- `led.h` - Header file
- `led.c` - Implementation file  
- `led_example.h` - Example header (optional)
- `led_example.c` - Example implementation (optional)

### 2. Include in Build System

#### For STM32CubeIDE:
1. Add files to your `Src` and `Inc` directories
2. Files will be automatically included in build

#### For Makefile:
```makefile
# Add to C sources
C_SOURCES += Peripherals/LED/led.c
C_SOURCES += Peripherals/LED/led_example.c

# Add to include paths  
C_INCLUDES += -IPeripherals/LED
```

#### For CMake:
```cmake
# Add source files
target_sources(${PROJECT_NAME} PRIVATE
    Peripherals/LED/led.c
    Peripherals/LED/led_example.c
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    Peripherals/LED
)
```

### 3. System Integration

#### GPIO Clock Enable
Ensure GPIO clocks are enabled in your system initialization:

```c
// In main.c or system initialization
void SystemClock_Config(void) {
    // ... other clock configuration ...
    
    // Enable GPIO clocks
    __HAL_RCC_GPIOG_CLK_ENABLE();
}
```

#### Timer Setup for PWM (Optional)
If using PWM brightness control:

```c
// In main.c
TIM_HandleTypeDef htim3;

void MX_TIM3_Init(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 83;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
}
```

#### Main Loop Integration
```c
int main(void) {
    // System initialization
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM3_Init(); // If using PWM
    
    // Initialize LEDs
    LED_Handle_t ledHandles[LED_COUNT];
    LED_InitAll(ledHandles);
    
    // Set initial patterns
    LED_SetPattern(&ledHandles[LED_GREEN], LED_PATTERN_SLOW_BLINK);
    
    while (1) {
        // Update LEDs (call every 10ms for smooth operation)
        LED_UpdateAll(ledHandles);
        
        // Your application code here
        
        HAL_Delay(10);
    }
}
```

## Performance Considerations

### Update Frequency
- Call `LED_Update()` every 10ms for smooth pattern operation
- Higher frequencies provide smoother effects but increase CPU usage
- Lower frequencies may cause visible stepping in breathing/fading effects

### Memory Usage
- Each LED handle uses approximately 64 bytes of RAM
- Custom patterns consume additional memory based on step count
- Static allocation is recommended for embedded systems

### Timing Accuracy
- Pattern timing depends on system tick accuracy (typically 1ms)
- PWM frequency should be >100Hz to avoid visible flicker
- Update frequency should be at least 10x the desired pattern frequency

## Troubleshooting

### Common Issues

#### LEDs Not Working
1. **Check GPIO clock**: Ensure `__HAL_RCC_GPIOG_CLK_ENABLE()` is called
2. **Verify pin configuration**: Confirm correct port/pin assignments
3. **Check power supply**: Ensure adequate power for LED operation
4. **Test with multimeter**: Verify voltage levels at GPIO pins

#### Brightness Control Not Working
1. **PWM Configuration**: Ensure timer is properly configured and started
2. **Timer Clock**: Verify timer clock is enabled
3. **Channel Assignment**: Check correct timer channel assignment
4. **GPIO Alternate Function**: Set GPIO to alternate function mode for PWM

#### Patterns Not Updating
1. **Update Calls**: Ensure `LED_Update()` is called regularly
2. **System Tick**: Verify HAL_GetTick() is working correctly
3. **Pattern State**: Check LED state using `LED_GetState()`
4. **Error Codes**: Check for errors using `LED_GetError()`

#### Performance Issues
1. **Update Frequency**: Reduce update frequency if CPU usage is high
2. **Pattern Complexity**: Simplify custom patterns with many steps
3. **Multiple LEDs**: Consider staggered updates for many LEDs

### Debug Tips

#### Enable Debug Output
```c
// Add debug prints to LED functions
#define LED_DEBUG_ENABLED 1

#if LED_DEBUG_ENABLED
#define LED_DEBUG_PRINT(fmt, ...) printf("[LED_DEBUG]: " fmt "\n", ##__VA_ARGS__)
#else
#define LED_DEBUG_PRINT(fmt, ...)
#endif
```

#### Monitor LED States
```c
// In main loop
for (int i = 0; i < LED_COUNT; i++) {
    LED_State_t state = LED_GetState(&ledHandles[i]);
    bool isOn = LED_IsOn(&ledHandles[i]);
    uint8_t brightness = LED_GetBrightness(&ledHandles[i]);
    uint32_t error = LED_GetError(&ledHandles[i]);
    
    printf("LED %d: State=%d, On=%d, Brightness=%d, Error=%lu\n", 
           i, state, isOn, brightness, (unsigned long)error);
}
```

## Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| LED_ERROR_NONE | 0x00 | No error |
| LED_ERROR_INVALID_PARAM | 0x01 | Invalid parameter |
| LED_ERROR_GPIO_INIT | 0x02 | GPIO initialization failed |
| LED_ERROR_PWM_INIT | 0x03 | PWM initialization failed |
| LED_ERROR_TIMER_START | 0x04 | Timer start failed |
| LED_ERROR_INVALID_STATE | 0x05 | Invalid state |

## Version History

- **v1.0** (2025-09-03): Initial release
  - Basic LED control functions
  - PWM brightness control
  - Pattern support
  - Multi-LED coordination
  - Comprehensive examples

## License

This software is provided as-is for educational and development purposes. Please refer to your STM32 license agreement for commercial usage terms.

## Support

For issues and questions:
1. Check the troubleshooting section above
2. Review example implementations
3. Consult STM32F429 reference manual for hardware details
4. Check STM32 HAL library documentation for API details
