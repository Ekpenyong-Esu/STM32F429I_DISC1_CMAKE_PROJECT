# LCD Character Display Driver

HD44780-compatible character LCD driver for STM32F429 with support for 4-bit and 8-bit modes.

## Features

- **Multiple Display Sizes**: 16x2, 20x4, 16x4, 20x2, 8x2, 40x2, and custom
- **Interface Modes**: 4-bit (6 pins) or 8-bit (10 pins) data interface
- **Custom Characters**: Create up to 8 custom characters
- **Display Control**: Backlight, cursor, blinking control
- **Scrolling**: Left/right scrolling with auto-scroll mode
- **Printf Support**: Formatted printing like standard printf
- **Handle-Based API**: Consistent with other peripheral drivers

## Pin Connections

### 4-bit Mode (Recommended)
| LCD Pin | Function | MCU Connection |
|---------|----------|----------------|
| VSS     | Ground   | GND            |
| VDD     | Power    | 5V             |
| V0      | Contrast | Potentiometer  |
| RS      | Register Select | GPIO (Output) |
| RW      | Read/Write | GND (or GPIO) |
| E       | Enable   | GPIO (Output)  |
| D4      | Data 4   | GPIO (Output)  |
| D5      | Data 5   | GPIO (Output)  |
| D6      | Data 6   | GPIO (Output)  |
| D7      | Data 7   | GPIO (Output)  |
| A       | Backlight+ | 5V (or GPIO) |
| K       | Backlight- | GND          |

### 8-bit Mode
Same as 4-bit plus D0-D3 connected to GPIOs.

## Quick Start

### Basic 16x2 LCD (4-bit Mode)

```c
#include "lcd.h"

LCD_HandleTypeDef hLCD;

void LCD_Setup(void)
{
    LCD_PinsTypeDef pins = {
        .rs = {GPIOA, GPIO_PIN_0},
        .rw = {NULL, 0},            // RW tied to GND
        .en = {GPIOA, GPIO_PIN_1},
        .d4 = {GPIOA, GPIO_PIN_2},
        .d5 = {GPIOA, GPIO_PIN_3},
        .d6 = {GPIOA, GPIO_PIN_4},
        .d7 = {GPIOA, GPIO_PIN_5},
        .backlight = {GPIOA, GPIO_PIN_6}
    };
    
    LCD_InitDefault(&hLCD, &pins);
    
    LCD_PrintString(&hLCD, "Hello World!");
}
```

### 20x4 LCD with Full Config

```c
#include "lcd.h"

LCD_HandleTypeDef hLCD;

void LCD_Setup_20x4(void)
{
    LCD_ConfigTypeDef config = {
        .pins = {
            .rs = {GPIOB, GPIO_PIN_0},
            .rw = {NULL, 0},
            .en = {GPIOB, GPIO_PIN_1},
            .d4 = {GPIOB, GPIO_PIN_4},
            .d5 = {GPIOB, GPIO_PIN_5},
            .d6 = {GPIOB, GPIO_PIN_6},
            .d7 = {GPIOB, GPIO_PIN_7},
            .backlight = {GPIOB, GPIO_PIN_8}
        },
        .mode = LCD_MODE_4BIT,
        .size = LCD_SIZE_20x4,
        .useRW = false,
        .useBacklight = true
    };
    
    LCD_Init(&hLCD, &config);
    
    LCD_PrintStringAt(&hLCD, 0, 0, "Line 1");
    LCD_PrintStringAt(&hLCD, 0, 1, "Line 2");
    LCD_PrintStringAt(&hLCD, 0, 2, "Line 3");
    LCD_PrintStringAt(&hLCD, 0, 3, "Line 4");
}
```

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `LCD_Init()` | Initialize LCD with full configuration |
| `LCD_InitDefault()` | Initialize 16x2 LCD in 4-bit mode |
| `LCD_DeInit()` | Deinitialize LCD |

### Basic Operations

| Function | Description |
|----------|-------------|
| `LCD_Clear()` | Clear display |
| `LCD_Home()` | Return cursor to home (0,0) |
| `LCD_DisplayOn()` | Turn display on |
| `LCD_DisplayOff()` | Turn display off |
| `LCD_BacklightOn()` | Turn backlight on |
| `LCD_BacklightOff()` | Turn backlight off |

### Cursor Control

| Function | Description |
|----------|-------------|
| `LCD_SetCursor()` | Set cursor position (col, row) |
| `LCD_CursorOn()` | Show cursor |
| `LCD_CursorOff()` | Hide cursor |
| `LCD_BlinkOn()` | Enable cursor blinking |
| `LCD_BlinkOff()` | Disable cursor blinking |
| `LCD_CursorLeft()` | Move cursor left |
| `LCD_CursorRight()` | Move cursor right |

### Printing

| Function | Description |
|----------|-------------|
| `LCD_PrintChar()` | Print single character |
| `LCD_PrintString()` | Print null-terminated string |
| `LCD_PrintStringAt()` | Print string at position |
| `LCD_PrintInt()` | Print integer value |
| `LCD_PrintFloat()` | Print float with decimals |
| `LCD_PrintHex()` | Print hexadecimal value |
| `LCD_Printf()` | Formatted print (printf-style) |
| `LCD_ClearLine()` | Clear specific line |

### Scrolling

| Function | Description |
|----------|-------------|
| `LCD_ScrollLeft()` | Scroll display left |
| `LCD_ScrollRight()` | Scroll display right |
| `LCD_AutoScrollOn()` | Enable auto-scroll |
| `LCD_AutoScrollOff()` | Disable auto-scroll |

### Custom Characters

| Function | Description |
|----------|-------------|
| `LCD_CreateChar()` | Create custom character (0-7) |
| `LCD_PrintCustomChar()` | Print custom character |

## Custom Characters

Create up to 8 custom characters (5x8 pixels):

```c
// Heart symbol
const uint8_t heartChar[] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};

// Degree symbol
const uint8_t degreeChar[] = {
    0b01100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

void CreateSymbols(void)
{
    LCD_CreateChar(&hLCD, 0, heartChar);
    LCD_CreateChar(&hLCD, 1, degreeChar);
    
    // Use them
    LCD_PrintString(&hLCD, "I ");
    LCD_PrintCustomChar(&hLCD, 0);  // Heart
    LCD_PrintString(&hLCD, " STM32");
    
    LCD_SetCursor(&hLCD, 0, 1);
    LCD_PrintString(&hLCD, "Temp: 25");
    LCD_PrintCustomChar(&hLCD, 1);  // Degree
    LCD_PrintString(&hLCD, "C");
}
```

## Display Sizes

| Size Enum | Columns | Rows |
|-----------|---------|------|
| `LCD_SIZE_16x2` | 16 | 2 |
| `LCD_SIZE_20x4` | 20 | 4 |
| `LCD_SIZE_16x4` | 16 | 4 |
| `LCD_SIZE_20x2` | 20 | 2 |
| `LCD_SIZE_8x2` | 8 | 2 |
| `LCD_SIZE_40x2` | 40 | 2 |
| `LCD_SIZE_CUSTOM` | User-defined | User-defined |

## Printf Examples

```c
// Integer
LCD_Printf(&hLCD, "Count: %d", 42);

// Float (use %.Nf format)
LCD_Printf(&hLCD, "Temp: %.2f C", 25.5);

// Multiple values
LCD_Printf(&hLCD, "%d/%d/%d", day, month, year);

// Hex
LCD_Printf(&hLCD, "Addr: 0x%04X", address);
```

## Progress Bar Example

```c
void ShowProgress(uint8_t percent)
{
    LCD_SetCursor(&hLCD, 0, 1);
    LCD_PrintString(&hLCD, "[");
    
    int bars = percent / 10;
    for (int i = 0; i < 10; i++) {
        LCD_PrintString(&hLCD, i < bars ? "#" : "-");
    }
    
    LCD_Printf(&hLCD, "] %3d%%", percent);
}
```

## Sensor Display Template

```c
void DisplaySensorData(float temp, float humidity, float pressure)
{
    // Line 1: Temperature
    LCD_SetCursor(&hLCD, 0, 0);
    LCD_Printf(&hLCD, "T:%.1fC H:%.0f%%", temp, humidity);
    
    // Line 2: Pressure
    LCD_SetCursor(&hLCD, 0, 1);
    LCD_Printf(&hLCD, "P:%.0f hPa", pressure);
}
```

## Timing Notes

- Power-on delay: 50ms required before initialization
- Clear/Home commands: 2ms execution time
- Other commands: ~50µs execution time
- Enable pulse width: 1µs minimum

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Blank display | Check contrast (V0), verify power |
| Black boxes | Initialization failed, check timing |
| Garbled text | Verify 4-bit/8-bit mode matches wiring |
| No backlight | Check backlight pin or A/K connections |
| Missing characters | Wrong cursor position, check bounds |

## Dependencies

- STM32 HAL Driver
- GPIO peripheral

## Memory Usage

- Handle: ~40 bytes per LCD
- Stack: ~80 bytes for Printf buffer
- No dynamic allocation

## Thread Safety

This driver is **not thread-safe**. Use mutexes in FreeRTOS:

```c
osMutexId_t lcdMutex;

void LCD_ThreadSafePrint(const char* str)
{
    osMutexAcquire(lcdMutex, osWaitForever);
    LCD_PrintString(&hLCD, str);
    osMutexRelease(lcdMutex);
}
```

## License

Part of the STM32F429 peripheral driver library.
