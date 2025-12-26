# Phase 5: Application Features

## 5.1 Screen Designs

### Main Menu Screen

```
┌─────────────────────────────────────────┐
│ ■ HMI Panel          12:34  🔔  ⚙️     │ Header
├─────────────────────────────────────────┤
│                                         │
│   ┌─────────┐  ┌─────────┐             │
│   │ 📊      │  │ ⚠️      │             │
│   │ Monitor │  │ Alarms  │             │
│   └─────────┘  └─────────┘             │
│                                         │
│   ┌─────────┐  ┌─────────┐             │
│   │ 📈      │  │ ⚙️      │             │
│   │ Trends  │  │ Config  │             │
│   └─────────┘  └─────────┘             │
│                                         │
│   ┌─────────┐  ┌─────────┐             │
│   │ 🔧      │  │ ℹ️      │             │
│   │ Diagnos │  │ About   │             │
│   └─────────┘  └─────────┘             │
│                                         │
├─────────────────────────────────────────┤
│ Status: Connected ● | Errors: 0        │ Footer
└─────────────────────────────────────────┘
```

### Monitor Screen

```
┌─────────────────────────────────────────┐
│ ◄ Monitor            12:34  🔔  ⚙️     │
├─────────────────────────────────────────┤
│                                         │
│   Temperature      Pressure             │
│   ┌─────────┐    ┌─────────┐           │
│   │   ╭───╮ │    │   ╭───╮ │           │
│   │  ╱  │  ╲│    │  ╱  │  ╲│           │
│   │ ╱   │   │    │ ╱   │   │           │
│   │╱    ▼   │    │╱    ▼   │           │
│   │  45.2°C │    │  3.2bar │           │
│   └─────────┘    └─────────┘           │
│                                         │
│   Status          Output                │
│   ┌──┐ Running    ┌─────────────────┐  │
│   │●│            │████████░░░  68% │  │
│   └──┘ Ready      └─────────────────┘  │
│                                         │
│   ┌────────┐  ┌────────┐  ┌────────┐   │
│   │ START  │  │  STOP  │  │ RESET  │   │
│   └────────┘  └────────┘  └────────┘   │
│                                         │
├─────────────────────────────────────────┤
│ Page 1/3     ●○○                       │
└─────────────────────────────────────────┘
```

### Trend Screen

```
┌─────────────────────────────────────────┐
│ ◄ Trends             12:34  🔔  ⚙️     │
├─────────────────────────────────────────┤
│ 100 ┤                                   │
│     │     ╱╲    ╱╲                     │
│  75 ┤    ╱  ╲  ╱  ╲                    │
│     │   ╱    ╲╱    ╲       ╱          │
│  50 ┤──╱──────────────╲───╱────────── │
│     │ ╱                 ╲╱             │
│  25 ┤╱                                 │
│     │                                   │
│   0 ┼───┬───┬───┬───┬───┬───┬───┬───  │
│     -1h      -30m      Now             │
├─────────────────────────────────────────┤
│ ● Temperature  ● Setpoint  ● Output    │
├─────────────────────────────────────────┤
│ [1 Hour] [8 Hour] [24 Hour] [7 Days]   │
└─────────────────────────────────────────┘
```

### Alarm List Screen

```
┌─────────────────────────────────────────┐
│ ◄ Alarms (3 Active)  12:34  🔔  ⚙️     │
├─────────────────────────────────────────┤
│ ┌───────────────────────────────────┐   │
│ │ 🔴 HIGH TEMPERATURE               │   │
│ │    Value: 85.2°C  Limit: 80.0°C  │   │
│ │    12:30:45                  [ACK]│   │
│ └───────────────────────────────────┘   │
│ ┌───────────────────────────────────┐   │
│ │ 🟡 LOW PRESSURE WARNING           │   │
│ │    Value: 1.8bar  Limit: 2.0bar  │   │
│ │    12:28:12                  [ACK]│   │
│ └───────────────────────────────────┘   │
│ ┌───────────────────────────────────┐   │
│ │ 🔴 COMMUNICATION FAILURE          │   │
│ │    Device: PLC-01                │   │
│ │    12:25:00                  [ACK]│   │
│ └───────────────────────────────────┘   │
├─────────────────────────────────────────┤
│ [ACK ALL]  [HISTORY]  [SILENCE]        │
└─────────────────────────────────────────┘
```

### Configuration Screen

```
┌─────────────────────────────────────────┐
│ ◄ Configuration      12:34  🔔  ⚙️     │
├─────────────────────────────────────────┤
│                                         │
│   ┌─────────────────────────────────┐   │
│   │ 🔌 Communication                │ > │
│   └─────────────────────────────────┘   │
│   ┌─────────────────────────────────┐   │
│   │ ⚠️ Alarm Settings               │ > │
│   └─────────────────────────────────┘   │
│   ┌─────────────────────────────────┐   │
│   │ 🖥️ Display Settings            │ > │
│   └─────────────────────────────────┘   │
│   ┌─────────────────────────────────┐   │
│   │ 📊 Data Logging                 │ > │
│   └─────────────────────────────────┘   │
│   ┌─────────────────────────────────┐   │
│   │ 🔐 Security                     │ > │
│   └─────────────────────────────────┘   │
│   ┌─────────────────────────────────┐   │
│   │ 🔄 System                       │ > │
│   └─────────────────────────────────┘   │
│                                         │
└─────────────────────────────────────────┘
```

---

## 5.2 Alarm System

### Alarm Manager

```c
// alarm_manager.h
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define ALARM_MAX_COUNT         64
#define ALARM_HISTORY_SIZE      256
#define ALARM_MESSAGE_LEN       48

// Alarm priority levels
typedef enum {
    ALARM_PRIORITY_INFO,        // Informational
    ALARM_PRIORITY_WARNING,     // Warning (yellow)
    ALARM_PRIORITY_ALARM,       // Alarm (red)
    ALARM_PRIORITY_CRITICAL     // Critical (flashing red)
} AlarmPriority_t;

// Alarm states
typedef enum {
    ALARM_STATE_NORMAL,         // Not active
    ALARM_STATE_ACTIVE,         // Active, not acknowledged
    ALARM_STATE_ACKNOWLEDGED,   // Active and acknowledged
    ALARM_STATE_RETURNED        // Returned to normal (cleared)
} AlarmState_t;

// Alarm condition types
typedef enum {
    ALARM_COND_HIGH,            // Value > limit
    ALARM_COND_LOW,             // Value < limit
    ALARM_COND_HIGH_HIGH,       // Value > critical limit
    ALARM_COND_LOW_LOW,         // Value < critical limit
    ALARM_COND_DEVIATION,       // |Value - setpoint| > limit
    ALARM_COND_RATE_OF_CHANGE,  // dV/dt > limit
    ALARM_COND_DIGITAL_ON,      // Digital input = ON
    ALARM_COND_DIGITAL_OFF,     // Digital input = OFF
    ALARM_COND_COMMUNICATION    // Communication failure
} AlarmCondition_t;

// Alarm definition
typedef struct {
    uint16_t id;                    // Unique alarm ID
    char name[32];                  // Alarm name
    char message[ALARM_MESSAGE_LEN];// Alarm message
    AlarmPriority_t priority;
    AlarmCondition_t condition;
    uint16_t tagId;                 // Associated data tag
    float limit;                    // Alarm limit value
    float deadband;                 // Hysteresis
    uint16_t delay;                 // On-delay (ms)
    bool enabled;
    bool requiresAck;
    
    // Runtime state
    AlarmState_t state;
    uint32_t activateTime;
    uint32_t acknowledgeTime;
    uint32_t returnTime;
    float triggerValue;
    
} AlarmDef_t;

// Alarm history entry
typedef struct {
    uint16_t alarmId;
    AlarmState_t state;
    uint32_t timestamp;
    float value;
} AlarmHistoryEntry_t;

// Alarm Manager API
void AlarmManager_Init(void);
AlarmDef_t* AlarmManager_CreateAlarm(uint16_t id, const char* name);
AlarmDef_t* AlarmManager_GetAlarm(uint16_t id);
void AlarmManager_DeleteAlarm(uint16_t id);

// Configuration
void AlarmManager_SetCondition(uint16_t id, AlarmCondition_t cond, float limit);
void AlarmManager_SetPriority(uint16_t id, AlarmPriority_t priority);
void AlarmManager_SetDelay(uint16_t id, uint16_t delayMs);
void AlarmManager_SetDeadband(uint16_t id, float deadband);
void AlarmManager_Enable(uint16_t id, bool enable);

// Operations
void AlarmManager_Acknowledge(uint16_t id);
void AlarmManager_AcknowledgeAll(void);
void AlarmManager_Silence(void);
void AlarmManager_Unsilence(void);
bool AlarmManager_IsSilenced(void);

// Status queries
uint16_t AlarmManager_GetActiveCount(void);
uint16_t AlarmManager_GetUnackedCount(void);
AlarmDef_t** AlarmManager_GetActiveAlarms(uint16_t* count);
AlarmPriority_t AlarmManager_GetHighestPriority(void);

// History
AlarmHistoryEntry_t* AlarmManager_GetHistory(uint16_t* count);
void AlarmManager_ClearHistory(void);

// Processing (call from RTOS task)
void AlarmManager_Process(void);

// Callbacks
typedef void (*AlarmCallback_t)(AlarmDef_t* alarm, AlarmState_t oldState);
void AlarmManager_RegisterCallback(AlarmCallback_t callback);

#endif // ALARM_MANAGER_H
```

### Alarm Configuration Example

```c
void ConfigureAlarms(void)
{
    AlarmDef_t* alarm;
    
    // High temperature alarm
    alarm = AlarmManager_CreateAlarm(ALARM_TEMP_HIGH, "High Temperature");
    alarm->message = "Temperature exceeded high limit";
    alarm->priority = ALARM_PRIORITY_ALARM;
    alarm->condition = ALARM_COND_HIGH;
    alarm->tagId = TAG_TEMPERATURE;
    alarm->limit = 80.0f;
    alarm->deadband = 2.0f;
    alarm->delay = 5000;        // 5 second delay
    alarm->requiresAck = true;
    alarm->enabled = true;
    
    // Critical high temperature
    alarm = AlarmManager_CreateAlarm(ALARM_TEMP_HI_HI, "Critical Temperature");
    alarm->message = "CRITICAL: Temperature very high!";
    alarm->priority = ALARM_PRIORITY_CRITICAL;
    alarm->condition = ALARM_COND_HIGH_HIGH;
    alarm->tagId = TAG_TEMPERATURE;
    alarm->limit = 90.0f;
    alarm->deadband = 1.0f;
    alarm->delay = 0;           // Immediate
    alarm->requiresAck = true;
    alarm->enabled = true;
    
    // Low pressure warning
    alarm = AlarmManager_CreateAlarm(ALARM_PRESS_LOW, "Low Pressure Warning");
    alarm->message = "Pressure below normal";
    alarm->priority = ALARM_PRIORITY_WARNING;
    alarm->condition = ALARM_COND_LOW;
    alarm->tagId = TAG_PRESSURE;
    alarm->limit = 2.0f;
    alarm->deadband = 0.2f;
    alarm->delay = 3000;
    alarm->requiresAck = false;
    alarm->enabled = true;
    
    // Communication failure
    alarm = AlarmManager_CreateAlarm(ALARM_COMM_FAIL, "Communication Failure");
    alarm->message = "Lost communication with PLC";
    alarm->priority = ALARM_PRIORITY_ALARM;
    alarm->condition = ALARM_COND_COMMUNICATION;
    alarm->tagId = TAG_COMM_STATUS;
    alarm->delay = 10000;       // 10 second timeout
    alarm->requiresAck = true;
    alarm->enabled = true;
}
```

---

## 5.3 Data Logging

### Data Logger

```c
// data_logger.h
#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "fatfs.h"

#define LOG_MAX_TAGS        32
#define LOG_FILENAME_LEN    32
#define LOG_BUFFER_SIZE     4096

// Log file format
typedef enum {
    LOG_FORMAT_CSV,         // Comma-separated values
    LOG_FORMAT_BINARY       // Binary format (smaller)
} LogFormat_t;

// Logger configuration
typedef struct {
    bool enabled;
    LogFormat_t format;
    uint32_t intervalMs;        // Logging interval
    uint16_t maxFileSizeMB;     // Max file size before rotation
    uint16_t maxFiles;          // Max number of files to keep
    char filePrefix[16];        // File name prefix
    uint16_t tagIds[LOG_MAX_TAGS];  // Tags to log
    uint8_t tagCount;
} LoggerConfig_t;

// Logger status
typedef struct {
    bool logging;
    bool sdCardPresent;
    bool sdCardMounted;
    uint32_t recordCount;
    uint32_t fileSize;
    uint32_t diskFreeKB;
    char currentFile[LOG_FILENAME_LEN];
    uint32_t lastLogTime;
    uint32_t errorCount;
} LoggerStatus_t;

// Data Logger API
void DataLogger_Init(void);
void DataLogger_Configure(const LoggerConfig_t* config);
void DataLogger_Start(void);
void DataLogger_Stop(void);
void DataLogger_Pause(void);
void DataLogger_Resume(void);

// Tag management
void DataLogger_AddTag(uint16_t tagId);
void DataLogger_RemoveTag(uint16_t tagId);
void DataLogger_ClearTags(void);

// Manual logging
void DataLogger_LogNow(void);
void DataLogger_LogEvent(const char* event);

// Status
LoggerStatus_t* DataLogger_GetStatus(void);
bool DataLogger_IsLogging(void);

// File operations
uint16_t DataLogger_GetFileList(char filenames[][LOG_FILENAME_LEN], uint16_t maxFiles);
bool DataLogger_DeleteFile(const char* filename);
bool DataLogger_DeleteAllFiles(void);
bool DataLogger_ExportToUSB(const char* filename);

// Processing (call from RTOS task)
void DataLogger_Process(void);

#endif // DATA_LOGGER_H
```

### CSV Output Example

```csv
Timestamp,Temperature,Pressure,Setpoint,Output,Running
2025-12-11 10:00:00,45.2,3.2,50.0,68.5,1
2025-12-11 10:00:01,45.4,3.2,50.0,69.0,1
2025-12-11 10:00:02,45.6,3.1,50.0,69.5,1
2025-12-11 10:00:03,45.8,3.1,50.0,70.0,1
```

---

## 5.4 User Authentication

```c
// auth_manager.h
#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// User levels
typedef enum {
    USER_LEVEL_VIEWER,      // View only
    USER_LEVEL_OPERATOR,    // View + operate
    USER_LEVEL_ENGINEER,    // View + operate + configure
    USER_LEVEL_ADMIN        // Full access
} UserLevel_t;

// User definition
typedef struct {
    uint8_t id;
    char username[16];
    uint16_t pin;           // 4-digit PIN
    UserLevel_t level;
    bool active;
} User_t;

// Session info
typedef struct {
    bool loggedIn;
    User_t* currentUser;
    uint32_t loginTime;
    uint32_t lastActivity;
} Session_t;

// Authentication API
void Auth_Init(void);
bool Auth_Login(const char* username, uint16_t pin);
bool Auth_LoginByPin(uint16_t pin);
void Auth_Logout(void);
bool Auth_IsLoggedIn(void);
UserLevel_t Auth_GetCurrentLevel(void);
const char* Auth_GetCurrentUsername(void);

// Authorization checks
bool Auth_CanOperate(void);
bool Auth_CanConfigure(void);
bool Auth_CanAdmin(void);
bool Auth_CheckLevel(UserLevel_t requiredLevel);

// User management (Admin only)
bool Auth_CreateUser(const char* username, uint16_t pin, UserLevel_t level);
bool Auth_DeleteUser(uint8_t userId);
bool Auth_ChangePin(uint8_t userId, uint16_t newPin);
bool Auth_SetLevel(uint8_t userId, UserLevel_t level);
User_t** Auth_GetUserList(uint8_t* count);

// Session management
void Auth_ResetTimeout(void);
uint32_t Auth_GetIdleTime(void);
void Auth_SetAutoLogoutTime(uint32_t seconds);

#endif // AUTH_MANAGER_H
```

---

## 5.5 Recipe Management (Premium Feature)

```c
// recipe_manager.h
#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define RECIPE_MAX_COUNT        32
#define RECIPE_MAX_PARAMS       64
#define RECIPE_NAME_LEN         32

// Recipe parameter
typedef struct {
    uint16_t tagId;         // Data tag to set
    float value;            // Value to set
} RecipeParam_t;

// Recipe definition
typedef struct {
    uint8_t id;
    char name[RECIPE_NAME_LEN];
    char description[64];
    RecipeParam_t params[RECIPE_MAX_PARAMS];
    uint8_t paramCount;
    bool locked;            // Prevent modification
} Recipe_t;

// Recipe Manager API
void RecipeManager_Init(void);
Recipe_t* RecipeManager_Create(const char* name);
Recipe_t* RecipeManager_Get(uint8_t id);
Recipe_t* RecipeManager_GetByName(const char* name);
void RecipeManager_Delete(uint8_t id);

// Parameter management
void RecipeManager_AddParam(uint8_t recipeId, uint16_t tagId, float value);
void RecipeManager_UpdateParam(uint8_t recipeId, uint16_t tagId, float value);
void RecipeManager_RemoveParam(uint8_t recipeId, uint16_t tagId);

// Operations
bool RecipeManager_Load(uint8_t recipeId);       // Apply recipe values
bool RecipeManager_Save(uint8_t recipeId);       // Save current values to recipe
bool RecipeManager_Compare(uint8_t recipeId);    // Compare with current values

// File operations
bool RecipeManager_Export(uint8_t recipeId, const char* filename);
bool RecipeManager_Import(const char* filename);
bool RecipeManager_SaveAll(void);
bool RecipeManager_LoadAll(void);

#endif // RECIPE_MANAGER_H
```

---

## 5.6 Implementation Steps

### Step 1: Main Menu Screen (Day 21)
```
[ ] Create main menu layout
[ ] Implement navigation buttons
[ ] Add status bar (header/footer)
[ ] Test screen transitions
```

### Step 2: Monitor Screen (Day 22-23)
```
[ ] Create gauge widgets for PV display
[ ] Create status indicators
[ ] Add control buttons
[ ] Wire to data manager
[ ] Test with simulated data
```

### Step 3: Alarm System (Day 24-25)
```
[ ] Implement alarm manager
[ ] Create alarm list screen
[ ] Create alarm history screen
[ ] Add audible alerts
[ ] Test alarm sequences
```

### Step 4: Trend Screen (Day 26)
```
[ ] Implement trend buffer
[ ] Create chart widget
[ ] Add time scale selection
[ ] Add data export
```

### Step 5: Data Logging (Day 27)
```
[ ] Implement SD card file system
[ ] Create logging task
[ ] Add log configuration screen
[ ] Test with long-duration logging
```

### Step 6: Configuration Screens (Day 28)
```
[ ] Communication settings
[ ] Alarm settings
[ ] Display settings
[ ] Security settings
```

---

## 5.7 Next Steps

1. ✅ Features designed
2. ➡️ Proceed to `06_TESTING.md` for testing strategy
3. Implement screens incrementally
4. Test each feature thoroughly
5. Get user feedback

---

## Checklist

- [ ] Main menu working
- [ ] Monitor screen functional
- [ ] Alarms operational
- [ ] Trends displaying
- [ ] Data logging working
- [ ] Configuration screens done
- [ ] Authentication working
