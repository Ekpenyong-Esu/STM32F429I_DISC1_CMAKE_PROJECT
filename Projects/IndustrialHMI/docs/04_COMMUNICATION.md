# Phase 4: Communication Protocols

## 4.1 Modbus RTU Implementation

### Overview
Modbus RTU is the most common industrial protocol. We'll implement both Master and Slave modes.

### Hardware Setup

```
STM32F429 ──────┬──── MAX485 ──────┬──── Industrial Device
                │                   │
    PA2 (TX) ───┼──── DI            │
    PA3 (RX) ───┼──── RO         A ─┼──── A (+)
    PB0 (DE) ───┼──── DE/RE      B ─┼──── B (-)
    GND ────────┼──── GND      GND ─┴──── GND
```

### Modbus Data Structures

```c
// modbus_rtu.h
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stdint.h>
#include <stdbool.h>

// Modbus function codes
typedef enum {
    MODBUS_FC_READ_COILS             = 0x01,
    MODBUS_FC_READ_DISCRETE_INPUTS   = 0x02,
    MODBUS_FC_READ_HOLDING_REGISTERS = 0x03,
    MODBUS_FC_READ_INPUT_REGISTERS   = 0x04,
    MODBUS_FC_WRITE_SINGLE_COIL      = 0x05,
    MODBUS_FC_WRITE_SINGLE_REGISTER  = 0x06,
    MODBUS_FC_WRITE_MULTIPLE_COILS   = 0x0F,
    MODBUS_FC_WRITE_MULTIPLE_REGS    = 0x10
} ModbusFunctionCode_t;

// Modbus exception codes
typedef enum {
    MODBUS_EX_NONE                   = 0x00,
    MODBUS_EX_ILLEGAL_FUNCTION       = 0x01,
    MODBUS_EX_ILLEGAL_DATA_ADDRESS   = 0x02,
    MODBUS_EX_ILLEGAL_DATA_VALUE     = 0x03,
    MODBUS_EX_SLAVE_DEVICE_FAILURE   = 0x04,
    MODBUS_EX_ACKNOWLEDGE            = 0x05,
    MODBUS_EX_SLAVE_BUSY             = 0x06,
    MODBUS_EX_MEMORY_PARITY_ERROR    = 0x08,
    MODBUS_EX_GATEWAY_PATH_FAILED    = 0x0A,
    MODBUS_EX_GATEWAY_TARGET_FAILED  = 0x0B
} ModbusException_t;

// Modbus status
typedef enum {
    MODBUS_OK = 0,
    MODBUS_ERROR_CRC,
    MODBUS_ERROR_TIMEOUT,
    MODBUS_ERROR_FRAME,
    MODBUS_ERROR_EXCEPTION,
    MODBUS_ERROR_BUSY
} ModbusStatus_t;

// Configuration
typedef struct {
    uint8_t slaveAddress;       // 1-247 (0 = broadcast)
    uint32_t baudRate;          // 9600, 19200, 38400, 57600, 115200
    uint8_t parity;             // 0=None, 1=Even, 2=Odd
    uint8_t stopBits;           // 1 or 2
    uint16_t timeout;           // Response timeout (ms)
    uint16_t interFrameDelay;   // Inter-frame delay (ms)
} ModbusConfig_t;

// Request/Response frame
typedef struct {
    uint8_t slaveAddress;
    uint8_t functionCode;
    uint16_t startAddress;
    uint16_t quantity;
    uint8_t* data;
    uint16_t dataLength;
    ModbusException_t exception;
} ModbusFrame_t;

// Callback types
typedef void (*ModbusCallback_t)(ModbusFrame_t* frame, ModbusStatus_t status);

#endif // MODBUS_RTU_H
```

### Modbus Master API

```c
// modbus_master.h
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include "modbus_rtu.h"

// Initialize Modbus Master
ModbusStatus_t ModbusMaster_Init(const ModbusConfig_t* config);

// Synchronous requests (blocking)
ModbusStatus_t ModbusMaster_ReadCoils(uint8_t slave, uint16_t addr, uint16_t qty, uint8_t* data);
ModbusStatus_t ModbusMaster_ReadDiscreteInputs(uint8_t slave, uint16_t addr, uint16_t qty, uint8_t* data);
ModbusStatus_t ModbusMaster_ReadHoldingRegisters(uint8_t slave, uint16_t addr, uint16_t qty, uint16_t* data);
ModbusStatus_t ModbusMaster_ReadInputRegisters(uint8_t slave, uint16_t addr, uint16_t qty, uint16_t* data);
ModbusStatus_t ModbusMaster_WriteSingleCoil(uint8_t slave, uint16_t addr, bool value);
ModbusStatus_t ModbusMaster_WriteSingleRegister(uint8_t slave, uint16_t addr, uint16_t value);
ModbusStatus_t ModbusMaster_WriteMultipleCoils(uint8_t slave, uint16_t addr, uint16_t qty, const uint8_t* data);
ModbusStatus_t ModbusMaster_WriteMultipleRegisters(uint8_t slave, uint16_t addr, uint16_t qty, const uint16_t* data);

// Asynchronous requests (non-blocking)
ModbusStatus_t ModbusMaster_ReadHoldingRegistersAsync(uint8_t slave, uint16_t addr, uint16_t qty, ModbusCallback_t callback);
ModbusStatus_t ModbusMaster_WriteMultipleRegistersAsync(uint8_t slave, uint16_t addr, uint16_t qty, const uint16_t* data, ModbusCallback_t callback);

// Status
bool ModbusMaster_IsBusy(void);
void ModbusMaster_Poll(void);
uint32_t ModbusMaster_GetErrorCount(void);
void ModbusMaster_ResetErrorCount(void);

#endif // MODBUS_MASTER_H
```

### Modbus Slave API

```c
// modbus_slave.h
#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include "modbus_rtu.h"

// Register memory map
#define MODBUS_COIL_COUNT           256
#define MODBUS_DISCRETE_COUNT       256
#define MODBUS_HOLDING_REG_COUNT    256
#define MODBUS_INPUT_REG_COUNT      256

// Callbacks for register access
typedef bool (*ModbusReadCoilCallback)(uint16_t address, bool* value);
typedef bool (*ModbusWriteCoilCallback)(uint16_t address, bool value);
typedef bool (*ModbusReadRegisterCallback)(uint16_t address, uint16_t* value);
typedef bool (*ModbusWriteRegisterCallback)(uint16_t address, uint16_t value);

// Initialize Modbus Slave
ModbusStatus_t ModbusSlave_Init(const ModbusConfig_t* config);

// Set callbacks (optional - use internal memory if NULL)
void ModbusSlave_SetCoilCallbacks(ModbusReadCoilCallback read, ModbusWriteCoilCallback write);
void ModbusSlave_SetDiscreteCallback(ModbusReadCoilCallback read);
void ModbusSlave_SetHoldingRegCallbacks(ModbusReadRegisterCallback read, ModbusWriteRegisterCallback write);
void ModbusSlave_SetInputRegCallback(ModbusReadRegisterCallback read);

// Direct memory access (when not using callbacks)
void ModbusSlave_SetCoil(uint16_t address, bool value);
bool ModbusSlave_GetCoil(uint16_t address);
void ModbusSlave_SetHoldingRegister(uint16_t address, uint16_t value);
uint16_t ModbusSlave_GetHoldingRegister(uint16_t address);
void ModbusSlave_SetInputRegister(uint16_t address, uint16_t value);

// Process incoming requests (call from main loop or RTOS task)
void ModbusSlave_Poll(void);

#endif // MODBUS_SLAVE_H
```

---

## 4.2 CAN Bus Implementation

### Hardware Setup

```
STM32F429 ────────┬──── CAN Transceiver ────┬──── CAN Bus
                  │     (SN65HVD230)         │
    PD0 (RX) ─────┼──── CANRX               │
    PD1 (TX) ─────┼──── CANTX            H ─┼──── CAN_H
    3.3V ─────────┼──── VCC              L ─┼──── CAN_L
    GND ──────────┼──── GND            GND ─┴──── GND
```

### CAN API

```c
// can_hmi.h - Higher level CAN for HMI
#ifndef CAN_HMI_H
#define CAN_HMI_H

#include "can.h"

// CAN message types for HMI
typedef enum {
    CAN_MSG_DATA_REQUEST,       // Request data from device
    CAN_MSG_DATA_RESPONSE,      // Response with data
    CAN_MSG_COMMAND,            // Send command to device
    CAN_MSG_STATUS,             // Device status update
    CAN_MSG_ALARM,              // Alarm notification
    CAN_MSG_HEARTBEAT           // Keep-alive
} CAN_HMI_MsgType_t;

// CAN data packet
typedef struct {
    uint16_t deviceId;
    CAN_HMI_MsgType_t type;
    uint16_t parameterId;
    union {
        int32_t intValue;
        float floatValue;
        uint8_t bytes[4];
    } data;
} CAN_HMI_Packet_t;

// Initialize CAN for HMI
void CAN_HMI_Init(uint32_t bitrate);

// Send packets
bool CAN_HMI_SendCommand(uint16_t deviceId, uint16_t cmdId, int32_t value);
bool CAN_HMI_RequestData(uint16_t deviceId, uint16_t parameterId);

// Receive handling
void CAN_HMI_RegisterCallback(void (*callback)(CAN_HMI_Packet_t* packet));
void CAN_HMI_Poll(void);

#endif // CAN_HMI_H
```

---

## 4.3 Data Manager

The data manager bridges communication protocols with the UI.

```c
// data_manager.h
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// Maximum number of data tags
#define DATA_TAG_MAX 256

// Data types
typedef enum {
    DATA_TYPE_BOOL,
    DATA_TYPE_INT8,
    DATA_TYPE_INT16,
    DATA_TYPE_INT32,
    DATA_TYPE_UINT8,
    DATA_TYPE_UINT16,
    DATA_TYPE_UINT32,
    DATA_TYPE_FLOAT,
    DATA_TYPE_STRING
} DataType_t;

// Data source
typedef enum {
    DATA_SOURCE_LOCAL,          // Internal variable
    DATA_SOURCE_MODBUS_COIL,    // Modbus coil
    DATA_SOURCE_MODBUS_DISC,    // Modbus discrete input
    DATA_SOURCE_MODBUS_HOLD,    // Modbus holding register
    DATA_SOURCE_MODBUS_INPUT,   // Modbus input register
    DATA_SOURCE_CAN,            // CAN bus
    DATA_SOURCE_ANALOG,         // ADC input
    DATA_SOURCE_DIGITAL         // Digital GPIO
} DataSource_t;

// Data quality
typedef enum {
    DATA_QUALITY_GOOD,
    DATA_QUALITY_UNCERTAIN,
    DATA_QUALITY_BAD,
    DATA_QUALITY_OFFLINE
} DataQuality_t;

// Data tag definition
typedef struct {
    uint16_t id;                    // Unique tag ID
    char name[32];                  // Tag name
    char description[64];           // Description
    DataType_t type;                // Data type
    DataSource_t source;            // Data source
    
    // Source configuration
    union {
        struct {
            uint8_t slaveAddress;
            uint16_t address;
        } modbus;
        struct {
            uint16_t deviceId;
            uint16_t parameterId;
        } can;
        struct {
            uint8_t channel;
        } analog;
        struct {
            GPIO_TypeDef* port;
            uint16_t pin;
        } digital;
    } sourceConfig;
    
    // Current value
    union {
        bool boolVal;
        int8_t int8Val;
        int16_t int16Val;
        int32_t int32Val;
        uint8_t uint8Val;
        uint16_t uint16Val;
        uint32_t uint32Val;
        float floatVal;
        char strVal[32];
    } value;
    
    // Metadata
    float scaleFactor;              // Raw * scale + offset = engineering
    float offset;
    float minValue;                 // Engineering units
    float maxValue;
    char unit[8];                   // Engineering unit
    uint8_t decimals;               // Display precision
    
    // Status
    DataQuality_t quality;
    uint32_t lastUpdateTime;
    uint32_t updateCount;
    
    // Change callback
    void (*onChange)(uint16_t tagId);
    
} DataTag_t;

// Data Manager API
void DataManager_Init(void);
DataTag_t* DataManager_CreateTag(uint16_t id, const char* name, DataType_t type);
DataTag_t* DataManager_GetTag(uint16_t id);
DataTag_t* DataManager_GetTagByName(const char* name);
void DataManager_DeleteTag(uint16_t id);

// Value access
bool DataManager_GetBool(uint16_t id);
int32_t DataManager_GetInt(uint16_t id);
float DataManager_GetFloat(uint16_t id);
const char* DataManager_GetString(uint16_t id);

void DataManager_SetBool(uint16_t id, bool value);
void DataManager_SetInt(uint16_t id, int32_t value);
void DataManager_SetFloat(uint16_t id, float value);
void DataManager_SetString(uint16_t id, const char* value);

// Engineering value (with scaling)
float DataManager_GetEngValue(uint16_t id);
void DataManager_SetEngValue(uint16_t id, float value);

// Status
DataQuality_t DataManager_GetQuality(uint16_t id);
uint32_t DataManager_GetAge(uint16_t id);

// Callbacks
void DataManager_RegisterCallback(uint16_t id, void (*callback)(uint16_t));

// Communication
void DataManager_Poll(void);  // Update all remote tags
void DataManager_SetPollRate(uint16_t id, uint16_t rateMs);

#endif // DATA_MANAGER_H
```

### Tag Configuration Example

```c
// Example: Configure tags for a temperature controller
void ConfigureDataTags(void)
{
    DataTag_t* tag;
    
    // Process Value (Temperature reading)
    tag = DataManager_CreateTag(TAG_TEMPERATURE, "Temperature", DATA_TYPE_FLOAT);
    tag->source = DATA_SOURCE_MODBUS_INPUT;
    tag->sourceConfig.modbus.slaveAddress = 1;
    tag->sourceConfig.modbus.address = 0;
    tag->scaleFactor = 0.1f;    // Raw value is in 0.1°C units
    tag->offset = 0.0f;
    tag->minValue = -50.0f;
    tag->maxValue = 150.0f;
    tag->unit = "°C";
    tag->decimals = 1;
    
    // Setpoint
    tag = DataManager_CreateTag(TAG_SETPOINT, "Setpoint", DATA_TYPE_FLOAT);
    tag->source = DATA_SOURCE_MODBUS_HOLD;
    tag->sourceConfig.modbus.slaveAddress = 1;
    tag->sourceConfig.modbus.address = 10;
    tag->scaleFactor = 0.1f;
    tag->minValue = 0.0f;
    tag->maxValue = 100.0f;
    tag->unit = "°C";
    tag->decimals = 1;
    
    // Output percentage
    tag = DataManager_CreateTag(TAG_OUTPUT, "Output", DATA_TYPE_FLOAT);
    tag->source = DATA_SOURCE_MODBUS_INPUT;
    tag->sourceConfig.modbus.slaveAddress = 1;
    tag->sourceConfig.modbus.address = 1;
    tag->scaleFactor = 0.1f;
    tag->minValue = 0.0f;
    tag->maxValue = 100.0f;
    tag->unit = "%";
    tag->decimals = 1;
    
    // Running status
    tag = DataManager_CreateTag(TAG_RUNNING, "Running", DATA_TYPE_BOOL);
    tag->source = DATA_SOURCE_MODBUS_COIL;
    tag->sourceConfig.modbus.slaveAddress = 1;
    tag->sourceConfig.modbus.address = 0;
    
    // Alarm status
    tag = DataManager_CreateTag(TAG_ALARM, "Alarm", DATA_TYPE_BOOL);
    tag->source = DATA_SOURCE_MODBUS_DISC;
    tag->sourceConfig.modbus.slaveAddress = 1;
    tag->sourceConfig.modbus.address = 0;
}
```

---

## 4.4 Communication Task

```c
// comm_task.c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "data_manager.h"
#include "modbus_master.h"

#define COMM_TASK_STACK_SIZE    512
#define COMM_TASK_PRIORITY      (configMAX_PRIORITIES - 1)
#define COMM_POLL_INTERVAL_MS   100

static TaskHandle_t commTaskHandle;

// Communication task
void Task_Communication(void* params)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (1)
    {
        // Poll all Modbus tags
        DataManager_Poll();
        
        // Process any pending Modbus responses
        ModbusMaster_Poll();
        
        // Check communication health
        // ... error handling ...
        
        // Wait for next poll cycle
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(COMM_POLL_INTERVAL_MS));
    }
}

void Comm_Init(void)
{
    // Initialize Modbus
    ModbusConfig_t modbusConfig = {
        .slaveAddress = 0,      // Master mode
        .baudRate = 9600,
        .parity = 0,            // None
        .stopBits = 1,
        .timeout = 500,
        .interFrameDelay = 50
    };
    ModbusMaster_Init(&modbusConfig);
    
    // Create communication task
    xTaskCreate(Task_Communication, "Comm", COMM_TASK_STACK_SIZE, 
                NULL, COMM_TASK_PRIORITY, &commTaskHandle);
}
```

---

## 4.5 Implementation Steps

### Step 1: RS485 Hardware (Day 15)
```
[ ] Connect MAX485 module to USART2
[ ] Configure GPIO for DE/RE control
[ ] Test basic TX/RX with loopback
[ ] Verify timing with oscilloscope
```

### Step 2: Modbus Frame Layer (Day 16)
```
[ ] Implement CRC-16 calculation
[ ] Implement frame building
[ ] Implement frame parsing
[ ] Test with known good frames
```

### Step 3: Modbus Master (Day 17-18)
```
[ ] Implement read functions (FC01-04)
[ ] Implement write functions (FC05-06, 0F-10)
[ ] Implement timeout handling
[ ] Test with Modbus simulator/slave device
```

### Step 4: Data Manager Integration (Day 19-20)
```
[ ] Create tag management system
[ ] Implement polling mechanism
[ ] Wire to UI widgets
[ ] End-to-end testing
```

---

## 4.6 Testing Tools

### Recommended Software
1. **Modbus Poll** (Windows) - Modbus master simulator
2. **Modbus Slave** (Windows) - Modbus slave simulator
3. **QModMaster** (Cross-platform) - Free Modbus master
4. **mbpoll** (Linux) - Command-line Modbus tool

### Test Procedure
```bash
# Using mbpoll on Linux
# Read 10 holding registers from slave 1, starting at address 0
mbpoll -m rtu -b 9600 -P none -a 1 -r 0 -c 10 /dev/ttyUSB0

# Write value 100 to register 10
mbpoll -m rtu -b 9600 -P none -a 1 -r 10 /dev/ttyUSB0 100
```

---

## 4.7 Next Steps

1. ✅ Communication protocols designed
2. ➡️ Proceed to `05_FEATURES.md` for application features
3. Order RS485 module
4. Set up Modbus test environment
5. Implement protocol stack

---

## Checklist

- [ ] RS485 hardware connected
- [ ] Modbus CRC working
- [ ] Modbus Master functional
- [ ] Data Manager integrated
- [ ] End-to-end data flow tested
