# PWR - Power Management Driver

## Overview

The PWR driver provides power management functionality for the STM32F429, including sleep modes, voltage detection, and backup domain control.

## Features

- Sleep, Stop, and Standby low-power modes
- Programmable Voltage Detector (PVD)
- Backup register access
- Voltage scaling control
- Wakeup pin management

## Power Modes

| Mode | Current | Wake Time | RAM | Wake Sources |
|------|---------|-----------|-----|--------------|
| Run | ~100 mA | N/A | Yes | N/A |
| Sleep | ~10 mA | <1 µs | Yes | Any interrupt |
| Stop | ~100 µA | ~4 µs | Yes | EXTI, RTC |
| Standby | ~2.5 µA | ~50 µs | No* | WKUP pin, RTC, Reset |

\* Backup registers retained if VBAT is connected

## Quick Start

### Initialize

```c
#include "pwr.h"

/* Initialize with defaults */
PWR_InitDefault();

/* Or with custom config */
PWR_ConfigTypeDef config = {
    .enablePVD = true,
    .pvdLevel = PWR_PVD_LEVEL_2V9,
    .enableBackupAccess = true,
    .enableWakeupPin = false
};
PWR_Init(&config);
```

### Enter Sleep Mode

```c
/* Simple sleep until any interrupt */
PWR_EnterSleepMode(PWR_SLEEP_MODE_WFI);

/* Sleep for specific duration */
PWR_SleepFor(1000);  /* 1 second */
```

### Enter Stop Mode

```c
/* Configure wakeup source (e.g., button on EXTI) first */

/* Enter Stop with low-power regulator */
PWR_EnterStopMode(PWR_REGULATOR_LOW_POWER, PWR_STOP_ENTRY_WFI);

/* After wakeup - restore clocks */
PWR_ConfigureAfterStop();
```

### Enter Standby Mode

```c
/* Save important data first */
PWR_WriteBackupRegister(0, myData);

/* Enable wakeup pin */
PWR_EnableWakeupPin(true);

/* Enter Standby - system resets on wake */
PWR_EnterStandbyMode();
```

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `PWR_Init(config)` | Initialize with configuration |
| `PWR_InitDefault()` | Initialize with defaults |
| `PWR_GetDefaultConfig(config)` | Get default config values |

### Sleep Mode

| Function | Description |
|----------|-------------|
| `PWR_EnterSleepMode(mode)` | Enter Sleep mode |
| `PWR_SleepFor(ms)` | Sleep for duration |

### Stop Mode

| Function | Description |
|----------|-------------|
| `PWR_EnterStopMode(reg, entry)` | Enter Stop mode |
| `PWR_ConfigureAfterStop()` | Restore clocks after wake |

### Standby Mode

| Function | Description |
|----------|-------------|
| `PWR_EnterStandbyMode()` | Enter Standby (no return) |
| `PWR_EnableWakeupPin(enable)` | Enable PA0 wakeup |
| `PWR_WasStandbyWakeup()` | Check if wake from Standby |
| `PWR_ClearStandbyFlag()` | Clear Standby flag |

### Voltage Detection

| Function | Description |
|----------|-------------|
| `PWR_EnablePVD(level)` | Enable voltage detector |
| `PWR_DisablePVD()` | Disable voltage detector |
| `PWR_GetPVDStatus()` | Get power status |
| `PWR_EnablePVDInterrupt()` | Enable PVD interrupt |

### Backup Registers

| Function | Description |
|----------|-------------|
| `PWR_EnableBackupAccess()` | Enable backup domain access |
| `PWR_DisableBackupAccess()` | Disable backup domain access |
| `PWR_WriteBackupRegister(idx, data)` | Write to backup register |
| `PWR_ReadBackupRegister(idx, data)` | Read from backup register |

## Standby Wakeup Detection

Check wakeup source at the start of `main()`:

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* Check if waking from Standby */
    if (PWR_WasStandbyWakeup())
    {
        printf("Woke from Standby!\n");
        PWR_ClearStandbyFlag();

        /* Recover saved data */
        uint32_t savedData;
        PWR_ReadBackupRegister(0, &savedData);
    }

    /* Rest of initialization... */
}
```

## PVD (Voltage Detector) Usage

Monitor power supply and save data before power loss:

```c
/* Enable PVD at 2.9V threshold */
PWR_EnablePVD(PWR_PVD_LEVEL_2V9);
PWR_EnablePVDInterrupt();

/* In stm32f4xx_it.c */
void PVD_IRQHandler(void)
{
    HAL_PWR_PVD_IRQHandler();
}

/* Callback when voltage drops */
void HAL_PWR_PVDCallback(void)
{
    /* Power is low! Save critical data */
    SaveCriticalData();

    /* Or enter low-power mode */
    PWR_EnterStandbyMode();
}
```

## Backup Registers

20 backup registers (32-bit each) retain data during:
- Sleep mode
- Stop mode
- Standby mode
- VDD power loss (if VBAT connected)

```c
/* Write configuration */
PWR_WriteBackupRegister(0, CONFIG_MAGIC);
PWR_WriteBackupRegister(1, deviceId);
PWR_WriteBackupRegister(2, calibrationValue);

/* Read back after wake */
uint32_t magic, id, cal;
PWR_ReadBackupRegister(0, &magic);
PWR_ReadBackupRegister(1, &id);
PWR_ReadBackupRegister(2, &cal);
```

## Important Notes

### Stop Mode Clock Recovery
After Stop mode wakeup, the system runs on HSI (16 MHz). You must reconfigure clocks:

```c
PWR_EnterStopMode(PWR_REGULATOR_LOW_POWER, PWR_STOP_ENTRY_WFI);

/* After wakeup */
PWR_ConfigureAfterStop();  /* Restores HSE and PLL */
```

### Standby Mode Caution
- **All RAM is lost** - only backup registers survive
- System **resets** on wakeup (starts from main())
- Use `PWR_WasStandbyWakeup()` to detect wake source

### VBAT Connection
For backup registers to survive VDD loss:
- Connect battery (1.65V-3.6V) to VBAT pin
- Or connect VBAT to VDD if not using battery backup

## Power Optimization Tips

1. **Disable unused peripherals**
   ```c
   __HAL_RCC_GPIOA_CLK_DISABLE();
   ```

2. **Use Stop mode when idle**
   - Much lower power than Sleep
   - Fast wakeup with RAM retention

3. **Use voltage scaling**
   - Lower voltage = lower power
   - Must match clock frequency

4. **Reduce clock frequency when possible**
   ```c
   PWR_EnableLowPowerMode();  /* Scale 3 */
   ```

## Status Codes

| Status | Description |
|--------|-------------|
| `PWR_OK` | Success |
| `PWR_ERROR` | General error |
| `PWR_INVALID_PARAM` | Bad parameter |
| `PWR_TIMEOUT` | Operation timeout |
| `PWR_NOT_READY` | System not ready |
