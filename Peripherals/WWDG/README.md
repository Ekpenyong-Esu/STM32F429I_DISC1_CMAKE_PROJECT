# WWDG - Window Watchdog Driver

## Overview

The Window Watchdog (WWDG) driver provides a hardware watchdog timer with a **window** constraint. Unlike IWDG, WWDG requires refresh within a specific time window - not too early and not too late.

## Features

- Window-based refresh timing
- Early Wakeup Interrupt (EWI) for last-chance refresh
- Configurable prescaler and window
- Reset source detection
- Precise timing calculations

## Key Differences from IWDG

| Feature | IWDG | WWDG |
|---------|------|------|
| Clock Source | LSI (~32 kHz) | APB1 (PCLK1) |
| Refresh Timing | Anytime before timeout | Only within window |
| Max Timeout | ~32 seconds | ~58 milliseconds |
| Early Warning | No | Yes (EWI) |
| Stop in Debug | Configurable | Configurable |

## Window Concept

```
Counter Value:
0x7F ──────────────┐
                   │  ← Cannot refresh here (too early)
Window (e.g. 0x50) ┤
                   │  ← VALID REFRESH WINDOW
0x40 ──────────────┤  ← EWI triggers here
                   │
0x3F ──────────────┘  ← RESET occurs here
```

## Quick Start

### Basic Usage

```c
#include "wwdg.h"

int main(void)
{
    /* Check reset source */
    if (WWDG_WasResetSource())
    {
        WWDG_ClearResetFlag();
    }

    /* Initialize with default settings */
    WWDG_Init();

    while (1)
    {
        /* Wait for window to open */
        while (!WWDG_IsInWindow()) { }

        /* Refresh within window */
        WWDG_Refresh();

        /* Do work... */
    }
}
```

### Custom Configuration

```c
WWDG_ConfigTypeDef config = {
    .Prescaler = WWDG_PRESCALER_8,
    .Window = 0x50,
    .Counter = 0x7F,
    .EWIMode = WWDG_EWI_DISABLE
};

WWDG_Init_Custom(&config);
```

### Early Wakeup Interrupt

```c
void MyEWICallback(void)
{
    /* Counter reached 0x40 - last chance! */
    /* WWDG is auto-refreshed in HAL callback */
}

void setup(void)
{
    WWDG_Init();
    WWDG_RegisterEWICallback(MyEWICallback);
    WWDG_EnableEWI();
}
```

## API Reference

### Initialization Functions

| Function | Description |
|----------|-------------|
| `WWDG_Init()` | Initialize with default settings |
| `WWDG_Init_Custom(config)` | Initialize with custom configuration |
| `WWDG_DeInit()` | Deinitialize WWDG |

### Operation Functions

| Function | Description |
|----------|-------------|
| `WWDG_Refresh()` | Refresh counter (must be in window) |
| `WWDG_RefreshWithCounter(value)` | Refresh with specific counter value |
| `WWDG_Start()` | Start watchdog (auto-started by Init) |

### Interrupt Functions

| Function | Description |
|----------|-------------|
| `WWDG_RegisterEWICallback(cb)` | Register EWI callback |
| `WWDG_EnableEWI()` | Enable Early Wakeup Interrupt |

### Utility Functions

| Function | Description |
|----------|-------------|
| `WWDG_WasResetSource()` | Check if WWDG caused reset |
| `WWDG_ClearResetFlag()` | Clear reset flag |
| `WWDG_GetCounter()` | Get current counter value |
| `WWDG_IsInWindow()` | Check if refresh is valid |
| `WWDG_CalculateTimeout()` | Calculate timing values |

## Timeout Calculation

```
T_wwdg = (4096 × 2^WDGTB × (T[5:0] + 1)) / F_pclk1

Where:
- WDGTB = Prescaler bits (0-3)
- T[5:0] = Counter bits 0-5 (counter - 0x40)
- F_pclk1 = APB1 clock (45 MHz for STM32F429 at 180MHz)
```

### Example Configurations

| Prescaler | Counter | Window | Reset Timeout |
|-----------|---------|--------|---------------|
| 1 | 0x7F | 0x50 | ~5.8 ms |
| 8 | 0x7F | 0x50 | ~46.6 ms |
| 8 | 0x7F | 0x7E | ~58 ms max |

## Use Cases

1. **Critical timing requirements** - When refresh must occur at specific intervals
2. **Software health monitoring** - Ensure code reaches refresh point in expected time
3. **Debugging** - EWI provides breakpoint opportunity before reset
4. **Dual watchdog** - Use with IWDG for layered protection

## Best Practices

1. **Calculate window carefully** - Too narrow = difficult to hit, too wide = less protection
2. **Use EWI for debugging** - Helps identify timing issues
3. **Monitor counter value** - `WWDG_GetCounter()` helps tune timing
4. **Check `WWDG_IsInWindow()`** - Before refreshing to avoid premature refresh reset

## IRQ Handler

Add to your `stm32f4xx_it.c`:

```c
void WWDG_IRQHandler(void)
{
    HAL_WWDG_IRQHandler(&hwwdg);
}
```

## Debugging Tips

- Use EWI to catch near-timeout events
- Monitor counter with `WWDG_GetCounter()`
- Check reset source at startup
- Start with wider window during development
