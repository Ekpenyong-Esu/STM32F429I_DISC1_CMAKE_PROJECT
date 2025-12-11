# IWDG - Independent Watchdog Driver

## Overview

The Independent Watchdog (IWDG) driver provides a hardware watchdog timer to ensure system reliability. The IWDG automatically resets the microcontroller if the software fails to refresh it within the timeout period.

## Features

- Easy initialization with millisecond timeout specification
- Automatic prescaler/reload calculation
- Reset source detection
- FreeRTOS integration support
- HAL-based implementation

## Key Characteristics

| Parameter | Value |
|-----------|-------|
| Clock Source | LSI (~32 kHz) |
| Timeout Range | ~125 µs to ~32 seconds |
| Reload Range | 0 - 4095 |
| Prescaler Options | 4, 8, 16, 32, 64, 128, 256 |

**Important**: Once started, the IWDG **cannot be stopped** except by system reset!

## Quick Start

### Basic Usage (1 second timeout)

```c
#include "iwdg.h"

int main(void)
{
    /* Check if previous reset was caused by watchdog */
    if (IWDG_WasResetSource())
    {
        /* Handle watchdog reset */
        IWDG_ClearResetFlag();
    }

    /* Initialize with default 1 second timeout */
    IWDG_Init();

    while (1)
    {
        /* Main application code */
        DoWork();

        /* Must refresh before timeout! */
        IWDG_Refresh();
    }
}
```

### Custom Timeout

```c
/* Initialize with 2 second timeout */
IWDG_Init_TimeoutMs(2000);

/* Or use predefined constants */
IWDG_Init_TimeoutMs(IWDG_TIMEOUT_4S);
```

### Advanced Configuration

```c
IWDG_ConfigTypeDef config = {
    .Prescaler = IWDG_PRESCALER_64,
    .Reload = 2000
};

/* Calculate actual timeout */
uint32_t timeout = IWDG_CalculateTimeout(config.Prescaler, config.Reload);

IWDG_Init_Custom(&config);
```

## API Reference

### Initialization Functions

| Function | Description |
|----------|-------------|
| `IWDG_Init()` | Initialize with default 1 second timeout |
| `IWDG_Init_TimeoutMs(ms)` | Initialize with specified timeout |
| `IWDG_Init_Custom(config)` | Initialize with manual configuration |

### Operation Functions

| Function | Description |
|----------|-------------|
| `IWDG_Refresh()` | Reset watchdog counter (must call periodically) |
| `IWDG_Start()` | Start watchdog (auto-started by Init) |

### Utility Functions

| Function | Description |
|----------|-------------|
| `IWDG_WasResetSource()` | Check if IWDG caused last reset |
| `IWDG_ClearResetFlag()` | Clear reset flag |
| `IWDG_CalculateTimeout()` | Calculate timeout from prescaler/reload |
| `IWDG_GetStatusString()` | Get human-readable status string |

## Timeout Calculation

Timeout formula:
```
timeout_ms = (reload × prescaler) / LSI_frequency × 1000
           = (reload × prescaler) / 32
```

### Common Configurations

| Prescaler | Reload | Timeout |
|-----------|--------|---------|
| 4 | 4095 | ~512 ms |
| 32 | 1000 | ~1 s |
| 32 | 4095 | ~4 s |
| 256 | 4095 | ~32 s |

## Best Practices

1. **Always check reset source** at startup to detect watchdog resets
2. **Refresh in main loop** or dedicated task, not in ISR
3. **Set timeout appropriately** - long enough for normal operation, short enough to catch hangs
4. **Test thoroughly** - verify watchdog triggers correctly when expected
5. **Consider FreeRTOS integration** - use dedicated low-priority task for refreshing

## FreeRTOS Integration

```c
void WatchdogTask(void *pvParameters)
{
    IWDG_Init_TimeoutMs(IWDG_TIMEOUT_2S);

    while (1)
    {
        IWDG_Refresh();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

## Debugging Tips

- Use `IWDG_WasResetSource()` to detect unexpected resets
- Start with longer timeouts during development
- Disable IWDG during debug sessions if needed (via debug options)

## Hardware Notes

- LSI oscillator accuracy is ±5%, affecting timeout precision
- IWDG runs independently from main system clock
- IWDG continues running in low-power modes (except Standby)
