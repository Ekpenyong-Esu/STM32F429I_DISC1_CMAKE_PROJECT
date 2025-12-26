# Code Implementation Roadmap - Industrial HMI Project

**Purpose**: Sequential guide for writing code from first to last module  
**Priority**: Code that other modules depend on comes first  
**Total Modules**: 25+ files across 5 layers

---

## 📋 Code Implementation Order (Dependency-Based)

### LAYER 0: Core Data Types & Definitions (START HERE - Foundation)

**Why First**: All other layers depend on these definitions

#### Code 1️⃣: Create Data Types & Constants
**File**: `Projects/IndustrialHMI/src/config/types.h`
```c
#ifndef HMI_TYPES_H
#define HMI_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Sensor data types
typedef struct {
    float value;
    float min;
    float max;
    uint32_t timestamp;
    bool valid;
} SensorValue_t;

// Alarm types
typedef enum {
    ALARM_NONE = 0,
    ALARM_LOW = 1,
    ALARM_HIGH = 2,
    ALARM_CRITICAL = 3,
    ALARM_WARNING = 4
} AlarmLevel_t;

// Screen IDs
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_MONITOR = 1,
    SCREEN_ALARM = 2,
    SCREEN_TREND = 3,
    SCREEN_CONFIG = 4,
    SCREEN_ABOUT = 5
} ScreenID_t;

// Widget types
typedef enum {
    WIDGET_BUTTON = 0,
    WIDGET_LABEL = 1,
    WIDGET_GAUGE = 2,
    WIDGET_LED = 3,
    WIDGET_CHART = 4,
    WIDGET_SLIDER = 5
} WidgetType_t;

#endif
```
**Deliverable**: All data types defined, no compilation errors

---

#### Code 2️⃣: Create Configuration File
**File**: `Projects/IndustrialHMI/src/config/hmi_config.h`
```c
#ifndef HMI_CONFIG_H
#define HMI_CONFIG_H

// Display config
#define LCD_WIDTH               240
#define LCD_HEIGHT              320
#define LCD_COLOR_DEPTH         16

// UI config
#define MAX_SCREENS             6
#define MAX_WIDGETS_PER_SCREEN  20
#define MAX_SENSORS             10
#define MAX_ALARMS              20

// Communication config
#define MODBUS_BAUDRATE         9600
#define MODBUS_TIMEOUT_MS       1000
#define CAN_BAUDRATE            500000

// Timing config
#define UI_UPDATE_RATE_MS       100    // 10 FPS
#define SENSOR_READ_RATE_MS     1000   // 1 sec
#define TREND_LOG_RATE_MS       10000  // 10 sec

// Data logging
#define LOG_BUFFER_SIZE         1024
#define LOG_FLASH_SECTOR        11
#define LOG_FLASH_ADDRESS       0x080E0000

#endif
```
**Deliverable**: All configuration constants defined

---

### LAYER 1: Data Management (Build Next - Feeds UI & Communications)

#### Code 3️⃣: Create Data Manager
**File**: `Projects/IndustrialHMI/src/middleware/data/data_manager.h`
```c
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "../../config/types.h"

// Sensor value management
typedef struct {
    uint8_t id;
    char name[32];
    SensorValue_t current;
    SensorValue_t average;
    SensorValue_t min_today;
    SensorValue_t max_today;
} SensorTag_t;

// Public API
void DataMgr_Init(void);
void DataMgr_UpdateSensor(uint8_t sensor_id, float value);
SensorValue_t DataMgr_GetSensor(uint8_t sensor_id);
void DataMgr_LogTrend(void);
void DataMgr_GetStats(uint8_t sensor_id, float *avg, float *min, float *max);

#endif
```
**File**: `Projects/IndustrialHMI/src/middleware/data/data_manager.c`
```c
#include "data_manager.h"
#include <string.h>

#define MAX_SENSORS 10
static SensorTag_t sensors[MAX_SENSORS];

void DataMgr_Init(void) {
    memset(sensors, 0, sizeof(sensors));
    // Initialize sensor names, min/max values
}

void DataMgr_UpdateSensor(uint8_t sensor_id, float value) {
    if (sensor_id >= MAX_SENSORS) return;
    
    sensors[sensor_id].current.value = value;
    sensors[sensor_id].current.timestamp = HAL_GetTick();
    sensors[sensor_id].current.valid = true;
    
    // Update min/max
    if (value < sensors[sensor_id].min_today.value) 
        sensors[sensor_id].min_today.value = value;
    if (value > sensors[sensor_id].max_today.value) 
        sensors[sensor_id].max_today.value = value;
}

SensorValue_t DataMgr_GetSensor(uint8_t sensor_id) {
    if (sensor_id >= MAX_SENSORS) return (SensorValue_t){0};
    return sensors[sensor_id].current;
}

// ... more functions
```
**Deliverable**: Data manager stores/retrieves sensor values, can calculate stats

---

#### Code 4️⃣: Create Alarm Manager
**File**: `Projects/IndustrialHMI/src/middleware/alarm/alarm_manager.h`
```c
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include "../../config/types.h"

typedef struct {
    uint8_t id;
    uint8_t sensor_id;
    AlarmLevel_t level;
    float threshold;
    bool active;
    uint32_t trigger_time;
    bool acknowledged;
} Alarm_t;

// Public API
void AlarmMgr_Init(void);
void AlarmMgr_CheckAlarms(void);
void AlarmMgr_SetAlarm(uint8_t alarm_id, uint8_t sensor_id, AlarmLevel_t level, float threshold);
Alarm_t* AlarmMgr_GetAlarm(uint8_t alarm_id);
void AlarmMgr_AcknowledgeAlarm(uint8_t alarm_id);
uint8_t AlarmMgr_GetActiveAlarmCount(void);
void AlarmMgr_TriggerAction(Alarm_t *alarm);  // Buzzer, LED, Relay

#endif
```
**Deliverable**: Alarm manager detects threshold violations, triggers actions

---

### LAYER 2: UI Framework Core (Build After Data Manager)

#### Code 5️⃣: Create UI Core Engine
**File**: `Projects/IndustrialHMI/src/ui/core/ui_core.h`
```c
#ifndef UI_CORE_H
#define UI_CORE_H

#include "../../config/types.h"

typedef struct UI_Widget {
    uint16_t x, y;
    uint16_t width, height;
    WidgetType_t type;
    void (*draw)(struct UI_Widget *self);
    void (*on_touch)(struct UI_Widget *self, uint16_t touch_x, uint16_t touch_y);
    void *data;  // Widget-specific data
} UI_Widget_t;

typedef struct {
    ScreenID_t id;
    char title[32];
    UI_Widget_t widgets[20];
    uint8_t widget_count;
} UI_Screen_t;

// Public API
void UI_Init(void);
void UI_Update(void);
void UI_Render(void);
void UI_HandleTouch(uint16_t x, uint16_t y);
void UI_RegisterWidget(UI_Widget_t *widget);

#endif
```
**Deliverable**: UI engine initializes, can draw and handle touch

---

#### Code 6️⃣: Create Screen Manager
**File**: `Projects/IndustrialHMI/src/ui/core/ui_screen.h`
```c
#ifndef UI_SCREEN_H
#define UI_SCREEN_H

#include "ui_core.h"

// Screen management
void ScreenMgr_Init(void);
void ScreenMgr_SwitchScreen(ScreenID_t screen_id);
ScreenID_t ScreenMgr_GetCurrentScreen(void);
void ScreenMgr_RegisterScreen(UI_Screen_t *screen);
UI_Screen_t* ScreenMgr_GetScreen(ScreenID_t id);

#endif
```
**Deliverable**: Can switch between screens, screens display correctly

---

#### Code 7️⃣: Create Touch Input Handler
**File**: `Projects/IndustrialHMI/src/ui/core/ui_touch.c`
```c
#include "ui_core.h"
#include "../../../Peripherals/TOUCHSCREEN/touchscreen.h"

#define TOUCH_DEBOUNCE_MS 20

static uint32_t last_touch_time = 0;
static bool touch_pressed = false;

void UI_TouchHandler_Init(void) {
    Touchscreen_Init();
}

void UI_TouchHandler_Update(void) {
    TouchData_t touch;
    uint32_t current_time = HAL_GetTick();
    
    if (Touchscreen_GetState(&touch) == TOUCH_DETECTED) {
        if ((current_time - last_touch_time) > TOUCH_DEBOUNCE_MS) {
            UI_HandleTouch(touch.X, touch.Y);
            last_touch_time = current_time;
            touch_pressed = true;
        }
    } else {
        touch_pressed = false;
    }
}
```
**Deliverable**: Touch input debounced and routed to widgets

---

### LAYER 3: Widget Library (Build After UI Core)

#### Code 8️⃣: Create Button Widget
**File**: `Projects/IndustrialHMI/src/ui/widgets/widget_button.h`
```c
#ifndef WIDGET_BUTTON_H
#define WIDGET_BUTTON_H

#include "../core/ui_core.h"

typedef struct {
    char label[32];
    void (*callback)(void);
    bool pressed;
    uint16_t bg_color;
    uint16_t fg_color;
} ButtonWidget_t;

UI_Widget_t* Button_Create(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                           const char *label, void (*callback)(void));
void Button_Draw(UI_Widget_t *widget);
void Button_OnTouch(UI_Widget_t *widget, uint16_t x, uint16_t y);

#endif
```
**Deliverable**: Button widget can be created, drawn, and clicked

---

#### Code 9️⃣: Create Label Widget
**File**: `Projects/IndustrialHMI/src/ui/widgets/widget_label.h`
```c
#ifndef WIDGET_LABEL_H
#define WIDGET_LABEL_H

#include "../core/ui_core.h"

typedef struct {
    char text[256];
    uint16_t color;
    uint8_t font_size;  // 0=8x16, 1=16x24
} LabelWidget_t;

UI_Widget_t* Label_Create(uint16_t x, uint16_t y, const char *text);
void Label_Update(UI_Widget_t *widget, const char *text);
void Label_Draw(UI_Widget_t *widget);

#endif
```
**Deliverable**: Label widget displays text with formatting

---

#### Code 🔟: Create Gauge Widget
**File**: `Projects/IndustrialHMI/src/ui/widgets/widget_gauge.h`
```c
#ifndef WIDGET_GAUGE_H
#define WIDGET_GAUGE_H

#include "../core/ui_core.h"

typedef struct {
    float value;        // Current needle position
    float min_value;
    float max_value;
    char unit[16];      // "°C", "bar", etc.
    uint16_t needle_color;
} GaugeWidget_t;

UI_Widget_t* Gauge_Create(uint16_t x, uint16_t y, uint16_t diameter,
                          float min, float max, const char *unit);
void Gauge_SetValue(UI_Widget_t *widget, float value);
void Gauge_Draw(UI_Widget_t *widget);

#endif
```
**Deliverable**: Gauge draws with needle at correct position

---

#### Code 1️⃣1️⃣: Create Chart Widget
**File**: `Projects/IndustrialHMI/src/ui/widgets/widget_chart.h`
```c
#ifndef WIDGET_CHART_H
#define WIDGET_CHART_H

#include "../core/ui_core.h"

typedef struct {
    float *data;           // Ring buffer of values
    uint16_t data_size;
    uint16_t data_count;   // Current entries
    float min_value;
    float max_value;
    uint16_t line_color;
} ChartWidget_t;

UI_Widget_t* Chart_Create(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint16_t max_points);
void Chart_AddPoint(UI_Widget_t *widget, float value);
void Chart_Draw(UI_Widget_t *widget);

#endif
```
**Deliverable**: Chart plots trend line with auto-scaling

---

### LAYER 4: Application Screens (Build After Widgets)

#### Code 1️⃣2️⃣: Create Main Menu Screen
**File**: `Projects/IndustrialHMI/src/app/screens/screen_main.h`
```c
#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include "../../ui/core/ui_core.h"

UI_Screen_t* MainScreen_Create(void);
void MainScreen_Init(void);

#endif
```
**File**: `Projects/IndustrialHMI/src/app/screens/screen_main.c`
```c
#include "screen_main.h"
#include "../../ui/widgets/widget_button.h"
#include "../../ui/widgets/widget_label.h"

static UI_Screen_t main_screen;

// Button callbacks
void btn_monitor_callback(void) {
    ScreenMgr_SwitchScreen(SCREEN_MONITOR);
}

void btn_alarm_callback(void) {
    ScreenMgr_SwitchScreen(SCREEN_ALARM);
}

// ... more callbacks

UI_Screen_t* MainScreen_Create(void) {
    main_screen.id = SCREEN_MENU;
    strcpy(main_screen.title, "HMI Panel");
    main_screen.widget_count = 0;
    
    // Add buttons
    UI_Widget_t *btn1 = Button_Create(10, 50, 100, 60, "Monitor", btn_monitor_callback);
    UI_Widget_t *btn2 = Button_Create(130, 50, 100, 60, "Alarms", btn_alarm_callback);
    // ... add more buttons
    
    return &main_screen;
}
```
**Deliverable**: Main menu screen with buttons for other screens

---

#### Code 1️⃣3️⃣: Create Monitor Screen
**File**: `Projects/IndustrialHMI/src/app/screens/screen_monitor.c`
```c
#include "screen_monitor.h"
#include "../../ui/widgets/widget_gauge.h"
#include "../../ui/widgets/widget_label.h"
#include "../../middleware/data/data_manager.h"

static UI_Screen_t monitor_screen;

UI_Screen_t* MonitorScreen_Create(void) {
    monitor_screen.id = SCREEN_MONITOR;
    strcpy(monitor_screen.title, "Monitor");
    
    // Add gauges for sensors
    UI_Widget_t *temp_gauge = Gauge_Create(20, 80, 80, 0, 100, "°C");
    UI_Widget_t *pressure_gauge = Gauge_Create(160, 80, 80, 0, 10, "bar");
    
    // Add labels for values
    UI_Widget_t *temp_label = Label_Create(50, 180, "Temperature");
    UI_Widget_t *pressure_label = Label_Create(190, 180, "Pressure");
    
    return &monitor_screen;
}
```
**Deliverable**: Monitor screen displays real-time sensor values in gauges

---

#### Code 1️⃣4️⃣: Create Alarm Screen
**File**: `Projects/IndustrialHMI/src/app/screens/screen_alarm.c`
```c
#include "screen_alarm.h"
#include "../../ui/widgets/widget_label.h"
#include "../../ui/widgets/widget_button.h"
#include "../../middleware/alarm/alarm_manager.h"

static UI_Screen_t alarm_screen;

void update_alarm_display(void) {
    // Get active alarms
    // Display each with ACK button
}

UI_Screen_t* AlarmScreen_Create(void) {
    alarm_screen.id = SCREEN_ALARM;
    strcpy(alarm_screen.title, "Active Alarms");
    
    update_alarm_display();
    
    return &alarm_screen;
}
```
**Deliverable**: Alarm screen shows active alarms with ACK buttons

---

#### Code 1️⃣5️⃣: Create Trend Screen
**File**: `Projects/IndustrialHMI/src/app/screens/screen_trend.c`
```c
#include "screen_trend.h"
#include "../../ui/widgets/widget_chart.h"
#include "../../middleware/data/data_manager.h"

static UI_Screen_t trend_screen;

UI_Screen_t* TrendScreen_Create(void) {
    trend_screen.id = SCREEN_TREND;
    strcpy(trend_screen.title, "Trends");
    
    // Add chart for temperature trend
    UI_Widget_t *chart = Chart_Create(10, 50, 220, 150, 100);
    
    return &trend_screen;
}
```
**Deliverable**: Trend screen plots sensor history over time

---

#### Code 1️⃣6️⃣: Create Config Screen
**File**: `Projects/IndustrialHMI/src/app/screens/screen_config.c`
```c
#include "screen_config.h"
#include "../../ui/widgets/widget_slider.h"
#include "../../ui/widgets/widget_label.h"

static UI_Screen_t config_screen;

UI_Screen_t* ConfigScreen_Create(void) {
    config_screen.id = SCREEN_CONFIG;
    strcpy(config_screen.title, "Configuration");
    
    // Add sliders/inputs for:
    // - Alarm thresholds
    // - Display brightness
    // - Date/Time
    
    return &config_screen;
}
```
**Deliverable**: Config screen allows setting parameters

---

### LAYER 5: Communication & Logging (Build After Screens)

#### Code 1️⃣7️⃣: Create Modbus RTU Driver
**File**: `Projects/IndustrialHMI/src/middleware/comm/modbus_rtu.h`
```c
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stdint.h>

typedef struct {
    uint8_t slave_id;
    uint16_t function_code;
    uint16_t start_address;
    uint16_t quantity;
    uint16_t *values;
} ModbusRequest_t;

// Public API
void ModbusRTU_Init(void);
bool ModbusRTU_ReadRegisters(uint8_t slave, uint16_t addr, uint16_t count, uint16_t *values);
bool ModbusRTU_WriteRegister(uint8_t slave, uint16_t addr, uint16_t value);
void ModbusRTU_Update(void);

#endif
```
**Deliverable**: Can read/write Modbus registers to PLC

---

#### Code 1️⃣8️⃣: Create Data Logger
**File**: `Projects/IndustrialHMI/src/middleware/logger/data_logger.h`
```c
#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdint.h>

typedef struct {
    uint32_t timestamp;
    float temp;
    float pressure;
    float humidity;
    uint8_t alarms;
} LogEntry_t;

// Public API
void Logger_Init(void);
void Logger_LogData(LogEntry_t *entry);
void Logger_ReadLog(uint16_t index, LogEntry_t *entry);
uint16_t Logger_GetLogCount(void);
void Logger_ClearLog(void);

#endif
```
**Deliverable**: Can log and retrieve sensor data from flash

---

#### Code 1️⃣9️⃣: Create Communication Manager
**File**: `Projects/IndustrialHMI/src/middleware/comm/comm_manager.h`
```c
#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

typedef enum {
    COMM_MODBUS_RTU = 0,
    COMM_CAN_BUS = 1,
    COMM_WIFI = 2
} CommProtocol_t;

// Protocol abstraction
void CommMgr_Init(CommProtocol_t protocol);
void CommMgr_SendData(uint16_t *data, uint16_t count);
void CommMgr_ReceiveData(void);
void CommMgr_Update(void);

#endif
```
**Deliverable**: Protocol-agnostic communication interface

---

### LAYER 6: Main Application (Final Integration)

#### Code 2️⃣0️⃣: Create Application Main
**File**: `Projects/IndustrialHMI/src/app/app_main.c`
```c
#include "FreeRTOS.h"
#include "task.h"
#include "../ui/core/ui_core.h"
#include "../middleware/data/data_manager.h"
#include "../middleware/alarm/alarm_manager.h"
#include "../middleware/comm/comm_manager.h"
#include "../middleware/logger/data_logger.h"

// FreeRTOS Task prototypes
void vTaskUI(void *pvParameters);
void vTaskSensor(void *pvParameters);
void vTaskAlarm(void *pvParameters);
void vTaskComm(void *pvParameters);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    // Initialize all modules
    DataMgr_Init();
    AlarmMgr_Init();
    Logger_Init();
    UI_Init();
    CommMgr_Init(COMM_MODBUS_RTU);
    
    // Create FreeRTOS tasks
    xTaskCreate(vTaskUI, "UI", 512, NULL, 2, NULL);
    xTaskCreate(vTaskSensor, "Sensor", 256, NULL, 2, NULL);
    xTaskCreate(vTaskAlarm, "Alarm", 256, NULL, 2, NULL);
    xTaskCreate(vTaskComm, "Comm", 256, NULL, 1, NULL);
    
    // Start scheduler
    vTaskStartScheduler();
    
    while(1);
}

// Task implementations
void vTaskUI(void *pvParameters) {
    while(1) {
        UI_Update();
        UI_Render();
        vTaskDelay(pdMS_TO_TICKS(100));  // 10 FPS
    }
}

void vTaskSensor(void *pvParameters) {
    while(1) {
        // Read DHT22, update data manager
        float temp = DHT_ReadTemperature();
        float hum = DHT_ReadHumidity();
        DataMgr_UpdateSensor(0, temp);
        DataMgr_UpdateSensor(1, hum);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second
    }
}

void vTaskAlarm(void *pvParameters) {
    while(1) {
        AlarmMgr_CheckAlarms();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vTaskComm(void *pvParameters) {
    while(1) {
        CommMgr_Update();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```
**Deliverable**: Complete application running with all modules

---

## 📊 Code Implementation Timeline

```
Week 1:  Codes 1-4    (Types, Config, Data Manager, Alarm Manager)
Week 2:  Codes 5-11   (UI Core, Screen Manager, Touch, Widgets)
Week 3:  Codes 12-16  (Application Screens)
Week 4:  Codes 17-19  (Communication & Logging)
Week 5:  Code 20      (Main Application Integration)
```

---

## 🔗 Dependency Graph

```
Code 1-2 (Types & Config)
    ↓
Code 3-4 (Data & Alarm Manager)
    ↓
Code 5-7 (UI Core, Screen Manager, Touch)
    ↓
Code 8-11 (Widget Library)
    ↓
Code 12-16 (Application Screens)
    ↓
Code 17-19 (Communication & Logging)
    ↓
Code 20 (Main Application)
```

---

## ✅ Milestones & Testing

| Code # | Milestone | Test Method |
|--------|-----------|-------------|
| 1-2 | Definitions Ready | Compiles without errors |
| 3-4 | Data System Ready | Can store/retrieve sensor values |
| 5-7 | UI Framework Ready | Can display text/graphics, touch works |
| 8-11 | Widgets Ready | Each widget renders correctly |
| 12-16 | Screens Ready | Can navigate between all screens |
| 17-19 | Communication | Can send/receive data |
| 20 | Full System | All features work together |

---

## 📁 Directory Structure to Create

```
Projects/IndustrialHMI/
├── src/
│   ├── app/
│   │   ├── screens/
│   │   │   ├── screen_main.c/h
│   │   │   ├── screen_monitor.c/h
│   │   │   ├── screen_alarm.c/h
│   │   │   ├── screen_trend.c/h
│   │   │   ├── screen_config.c/h
│   │   │   └── screen_about.c/h
│   │   └── app_main.c
│   ├── ui/
│   │   ├── core/
│   │   │   ├── ui_core.c/h
│   │   │   ├── ui_screen.c/h
│   │   │   ├── ui_touch.c/h
│   │   │   └── ui_render.c/h
│   │   └── widgets/
│   │       ├── widget_button.c/h
│   │       ├── widget_label.c/h
│   │       ├── widget_gauge.c/h
│   │       ├── widget_chart.c/h
│   │       ├── widget_led.c/h
│   │       ├── widget_slider.c/h
│   │       └── widget_numpad.c/h
│   ├── middleware/
│   │   ├── data/
│   │   │   ├── data_manager.c/h
│   │   │   └── data_types.h
│   │   ├── alarm/
│   │   │   ├── alarm_manager.c/h
│   │   │   └── alarm_history.c/h
│   │   ├── logger/
│   │   │   ├── data_logger.c/h
│   │   │   └── file_manager.c/h
│   │   └── comm/
│   │       ├── modbus_rtu.c/h
│   │       ├── can_protocol.c/h
│   │       └── comm_manager.c/h
│   └── config/
│       ├── types.h
│       ├── hmi_config.h
│       └── pin_config.h
└── inc/
    └── (mirror of src/ headers)
```

---

## 🚀 How to Execute This Roadmap

1. **Start with Code 1** - Create `types.h` and `hmi_config.h`
2. **Build Codes 3-4** - Implement data and alarm managers
3. **Implement Codes 5-7** - UI framework foundation
4. **Add Codes 8-11** - Widget library
5. **Create Codes 12-16** - Application screens
6. **Integrate Codes 17-19** - Communication and logging
7. **Finalize Code 20** - Main application with FreeRTOS tasks

**Do NOT skip ahead** - Each code depends on previous ones

---

**Next Step**: Start with Code 1 - Create `Projects/IndustrialHMI/src/config/types.h`
