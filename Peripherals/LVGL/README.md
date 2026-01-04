# LVGL Integration for STM32F429I-DISC1

LVGL (Light and Versatile Graphics Library) integration for the ILI9341 LCD
on STM32F429I-Discovery board using LTDC and SDRAM framebuffer.

## Features

- **LVGL v9** support with RGB565 color format
- **Double-buffered** SDRAM framebuffer (tear-free)
- **4-Screen GUI** - Home, Sensors, Settings, System Info
- **Touch support** ready (placeholder implementation)
- **Partial rendering** for memory efficiency

## Hardware Setup

```
STM32F429I-DISC1 LCD Stack:
┌─────────────────────────────────┐
│  LVGL (GUI rendering)           │
├─────────────────────────────────┤
│  lv_port_disp.c (flush to SDRAM)│
├─────────────────────────────────┤
│  LTDC (displays SDRAM content)  │
├─────────────────────────────────┤
│  ILI9341 (LCD controller)       │
├─────────────────────────────────┤
│  SDRAM (framebuffer storage)    │
└─────────────────────────────────┘
```

## Initialization Sequence

```c
#include "fmc.h"
#include "ili9341.h"
#include "ltdc.h"
#include "lvgl_app.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // 1. Initialize SDRAM (stores framebuffer)
    FMC_Driver_SDRAM_Init(&fmcHandle, &sdramConfig);
    
    // 2. Initialize ILI9341 LCD controller (SPI config, RGB mode)
    ILI9341_Init();
    
    // 3. Initialize LTDC (RGB parallel interface)
    LTDC_HW_Init();
    
    // 4. Initialize LVGL + GUI
    LVGL_App_Init();
    
    while(1) {
        LVGL_App_Tick();  // Process LVGL
        HAL_Delay(5);
    }
}
```

## API Usage

### Update Display Values

```c
#include "lvgl_app.h"

// Update temperature gauge (0-100°C)
LVGL_App_UpdateTemperature(25);

// Update humidity bar (0-100%)
LVGL_App_UpdateHumidity(60);

// Add data point to sensor chart
LVGL_App_AddChartData(sensor_value);

// Update status message
LVGL_App_UpdateStatus(LV_SYMBOL_OK " System OK");
```

## Files

| File | Description |
|------|-------------|
| `lv_port_disp.c/h` | Display driver - flushes to SDRAM |
| `lv_port_indev.c/h` | Touch input driver (placeholder) |
| `lvgl_app.c/h` | GUI application with 4 screens |
| `lv_conf.h` | LVGL configuration |

## Memory Layout (SDRAM)

```
0xD0000000  ┌─────────────────┐
            │  Framebuffer 0  │  153,600 bytes (240×320×2)
0xD0025800  ├─────────────────┤
            │  Framebuffer 1  │  153,600 bytes (240×320×2)
0xD004B000  ├─────────────────┤
            │  Available      │  ~7.7 MB remaining
            └─────────────────┘
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Black screen | Check SDRAM init, verify ILI9341 init before LTDC |
| Wrong colors | Check RGB565 format, may need BGR swap in lv_conf.h |
| Flickering | Double buffering issue, check swap_buffers() |
| Slow rendering | Increase DRAW_BUF_LINES in lv_port_disp.c |

## Dependencies

- SDRAM (Peripherals/SDRAM or FMC driver)
- ILI9341 (Peripherals/ILI9341)
- LTDC (Peripherals/LTDC)
- LVGL library (downloaded via CMake)

