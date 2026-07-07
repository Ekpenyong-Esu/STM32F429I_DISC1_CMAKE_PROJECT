# RTC (Real-Time Clock) Peripheral Driver

This directory contains the RTC peripheral driver implementation for the STM32F429I Discovery board.

## Files

- `rtc.h` - Header file with RTC function prototypes and type definitions
- `rtc.c` - Implementation of RTC functions
- `rtc_example.h` - Header file for RTC examples
- `rtc_example.c` - Example implementations demonstrating RTC usage
- `README.md` - This documentation file

## Features

### Core RTC Functions
- **Initialization/Deinitialization**: `RTC_Init()`, `RTC_DeInit()`
- **Time Management**: `RTC_SetTime()`, `RTC_GetTime()`
- **Date Management**: `RTC_SetDate()`, `RTC_GetDate()`
- **Alarm Management**: `RTC_SetAlarm()`, `RTC_GetAlarm()`, `RTC_DisableAlarm()`
- **Timestamp Support**: `RTC_GetTimestamp()`, `RTC_SetTimestamp()`
- **Formatting**: `RTC_FormatTimeString()`, `RTC_FormatDateString()`

### Data Types

#### RTC_Time_t
```c
typedef struct {
    uint8_t Hours;           // 0-23 (24h) or 0-12 (12h)
    uint8_t Minutes;         // 0-59
    uint8_t Seconds;         // 0-59
    uint8_t TimeFormat;      // AM/PM for 12h format
    uint32_t DayLightSaving; // Daylight saving setting
    uint32_t StoreOperation; // Store operation setting
} RTC_Time_t;
```

#### RTC_Date_t
```c
typedef struct {
    uint8_t WeekDay;  // 1-7 (Monday=1, Sunday=7)
    uint8_t Month;    // 1-12
    uint8_t Date;     // 1-31
    uint8_t Year;     // 0-99 (years since 2000)
} RTC_Date_t;
```

#### RTC_Alarm_t
```c
typedef struct {
    RTC_Time_t AlarmTime;          // Alarm time
    uint32_t AlarmMask;            // Alarm mask settings
    uint32_t AlarmSubSecondMask;   // Sub-seconds mask
    uint32_t AlarmDateWeekDaySel;  // Date or weekday selection
    uint8_t AlarmDateWeekDay;      // Alarm date/weekday value
    uint32_t Alarm;                // Alarm A or B
} RTC_Alarm_t;
```

## Usage Examples

### Basic Time/Date Operations
```c
#include "rtc.h"

// Initialize RTC
RTC_Init();

// Set time to 14:30:00
RTC_Time_t time = {
    .Hours = 14,
    .Minutes = 30,
    .Seconds = 0,
    .TimeFormat = RTC_HOURFORMAT_24
};
RTC_SetTime(&time);

// Set date to September 4, 2025
RTC_Date_t date = {
    .WeekDay = RTC_WEEKDAY_THURSDAY,
    .Month = RTC_MONTH_SEPTEMBER,
    .Date = 4,
    .Year = 25
};
RTC_SetDate(&date);

// Read current time and date
RTC_GetTime(&time);
RTC_GetDate(&date);
```

### Alarm Configuration
```c
// Set alarm for 14:31:00
RTC_Alarm_t alarm = {
    .AlarmTime = {
        .Hours = 14,
        .Minutes = 31,
        .Seconds = 0,
        .TimeFormat = RTC_HOURFORMAT_24
    },
    .AlarmMask = RTC_ALARMMASK_DATEWEEKDAY,
    .Alarm = RTC_ALARM_A
};
RTC_SetAlarm(&alarm);

// Implement callback for alarm events
void RTC_AlarmCallback(uint32_t Alarm)
{
    if (Alarm == RTC_ALARM_A) {
        // Handle Alarm A event
    }
}
```

### Timestamp Operations
```c
// Set time using Unix timestamp
uint32_t timestamp = 1725465600; // Sept 4, 2024 14:00:00 UTC
RTC_SetTimestamp(timestamp);

// Get current time as timestamp
uint32_t current = RTC_GetTimestamp();
```

### String Formatting
```c
char timeStr[20], dateStr[30];

// Format time as string
RTC_FormatTimeString(&time, timeStr, sizeof(timeStr));
printf("Time: %s\n", timeStr); // Output: "14:30:00"

// Format date as string
RTC_FormatDateString(&date, dateStr, sizeof(dateStr));
printf("Date: %s\n", dateStr); // Output: "Thu 04/09/2025"
```

## Configuration Requirements

### HAL Configuration
Ensure RTC module is enabled in `stm32f4xx_hal_conf.h`:
```c
#define HAL_RTC_MODULE_ENABLED
```

### Clock Configuration
The RTC driver automatically configures:
- LSE (Low Speed External) oscillator as RTC clock source
- RTC peripheral clock enable
- Prescaler values for 1Hz RTC tick

### Hardware Requirements
- 32.768 kHz crystal oscillator (LSE) on the STM32F429I Discovery board
- Backup domain power supply (VBAT) for RTC operation during main power loss

## Examples

The `rtc_example.c` file provides several complete examples:

1. **RTC_BasicExample()** - Basic initialization and time reading
2. **RTC_SetTimeExample()** - Setting and verifying time/date
3. **RTC_AlarmExample()** - Configuring RTC alarms
4. **RTC_TimestampExample()** - Unix timestamp operations
5. **RTC_FormatExample()** - String formatting demonstrations
6. **RTC_CompleteExample()** - Runs all examples

To use the examples:
```c
#include "rtc_example.h"

int main(void)
{
    // Run all RTC examples
    RTC_CompleteExample();
    
    // Or run individual examples
    RTC_BasicExample();
    RTC_AlarmExample();
    
    return 0;
}
```

## Error Handling

All functions return `RTC_StatusTypeDef` with the following values:
- `RTC_STATUS_OK` - Operation successful
- `RTC_STATUS_ERROR` - Operation failed
- `RTC_STATUS_BUSY` - RTC peripheral busy
- `RTC_STATUS_TIMEOUT` - Operation timed out

Always check return values for robust error handling:
```c
if (RTC_Init() != RTC_STATUS_OK) {
    // Handle initialization error
}
```

## Notes

- The RTC maintains time even when main power is lost (if VBAT is connected)
- Alarm interrupts require proper NVIC configuration
- The timestamp functions use a simplified algorithm and may not be accurate for all edge cases
- For production applications, consider using a more robust timestamp conversion library
- The LSE crystal provides high accuracy timekeeping

## Integration

To integrate this RTC driver into your project:

1. Add `rtc.c` and `rtc_example.c` to your build system
2. Include `rtc.h` in your source files
3. Ensure HAL RTC module is enabled
4. Call `RTC_Init()` during system initialization
5. Implement `RTC_AlarmCallback()` if using alarms
