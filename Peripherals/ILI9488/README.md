# ILI9488 TFT LCD Display Driver

This driver provides support for 4-inch TFT LCD displays with ILI9488 controller.

## Features

- 320x480 resolution (configurable orientation)
- 16-bit RGB565 color support
- SPI interface
- Hardware reset and chip select control
- Basic graphics functions (pixels, lines, rectangles, circles)
- Text rendering with 6x8 font
- Multiple orientation support

## Hardware Requirements

- ILI9488 TFT LCD display
- SPI peripheral
- 3 GPIO pins (CS, DC, RST)

## Usage Example

```c
#include "ili9488.h"

// Initialize display
ILI9488_Handle_t hili;
ILI9488_Init(&hili, GPIOB, GPIO_PIN_12, GPIOB, GPIO_PIN_13, GPIOB, GPIO_PIN_14);

// Clear screen
ILI9488_Clear(&hili, ILI9488_COLOR_BLACK);

// Draw some graphics
ILI9488_DrawPixel(&hili, 100, 100, ILI9488_COLOR_RED);
ILI9488_DrawRectangle(&hili, 50, 50, 100, 100, ILI9488_COLOR_BLUE);

// Write text
ILI9488_SetCursor(&hili, 10, 10);
ILI9488_WriteString(&hili, "Hello World!", ILI9488_COLOR_WHITE, ILI9488_COLOR_BLACK);
```

## Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| CS  | Chip Select | Active low |
| DC  | Data/Command | High for data, low for command |
| RST | Reset | Active low reset |

## CMake Integration

Enable the driver by setting `USE_ILI9488=ON` in CMakeLists.txt or command line:

```bash
cmake -DUSE_ILI9488=ON ..
```

## Dependencies

- STM32 HAL SPI driver
- GPIO driver
- Standard C libraries
