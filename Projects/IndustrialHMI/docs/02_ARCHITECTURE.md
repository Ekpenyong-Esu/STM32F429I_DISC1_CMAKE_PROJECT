# Phase 2: System Architecture

## 2.1 Project Structure

```
Projects/IndustrialHMI/
├── README.md
├── docs/
│   ├── 01_REQUIREMENTS.md
│   ├── 02_ARCHITECTURE.md
│   ├── 03_UI_FRAMEWORK.md
│   ├── 04_COMMUNICATION.md
│   ├── 05_FEATURES.md
│   ├── 06_TESTING.md
│   └── 07_PRODUCTION.md
├── src/
│   ├── app/                    # Application layer
│   │   ├── screens/            # Screen implementations
│   │   │   ├── screen_main.c
│   │   │   ├── screen_monitor.c
│   │   │   ├── screen_alarm.c
│   │   │   ├── screen_trend.c
│   │   │   ├── screen_config.c
│   │   │   └── screen_login.c
│   │   ├── app_main.c          # Application entry point
│   │   └── app_config.h        # Application configuration
│   ├── ui/                     # UI Framework
│   │   ├── core/
│   │   │   ├── ui_core.c       # Core UI engine
│   │   │   ├── ui_screen.c     # Screen manager
│   │   │   ├── ui_touch.c      # Touch input handler
│   │   │   └── ui_render.c     # Rendering engine
│   │   ├── widgets/
│   │   │   ├── widget_button.c
│   │   │   ├── widget_label.c
│   │   │   ├── widget_gauge.c
│   │   │   ├── widget_chart.c
│   │   │   ├── widget_led.c
│   │   │   ├── widget_slider.c
│   │   │   ├── widget_numpad.c
│   │   │   └── widget_keyboard.c
│   │   ├── themes/
│   │   │   ├── theme_industrial.c
│   │   │   └── theme_modern.c
│   │   └── fonts/
│   │       ├── font_8x16.c
│   │       ├── font_16x24.c
│   │       └── font_icons.c
│   ├── middleware/             # Middleware layer
│   │   ├── data/
│   │   │   ├── data_manager.c  # Tag/variable management
│   │   │   └── data_types.h
│   │   ├── alarm/
│   │   │   ├── alarm_manager.c
│   │   │   └── alarm_history.c
│   │   ├── logger/
│   │   │   ├── data_logger.c
│   │   │   └── file_manager.c
│   │   └── comm/
│   │       ├── modbus_rtu.c
│   │       ├── modbus_tcp.c
│   │       └── comm_manager.c
│   └── config/
│       ├── hmi_config.h        # Global HMI configuration
│       └── pin_config.h        # Pin assignments
├── inc/                        # Header files (mirrors src/)
├── assets/                     # Graphics assets
│   ├── icons/
│   ├── images/
│   └── splash/
└── tests/                      # Unit tests
```

---

## 2.2 Software Architecture Layers

### Layer 1: Hardware Abstraction (HAL)
**✅ Already implemented in main project Peripherals folder!**

```c
// Using existing tested drivers from ../../Peripherals/
#include "Peripherals/LTDC/ltdc.h"          // LCD display (tested)
#include "Peripherals/TOUCHSCREEN/touchscreen.h" // Touch input (tested)
#include "Peripherals/UART/uart.h"          // For RS485/Modbus + Debug console
#include "Peripherals/RTC/rtc.h"            // DS3231 real-time clock (tested)
#include "Peripherals/FLASH/flash.h"        // Internal flash for data logging
#include "Peripherals/GPIO/gpio.h"          // LEDs, relay control
#include "Peripherals/RELAY/relay.h"        // Relay driver (tested)
#include "Peripherals/BUZZER/buzzer.h"      // Active/passive buzzer (tested)
#include "Peripherals/DHT/dht.h"            // DHT22 temp/humidity (tested)
#include "Peripherals/CAN/can.h"            // CAN bus communication (available)
#include "Peripherals/ADC/adc.h"            // Analog inputs (if needed)
```

**Driver Status**: All core drivers verified for STM32F429I-DISC1 compatibility
**Documentation**: See [../../GETTING_STARTED.md](../../GETTING_STARTED.md) for driver testing

### Layer 2: UI Framework

```c
// ui_core.h - Core UI types and functions
typedef struct {
    uint16_t x, y;
    uint16_t width, height;
} UI_Rect_t;

typedef struct {
    uint16_t x, y;
    bool pressed;
    bool released;
    bool held;
} UI_TouchState_t;

typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_TOUCH_DOWN,
    UI_EVENT_TOUCH_UP,
    UI_EVENT_TOUCH_MOVE,
    UI_EVENT_TIMER,
    UI_EVENT_DATA_UPDATE
} UI_EventType_t;

typedef struct UI_Widget UI_Widget_t;

typedef void (*UI_DrawFunc)(UI_Widget_t* widget);
typedef void (*UI_EventFunc)(UI_Widget_t* widget, UI_EventType_t event);

struct UI_Widget {
    uint16_t id;
    UI_Rect_t bounds;
    bool visible;
    bool enabled;
    UI_DrawFunc draw;
    UI_EventFunc onEvent;
    void* userData;
    UI_Widget_t* next;  // Linked list
};
```

### Layer 3: Screen Manager

```c
// ui_screen.h - Screen management
typedef struct UI_Screen UI_Screen_t;

typedef void (*UI_ScreenInitFunc)(UI_Screen_t* screen);
typedef void (*UI_ScreenUpdateFunc)(UI_Screen_t* screen);
typedef void (*UI_ScreenDestroyFunc)(UI_Screen_t* screen);

struct UI_Screen {
    uint8_t id;
    const char* name;
    UI_Widget_t* widgets;       // Widget list
    UI_ScreenInitFunc init;
    UI_ScreenUpdateFunc update;
    UI_ScreenDestroyFunc destroy;
    void* userData;
};

// Screen manager API
void UI_ScreenManager_Init(void);
void UI_ScreenManager_Push(UI_Screen_t* screen);
void UI_ScreenManager_Pop(void);
void UI_ScreenManager_Switch(UI_Screen_t* screen);
UI_Screen_t* UI_ScreenManager_GetCurrent(void);
void UI_ScreenManager_Update(void);
```

### Layer 4: Data Manager

```c
// data_manager.h - Tag/Variable management
typedef enum {
    DATA_TYPE_BOOL,
    DATA_TYPE_INT16,
    DATA_TYPE_INT32,
    DATA_TYPE_FLOAT,
    DATA_TYPE_STRING
} DataType_t;

typedef struct {
    uint16_t id;
    const char* name;
    DataType_t type;
    union {
        bool boolVal;
        int16_t int16Val;
        int32_t int32Val;
        float floatVal;
        char strVal[32];
    } value;
    float minValue;
    float maxValue;
    const char* unit;
    bool alarmsEnabled;
    float alarmHigh;
    float alarmLow;
} DataTag_t;

// Data manager API
void DataManager_Init(void);
DataTag_t* DataManager_GetTag(uint16_t id);
DataTag_t* DataManager_GetTagByName(const char* name);
void DataManager_SetValue(uint16_t id, void* value);
void DataManager_RegisterCallback(uint16_t id, void (*callback)(DataTag_t*));
```

### Layer 5: Application Screens

```c
// screen_monitor.c - Example screen implementation
static UI_Widget_t* gauge_temp;
static UI_Widget_t* gauge_pressure;
static UI_Widget_t* led_status;
static UI_Widget_t* btn_start;
static UI_Widget_t* btn_stop;

void Screen_Monitor_Init(UI_Screen_t* screen)
{
    // Create widgets
    gauge_temp = Widget_Gauge_Create(20, 50, 100, 100);
    Widget_Gauge_SetRange(gauge_temp, 0, 100);
    Widget_Gauge_SetLabel(gauge_temp, "Temperature");
    Widget_Gauge_SetUnit(gauge_temp, "°C");
    Widget_Gauge_BindTag(gauge_temp, TAG_TEMPERATURE);
    
    gauge_pressure = Widget_Gauge_Create(140, 50, 100, 100);
    Widget_Gauge_SetRange(gauge_pressure, 0, 10);
    Widget_Gauge_SetLabel(gauge_pressure, "Pressure");
    Widget_Gauge_SetUnit(gauge_pressure, "bar");
    Widget_Gauge_BindTag(gauge_pressure, TAG_PRESSURE);
    
    led_status = Widget_LED_Create(20, 170, 30, 30);
    Widget_LED_BindTag(led_status, TAG_RUNNING);
    
    btn_start = Widget_Button_Create(60, 220, 80, 40, "START");
    Widget_Button_SetCallback(btn_start, OnStartPressed);
    
    btn_stop = Widget_Button_Create(150, 220, 80, 40, "STOP");
    Widget_Button_SetCallback(btn_stop, OnStopPressed);
    
    // Add to screen
    UI_Screen_AddWidget(screen, gauge_temp);
    UI_Screen_AddWidget(screen, gauge_pressure);
    UI_Screen_AddWidget(screen, led_status);
    UI_Screen_AddWidget(screen, btn_start);
    UI_Screen_AddWidget(screen, btn_stop);
}
```

---

## 2.3 State Machine

### Main Application State Machine

```
                    ┌─────────────┐
                    │   STARTUP   │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  INIT_HW    │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │ LOAD_CONFIG │
                    └──────┬──────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
    ┌────▼────┐     ┌──────▼──────┐    ┌─────▼─────┐
    │  LOGIN  │────►│    MAIN     │◄───│   ALARM   │
    └─────────┘     └──────┬──────┘    └───────────┘
                           │
         ┌─────────┬───────┼───────┬─────────┐
         │         │       │       │         │
    ┌────▼────┐ ┌──▼───┐ ┌─▼──┐ ┌──▼───┐ ┌───▼────┐
    │ MONITOR │ │ALARM │ │TREND│ │CONFIG│ │DIAGNOS │
    └─────────┘ └──────┘ └────┘ └──────┘ └────────┘
```

### Screen Navigation Flow

```c
// Screen IDs
typedef enum {
    SCREEN_SPLASH = 0,
    SCREEN_LOGIN,
    SCREEN_MAIN,
    SCREEN_MONITOR,
    SCREEN_ALARM_LIST,
    SCREEN_ALARM_HISTORY,
    SCREEN_TREND,
    SCREEN_CONFIG,
    SCREEN_CONFIG_COMM,
    SCREEN_CONFIG_ALARM,
    SCREEN_CONFIG_DISPLAY,
    SCREEN_DIAGNOSTICS,
    SCREEN_ABOUT,
    SCREEN_COUNT
} ScreenID_t;
```

---

## 2.4 Memory Map

### RAM Usage (256KB available)
```
┌─────────────────────────────────────┐ 0x20030000
│       Stack (8KB)                   │
├─────────────────────────────────────┤ 0x2002E000
│       Heap (16KB)                   │
├─────────────────────────────────────┤ 0x2002A000
│       Data Logger Buffer (32KB)     │
├─────────────────────────────────────┤ 0x20022000
│       Trend Data Buffer (16KB)      │
├─────────────────────────────────────┤ 0x2001E000
│       Alarm History (8KB)           │
├─────────────────────────────────────┤ 0x2001C000
│       Screen Widgets (16KB)         │
├─────────────────────────────────────┤ 0x20018000
│       Data Tags (8KB)               │
├─────────────────────────────────────┤ 0x20016000
│       Modbus Buffers (4KB)          │
├─────────────────────────────────────┤ 0x20015000
│       UI Frame Buffer (partial)     │
│       (Main buffer in SDRAM)        │
├─────────────────────────────────────┤ 0x20010000
│       Global Variables (64KB)       │
└─────────────────────────────────────┘ 0x20000000
```

### SDRAM Usage (8MB available)
```
┌─────────────────────────────────────┐ 0xD0800000
│       Reserved                      │
├─────────────────────────────────────┤ 0xD0600000
│       Image/Icon Cache (2MB)        │
├─────────────────────────────────────┤ 0xD0400000
│       Font Cache (1MB)              │
├─────────────────────────────────────┤ 0xD0300000
│       Trend History (1MB)           │
├─────────────────────────────────────┤ 0xD0200000
│       Double Frame Buffer (300KB x2)│
│       (240x320x2 = 153.6KB each)    │
├─────────────────────────────────────┤ 0xD0100000
│       Working Buffer (1MB)          │
└─────────────────────────────────────┘ 0xD0000000
```

---

## 2.5 Task Architecture (FreeRTOS)

```c
// Task priorities (higher = more priority)
#define TASK_PRIORITY_COMM      (configMAX_PRIORITIES - 1)  // Highest
#define TASK_PRIORITY_ALARM     (configMAX_PRIORITIES - 2)
#define TASK_PRIORITY_UI        (configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_LOGGER    (configMAX_PRIORITIES - 4)
#define TASK_PRIORITY_IDLE      0                           // Lowest

// Task definitions
void Task_Communication(void* params);  // Modbus polling, data exchange
void Task_AlarmManager(void* params);   // Check alarm conditions
void Task_UI(void* params);             // Screen updates, touch handling
void Task_DataLogger(void* params);     // SD card logging
void Task_Watchdog(void* params);       // System health monitoring

// Stack sizes
#define STACK_SIZE_COMM     512
#define STACK_SIZE_ALARM    256
#define STACK_SIZE_UI       1024
#define STACK_SIZE_LOGGER   512
#define STACK_SIZE_WDG      128
```

### Inter-Task Communication

```c
// Queues
QueueHandle_t queue_dataUpdate;     // Data changes from comm to UI
QueueHandle_t queue_alarmEvent;     // Alarm events
QueueHandle_t queue_logData;        // Data to log

// Semaphores
SemaphoreHandle_t mutex_dataAccess; // Protect shared data
SemaphoreHandle_t mutex_display;    // Protect LCD access
SemaphoreHandle_t sem_touchEvent;   // Touch event notification

// Event Groups
EventGroupHandle_t events_system;
#define EVENT_COMM_CONNECTED    (1 << 0)
#define EVENT_ALARM_ACTIVE      (1 << 1)
#define EVENT_SD_READY          (1 << 2)
#define EVENT_CONFIG_CHANGED    (1 << 3)
```

---

## 2.6 Configuration System

### Configuration Structure

```c
// hmi_config.h
typedef struct {
    // Display settings
    uint8_t brightness;         // 0-100%
    uint8_t screenTimeout;      // Minutes, 0=disabled
    uint8_t theme;              // Theme index
    uint8_t language;           // Language code
    
    // Communication settings
    struct {
        uint8_t slaveAddress;   // Modbus address
        uint32_t baudRate;      // 9600, 19200, 38400, 115200
        uint8_t parity;         // None, Even, Odd
        uint8_t stopBits;       // 1, 2
    } modbus;
    
    // Alarm settings
    struct {
        bool soundEnabled;
        uint8_t soundVolume;    // 0-100%
        uint16_t ackTimeout;    // Seconds
    } alarm;
    
    // Data logging
    struct {
        bool enabled;
        uint16_t interval;      // Seconds
        uint16_t maxFiles;      // Max log files to keep
    } logging;
    
    // Security
    struct {
        bool pinRequired;
        uint16_t pin;           // 4-digit PIN
        uint16_t lockTimeout;   // Minutes
    } security;
    
    uint32_t crc;               // Config CRC
} HMI_Config_t;
```

### Configuration Storage

```c
// Store in Flash (last sector) or EEPROM emulation
#define CONFIG_FLASH_ADDR    0x081E0000  // Sector 23

void Config_Load(HMI_Config_t* config);
void Config_Save(const HMI_Config_t* config);
void Config_LoadDefaults(HMI_Config_t* config);
bool Config_Validate(const HMI_Config_t* config);
```

---

## 2.7 Next Steps

1. ✅ Architecture documented
2. ➡️ Proceed to `03_UI_FRAMEWORK.md` for UI implementation
3. Create project directory structure
4. Implement UI core module
5. Build widget library

---

## Checklist

- [ ] Architecture reviewed
- [ ] Project structure created
- [ ] Memory allocation planned
- [ ] Task priorities defined
- [ ] Configuration structure defined
