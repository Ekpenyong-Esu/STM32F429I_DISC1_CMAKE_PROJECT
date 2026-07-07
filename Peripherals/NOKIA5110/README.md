# Nokia 5110 LCD Driver

This folder contains the driver for the Nokia 5110 LCD display (84x48 pixels) for the STM32F429 Discovery board.

## Features

- 84x48 pixel monochrome LCD display
- SPI interface communication
- Built-in 5x7 ASCII font
- Drawing primitives (pixels, lines, rectangles, circles)
- Text rendering
- Configurable contrast and display modes
- Buffer-based drawing for smooth updates

## Hardware Connections

The Nokia 5110 LCD requires the following connections to the STM32F429 Discovery board:

| Nokia 5110 Pin | STM32F429 Pin | Description |
|----------------|----------------|-------------|
| RST           | PB1           | Reset pin |
| CE            | PB0           | Chip Enable (SPI CS) |
| DC            | PB2           | Data/Command select |
| DIN           | PF9           | SPI MOSI (shared with ILI9341) |
| CLK           | PF7           | SPI SCK (shared with ILI9341) |
| VCC           | 3.3V          | Power supply |
| GND           | GND           | Ground |
| BL            | 3.3V/NC       | Backlight (optional) |

## Usage Example

```c
#include "nokia5110.h"

NOKIA5110_Handle_t hnok;

// Initialize the LCD
if (NOKIA5110_Init(&hnok) == NOKIA5110_OK) {
    // Draw some text
    NOKIA5110_DrawText(&hnok, 0, 0, "Hello World!", 1);

    // Draw a rectangle
    NOKIA5110_DrawRect(&hnok, 10, 10, 30, 20, 1);

    // Update the display
    NOKIA5110_Update(&hnok);
}
```

## API Reference

### Initialization and Configuration

- `NOKIA5110_Init()` - Initialize the LCD
- `NOKIA5110_DeInit()` - Deinitialize the LCD
- `NOKIA5110_Config()` - Configure LCD parameters
- `NOKIA5110_SetContrast()` - Set display contrast
- `NOKIA5110_SetMode()` - Set display mode

### Display Control

- `NOKIA5110_Clear()` - Clear the display
- `NOKIA5110_Update()` - Update display with buffer contents

### Drawing Functions

- `NOKIA5110_DrawPixel()` - Draw a single pixel
- `NOKIA5110_DrawLine()` - Draw a line
- `NOKIA5110_DrawRect()` - Draw a rectangle outline
- `NOKIA5110_FillRect()` - Draw a filled rectangle
- `NOKIA5110_DrawCircle()` - Draw a circle
- `NOKIA5110_DrawText()` - Draw text

### Utility Functions

- `NOKIA5110_GetWidth()` - Get display width
- `NOKIA5110_GetHeight()` - Get display height
- `NOKIA5110_GetDefaultConfig()` - Get default configuration

## Dependencies

- SPI peripheral driver (`Peripherals/SPI/`)
- GPIO peripheral driver (`Peripherals/GPIO/`)

## Notes

- The driver uses SPI5 which is shared with the ILI9341 display
- Make sure to initialize SPI before using the Nokia 5110
- The display buffer is 84x6 bytes (504 bytes total)
- Text rendering uses a built-in 5x7 ASCII font
- All drawing operations modify the buffer; call `NOKIA5110_Update()` to refresh the display
