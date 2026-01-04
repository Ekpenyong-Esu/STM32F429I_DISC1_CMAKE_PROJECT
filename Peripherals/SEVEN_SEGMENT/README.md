# Seven-Segment Display Driver for STM32F429

A flexible seven-segment display driver supporting both direct GPIO control and HT1621 LCD driver IC, with options for common cathode and common anode configurations.

## Features

- **Dual Driver Support**: GPIO direct control or HT1621 LCD driver IC
- **Polarity Options**: Common cathode and common anode configurations
- **Multi-Digit Support**: Up to 8 digits with multiplexing
- **Display Functions**: Integer, float, hex, string, and custom patterns
- **Character Set**: Digits 0-9, hex A-F, and common letters
- **Decimal Point**: Support for decimal point on each digit
- **Leading Zeros**: Optional leading zero display

## Hardware Configurations

### GPIO Mode (Direct Control)

```
       --A--
      |     |
      F     B
      |     |
       --G--
      |     |
      E     C
      |     |
       --D--  .DP
```

#### Common Cathode Configuration
- Segments are **active HIGH** (GPIO HIGH = segment ON)
- Common pin connects to GND through transistor (NPN)
- Use current-limiting resistors on segment pins (220-470Ω)

```
    MCU GPIO ──┬── R(330Ω) ──── Segment A
               ├── R(330Ω) ──── Segment B
               └── ...
    
    MCU GPIO ──── NPN Base
                   │
              Digit Common ──── GND
```

#### Common Anode Configuration
- Segments are **active LOW** (GPIO LOW = segment ON)
- Common pin connects to VCC through transistor (PNP)
- Use current-limiting resistors on segment pins (220-470Ω)

```
    MCU GPIO ──┬── R(330Ω) ──── Segment A
               ├── R(330Ω) ──── Segment B
               └── ...
    
    MCU GPIO ──── PNP Base
                   │
              Digit Common ──── VCC
```

### HT1621 LCD Driver Mode

The HT1621 is a RAM-mapped LCD driver that can drive up to 32×4 LCD segments.

```
    MCU                 HT1621
    ────                ──────
    GPIO (CS)  ──────── CS
    GPIO (WR)  ──────── WR
    GPIO (DATA)──────── DATA
    3.3V       ──────── VDD
    GND        ──────── VSS
```

## Quick Start

### 1. GPIO Mode - Common Cathode (4 Digits)

```c
#include "seven_segment.h"

SegDisplayHandle_t display;
SegGpioPin_t digitPins[4];

void init_display(void)
{
    SegDisplayConfig_t config = {0};
    
    config.driverType = SEG_DRIVER_GPIO;
    
    /* Segment pins (A-G + DP) */
    config.config.gpio.segments[SEG_A].port = GPIOA;
    config.config.gpio.segments[SEG_A].pin  = GPIO_PIN_0;
    config.config.gpio.segments[SEG_B].port = GPIOA;
    config.config.gpio.segments[SEG_B].pin  = GPIO_PIN_1;
    /* ... configure remaining segments ... */
    
    /* Digit select pins */
    digitPins[0] = (SegGpioPin_t){GPIOB, GPIO_PIN_0};
    digitPins[1] = (SegGpioPin_t){GPIOB, GPIO_PIN_1};
    digitPins[2] = (SegGpioPin_t){GPIOB, GPIO_PIN_2};
    digitPins[3] = (SegGpioPin_t){GPIOB, GPIO_PIN_3};
    
    config.config.gpio.digits = digitPins;
    config.config.gpio.digitCount = 4;
    config.config.gpio.polarity = SEG_COMMON_CATHODE;
    config.config.gpio.digitActiveHigh = true;
    
    Seg_Init(&display, &config);
}

/* Call in main loop or timer interrupt */
void update_display(void)
{
    Seg_Update(&display);  /* Must be called ~400Hz for 4 digits */
}
```

### 2. GPIO Mode - Common Anode

```c
/* Same as above, but change polarity settings: */
config.config.gpio.polarity = SEG_COMMON_ANODE;
config.config.gpio.digitActiveHigh = false;  /* PNP transistors */
```

### 3. HT1621 LCD Driver Mode

```c
#include "seven_segment.h"

SegDisplayHandle_t display;

void init_ht1621_display(void)
{
    SegDisplayConfig_t config = {0};
    
    config.driverType = SEG_DRIVER_HT1621;
    
    /* HT1621 control pins */
    config.config.ht1621.pins.csPort   = GPIOC;
    config.config.ht1621.pins.csPin    = GPIO_PIN_0;
    config.config.ht1621.pins.wrPort   = GPIOC;
    config.config.ht1621.pins.wrPin    = GPIO_PIN_1;
    config.config.ht1621.pins.dataPort = GPIOC;
    config.config.ht1621.pins.dataPin  = GPIO_PIN_2;
    
    config.config.ht1621.digitCount = 6;
    config.config.ht1621.bias = 3;     /* 1/3 bias */
    config.config.ht1621.commons = 4;  /* 4 commons */
    
    Seg_Init(&display, &config);
}
```

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `Seg_Init()` | Initialize display with configuration |
| `Seg_DeInit()` | Deinitialize and free resources |

### Control

| Function | Description |
|----------|-------------|
| `Seg_Enable()` | Enable display output |
| `Seg_Disable()` | Disable display output |
| `Seg_Clear()` | Clear all digits |
| `Seg_SetBrightness()` | Set brightness (HT1621 only) |

### Display Functions

| Function | Description |
|----------|-------------|
| `Seg_SetDigit()` | Display single digit (0-15) |
| `Seg_SetPattern()` | Display raw segment pattern |
| `Seg_DisplayInt()` | Display integer value |
| `Seg_DisplayFloat()` | Display floating-point value |
| `Seg_DisplayHex()` | Display hexadecimal value |
| `Seg_SetChar()` | Display single character |
| `Seg_DisplayString()` | Display string |

### Update Functions

| Function | Description |
|----------|-------------|
| `Seg_Update()` | Update display (call in timer for GPIO mode) |
| `Seg_Refresh()` | Refresh all digits (HT1621 mode) |

### Utility

| Function | Description |
|----------|-------------|
| `Seg_GetPattern()` | Get pattern for digit value |
| `Seg_CharToPattern()` | Get pattern for character |
| `Seg_Test()` | Test all segments (display 8.) |

## Multiplexing (GPIO Mode)

For GPIO mode with multiple digits, you must call `Seg_Update()` periodically:

```c
/* Option 1: Timer interrupt (recommended) */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
    Seg_Update(&display);
}

/* Option 2: Main loop with delay */
while (1) {
    Seg_Update(&display);
    HAL_Delay(2);  /* ~500Hz update rate */
}
```

**Recommended update rates:**
- 1 digit: No multiplexing needed
- 2 digits: ≥200Hz (100Hz per digit)
- 4 digits: ≥400Hz (100Hz per digit)
- 8 digits: ≥800Hz (100Hz per digit)

## Segment Patterns

Standard patterns are predefined:

| Character | Pattern | Segments |
|-----------|---------|----------|
| 0 | 0x3F | A,B,C,D,E,F |
| 1 | 0x06 | B,C |
| 2 | 0x5B | A,B,D,E,G |
| 3 | 0x4F | A,B,C,D,G |
| 4 | 0x66 | B,C,F,G |
| 5 | 0x6D | A,C,D,F,G |
| 6 | 0x7D | A,C,D,E,F,G |
| 7 | 0x07 | A,B,C |
| 8 | 0x7F | A,B,C,D,E,F,G |
| 9 | 0x6F | A,B,C,D,F,G |
| A | 0x77 | A,B,C,E,F,G |
| b | 0x7C | C,D,E,F,G |
| C | 0x39 | A,D,E,F |
| d | 0x5E | B,C,D,E,G |
| E | 0x79 | A,D,E,F,G |
| F | 0x71 | A,E,F,G |
| - | 0x40 | G |

## Supported Characters

The driver supports these characters:
- Digits: `0-9`
- Hex: `A-F` (uppercase and lowercase)
- Letters: `H, L, P, U, O, S, I, G, n, r, t, y`
- Symbols: `-`, `_`, ` ` (space), `.` (decimal point)

## Configuration Options

### SegDisplayConfig_t

| Field | Type | Description |
|-------|------|-------------|
| `driverType` | `SegDriverType_t` | `SEG_DRIVER_GPIO` or `SEG_DRIVER_HT1621` |
| `multiplexDelayUs` | `uint16_t` | Multiplex delay (GPIO mode) |
| `leadingZeros` | `bool` | Display leading zeros |

### SegGpioConfig_t (GPIO Mode)

| Field | Type | Description |
|-------|------|-------------|
| `segments` | `SegGpioPin_t[8]` | Segment GPIO pins |
| `digits` | `SegGpioPin_t*` | Digit select pins |
| `digitCount` | `uint8_t` | Number of digits |
| `polarity` | `SegPolarity_t` | Common cathode/anode |
| `digitActiveHigh` | `bool` | Digit select active level |

### SegHT1621Config_t (HT1621 Mode)

| Field | Type | Description |
|-------|------|-------------|
| `pins` | `SegHT1621Pins_t` | CS, WR, DATA pins |
| `digitCount` | `uint8_t` | Number of digits |
| `bias` | `uint8_t` | LCD bias (2 or 3) |
| `commons` | `uint8_t` | Number of commons (2-4) |

## Error Handling

All functions return `SegStatus_t`:

| Status | Description |
|--------|-------------|
| `SEG_OK` | Success |
| `SEG_ERROR` | General error |
| `SEG_INVALID_PARAM` | Invalid parameter |
| `SEG_NOT_INITIALIZED` | Driver not initialized |
| `SEG_BUSY` | Driver busy |

## Example: Clock Display

```c
void display_clock(uint8_t hours, uint8_t minutes)
{
    Seg_SetDigit(&display, 0, hours / 10, false);
    Seg_SetDigit(&display, 1, hours % 10, true);   /* DP as colon */
    Seg_SetDigit(&display, 2, minutes / 10, false);
    Seg_SetDigit(&display, 3, minutes % 10, false);
}
```

## Example: Temperature Display

```c
void display_temperature(float temp)
{
    Seg_DisplayFloat(&display, temp, 1);  /* e.g., "25.6" */
}
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Display flickering | Increase update rate or reduce digit count |
| Segments always on | Check polarity configuration |
| No display | Verify GPIO clock enable and pin assignments |
| Ghosting | Increase multiplex delay or add blanking |
| HT1621 not responding | Check wiring and timing delays |

## License

MIT License - see LICENSE file for details.
