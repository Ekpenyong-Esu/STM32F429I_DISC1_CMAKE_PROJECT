# Getting Started Guide - STM32F429I Sensor Console Project

## 🎯 Project Overview

This project implements a complete sensor monitoring and control system on the STM32F429I-DISC1 board with support for temperature/humidity sensing, real-time clock, CAN bus communication, relay control, and visual/audio feedback.

---

## 📋 Prerequisites Checklist

### Hardware Required
- ✅ STM32F429I-DISC1 Discovery Board (you have)
- ✅ ST-LINK/V2 programmer (built into DISC1)
- ✅ Micro-USB cable for power and debugging
- ✅ Breadboard and jumper wires

### Software Required
- CMake (v3.20+)
- ARM GCC Toolchain (`arm-none-eabi-gcc`)
- STM32CubeProgrammer
- VS Code (recommended) or any IDE
- Git

---

## 🚀 Quick Start - Step-by-Step Implementation Order

This guide follows a **complexity-first approach**: start with simple peripherals and gradually add more complex sensors.

---

## Phase 1: Basic Setup & LED Control (30 minutes)

### Step 1.1: Verify Build System

```bash
cd /home/mahonri/Desktop/BareMetal/Sensor_Cons

# Clean and rebuild
mkdir -p build/Debug
cd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make clean
make -j4

# Expected output: Sensor_Console.elf built successfully
```

**Success Criteria:** Project builds without errors

---

### Step 1.2: Flash & Verify Basic LED Blink

**File to modify:** `Core/Src/main.c`

```c
// In main() function, add simple LED blink test
while (1)
{
  HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);  // Green LED
  HAL_Delay(500);  // 500ms delay
}
```

**Flash to board:**
```bash
# From project root
STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst -rst --start
```

**Success Criteria:** Green LED on board blinks every 500ms

**Troubleshooting:**
- LED not blinking? Check GPIO pin definition in `main.h`
- Flash failed? Verify ST-LINK connection: `STM32_Programmer_CLI --list`

---

## Phase 2: Serial Console & Debugging (1 hour)

### Step 2.1: Setup FT232RL USB-to-UART

**Hardware Connection:**
```
FT232RL Pin → STM32F429I Pin
────────────────────────────
VCC (3.3V)  → Not connected (STM32 powered via USB)
GND         → GND
TX          → PA10 (USART1_RX)
RX          → PA9  (USART1_TX)
```

**Important:** RX connects to TX, TX connects to RX (crossover)

---

### Step 2.2: Test UART Communication

**File:** `Peripherals/UART/uart_example.c` (use as reference)

Add to `main.c`:
```c
#include "uart.h"

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  
  // Initialize UART
  UART_Init();
  
  // Test message
  char msg[] = "STM32F429I Console Ready!\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000);
  
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
    HAL_UART_Transmit(&huart1, (uint8_t*)"Alive\r\n", 7, 1000);
    HAL_Delay(1000);
  }
}
```

**Test on PC:**
```bash
# Linux/Mac
screen /dev/ttyUSB0 115200

# Or use minicom
minicom -D /dev/ttyUSB0 -b 115200

# Windows
# Use PuTTY or TeraTerm: COM port, 115200 baud
```

**Success Criteria:** Console shows "STM32F429I Console Ready!" and "Alive" every second

---

## Phase 3: GPIO Control - Relay & Buzzer (1 hour)

### Step 3.1: Test Relay Control

**Hardware Connection:**
```
SRD-05V Relay Module → STM32F429I
─────────────────────────────────
VCC    → 5V (if available) or 3.3V
GND    → GND
IN     → PB0 (or any free GPIO)
```

**Code Example:**

```c
#include "relay.h"

// In main()
Relay_Config_t relay_config = {
    .GPIO_Port = GPIOB,
    .GPIO_Pin = GPIO_PIN_0,
    .Polarity = RELAY_ACTIVE_HIGH  // Change to ACTIVE_LOW if needed
};

Relay_Handle_t relay;
Relay_Init(&relay, &relay_config);

// Test relay on/off
while (1)
{
    Relay_On(&relay);
    printf("Relay ON\r\n");
    HAL_Delay(2000);
    
    Relay_Off(&relay);
    printf("Relay OFF\r\n");
    HAL_Delay(2000);
}
```

**Success Criteria:** Relay clicks on/off every 2 seconds

---

### Step 3.2: Test Buzzer (Active & Passive)

**Active Buzzer (Simple GPIO):**
```
Active Buzzer → STM32F429I
──────────────────────────
VCC → 5V or 3.3V
GND → GND
I/O → PC0 (any GPIO)
```

**Code:**
```c
#include "buzzer.h"

Buzzer_Config_t buzzer_config = {
    .Type = BUZZER_TYPE_ACTIVE,
    .GPIO_Port = GPIOC,
    .GPIO_Pin = GPIO_PIN_0
};

Buzzer_Handle_t buzzer;
Buzzer_Init(&buzzer, &buzzer_config);

// Beep pattern
Buzzer_On(&buzzer);
HAL_Delay(100);
Buzzer_Off(&buzzer);
```

**Passive Buzzer (PWM Tone):**
```c
// Requires timer configuration (e.g., TIM3_CH1 on PA6)
Buzzer_Config_t pwm_buzzer = {
    .Type = BUZZER_TYPE_PASSIVE,
    .Timer_Handle = &htim3,
    .Timer_Channel = TIM_CHANNEL_1
};

Buzzer_Init(&pwm_buzzer, &pwm_buzzer_config);
Buzzer_PlayTone(&pwm_buzzer, 1000, 50);  // 1kHz, 50% duty
HAL_Delay(500);
Buzzer_Stop(&pwm_buzzer);
```

**Success Criteria:** 
- Active buzzer beeps on GPIO high
- Passive buzzer plays different tones with PWM

---

## Phase 4: I2C Sensors - DHT11/DHT22 (2 hours)

### Step 4.1: Connect DHT Sensor

**DHT22 Connection (Recommended for accuracy):**
```
DHT22 Pin → STM32F429I
─────────────────────
VCC (1) → 3.3V
DATA (2) → PA1 (with 10kΩ pullup to 3.3V)
NC (3)  → Not connected
GND (4) → GND
```

**Important:** Add 10kΩ resistor between DATA and VCC

---

### Step 4.2: Read Temperature & Humidity

**Code Example:**

```c
#include "dht.h"
#include <stdio.h>

// In main()
DHT_Config_t dht_config = {
    .Type = DHT_TYPE_DHT22,  // Or DHT_TYPE_DHT11
    .GPIO_Port = GPIOA,
    .GPIO_Pin = GPIO_PIN_1
};

DHT_Handle_t dht_sensor;
DHT_Init(&dht_sensor, &dht_config);

while (1)
{
    float temperature, humidity;
    DHT_Status_t status = DHT_Read(&dht_sensor, &temperature, &humidity);
    
    if (status == DHT_OK)
    {
        printf("Temperature: %.1f°C\r\n", temperature);
        printf("Humidity: %.1f%%\r\n", humidity);
    }
    else
    {
        printf("DHT Read Error: %d\r\n", status);
    }
    
    HAL_Delay(2000);  // DHT22 minimum 2-second interval
}
```

**Success Criteria:** Console displays temperature and humidity readings every 2 seconds

**Troubleshooting:**
- Checksum errors? Add 10kΩ pullup resistor
- No response? Check wiring and GPIO pin assignment
- DHT11 vs DHT22: DHT11 has integer values, DHT22 has decimals

---

## Phase 5: Real-Time Clock - DS3231 (1.5 hours)

### Step 5.1: Connect DS3231 RTC Module

**I2C Connection:**
```
DS3231 Pin → STM32F429I (I2C1)
──────────────────────────────
VCC  → 3.3V or 5V
GND  → GND
SCL  → PB6 (I2C1_SCL)
SDA  → PB9 (I2C1_SDA)
SQW  → PA2 (optional, for 1Hz interrupt)
```

**Note:** No external pullup needed if DS3231 module has onboard pullups

---

### Step 5.2: Initialize I2C & RTC

**Code:**

```c
#include "i2c.h"
#include "rtc.h"

// Initialize I2C bus
I2C_Init();  // Configures I2C1 (PB6/PB9)

// Set initial time (do this once)
RTC_Time_t time = {
    .Hours = 14,
    .Minutes = 30,
    .Seconds = 0,
    .TimeFormat = RTC_HOURFORMAT_24
};

RTC_Date_t date = {
    .Year = 25,   // 2025
    .Month = RTC_MONTH_DECEMBER,
    .Date = 11,
    .WeekDay = RTC_WEEKDAY_WEDNESDAY
};

RTC_Init();
RTC_SetTime(&time);
RTC_SetDate(&date);

// Read time in loop
while (1)
{
    RTC_GetTime(&time);
    RTC_GetDate(&date);
    
    printf("%02d/%02d/%02d %02d:%02d:%02d\r\n",
           date.Date, date.Month, date.Year,
           time.Hours, time.Minutes, time.Seconds);
    
    HAL_Delay(1000);
}
```

**Success Criteria:** Console displays current date/time updating every second

---

## Phase 6: CAN Bus Communication (2 hours)

### Step 6.1: Connect Waveshare CAN Module

**CAN Transceiver Connection:**
```
Waveshare CAN → STM32F429I
──────────────────────────
VCC  → 3.3V or 5V (check module)
GND  → GND
TX   → PA12 (CAN1_TX)
RX   → PA11 (CAN1_RX)
CANH → CAN Bus High
CANL → CAN Bus Low
```

**Important:** CAN bus requires 120Ω termination resistors at each end

---

### Step 6.2: CAN Communication Test

**Code:**

```c
#include "can.h"

// Configure CAN at 500 kbps
CAN_Config config = {
    .mode = CAN_MODE_NORMAL,
    .baud_rate = CAN_BAUDRATE_500KBPS,
    .auto_retransmission = true,
    .auto_bus_off_recovery = true
};

CAN_Init(&config);

// Configure filter to accept all messages
CAN_Filter filter = {
    .filter_id = 0,
    .filter_mask = 0,
    .filter_mode = CAN_FILTER_MASK_MODE,
    .filter_scale = CAN_FILTER_32BIT,
    .filter_fifo = CAN_FILTER_FIFO0,
    .filter_bank = 0
};

CAN_ConfigFilter(&filter);

// Send CAN message
CAN_Frame tx_frame = {
    .id = 0x123,
    .id_type = CAN_ID_STANDARD,
    .frame_type = CAN_FRAME_DATA,
    .dlc = 8,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
};

CAN_Transmit(&tx_frame);

// Receive CAN message
CAN_Frame rx_frame;
while (1)
{
    if (CAN_Receive(&rx_frame, 100) == HAL_OK)
    {
        printf("CAN RX ID: 0x%03lX, Data: ", rx_frame.id);
        for (int i = 0; i < rx_frame.dlc; i++)
        {
            printf("%02X ", rx_frame.data[i]);
        }
        printf("\r\n");
    }
    HAL_Delay(10);
}
```

**Success Criteria:** CAN messages transmitted and received successfully

**Troubleshooting:**
- Bus-off errors? Check 120Ω termination resistors
- No communication? Verify CAN_H and CAN_L connections
- Use CAN analyzer tool for debugging

---

## Phase 7: Advanced Integration - LCD Display (2 hours)

### Step 7.1: LTDC & Touchscreen Setup

The STM32F429I-DISC1 has a built-in 2.4" LCD with touchscreen. Use existing drivers:

**Code:**

```c
#include "ltdc.h"
#include "touchscreen.h"

// Initialize LCD
LTDC_Init();
LCD_Clear(LCD_COLOR_BLACK);

// Display sensor readings
char buffer[64];
sprintf(buffer, "Temp: %.1f C", temperature);
LCD_DisplayStringAt(10, 50, (uint8_t*)buffer, LEFT_MODE);

sprintf(buffer, "Humidity: %.1f%%", humidity);
LCD_DisplayStringAt(10, 80, (uint8_t*)buffer, LEFT_MODE);

// Initialize touchscreen
Touchscreen_Init();

// Detect touch
TouchData_t touch;
if (Touchscreen_GetState(&touch) == TOUCH_DETECTED)
{
    printf("Touch at X=%d, Y=%d\r\n", touch.X, touch.Y);
}
```

**Success Criteria:** Sensor data displayed on LCD, touchscreen responds to input

---

## Phase 8: Data Logging to Flash (1.5 hours)

### Step 8.1: Use Internal Flash for Data Storage

**Available:** Sector 11 (128KB) at address 0x080E0000

**Code:**

```c
#include "flash.h"

#define DATA_LOG_SECTOR    FLASH_SECTOR_11
#define DATA_LOG_ADDRESS   0x080E0000

// Write sensor data to flash
typedef struct {
    uint32_t timestamp;
    float temperature;
    float humidity;
} SensorLog_t;

SensorLog_t log_entry = {
    .timestamp = HAL_GetTick(),
    .temperature = 25.5,
    .humidity = 60.3
};

// Erase sector before writing
FLASH_EraseSector(DATA_LOG_SECTOR);

// Write data
FLASH_Write(DATA_LOG_ADDRESS, (uint8_t*)&log_entry, sizeof(SensorLog_t));

// Read back data
SensorLog_t read_log;
FLASH_Read(DATA_LOG_ADDRESS, (uint8_t*)&read_log, sizeof(SensorLog_t));

printf("Logged: Temp=%.1f, Humidity=%.1f\r\n", 
       read_log.temperature, read_log.humidity);
```

**Success Criteria:** Data persists across power cycles

---

## Phase 9: FreeRTOS Multi-Tasking (2 hours)

### Step 9.1: Create Sensor Monitoring Tasks

**FreeRTOS is already configured!** See `Core/Inc/FreeRTOSConfig.h`

**Code:**

```c
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Global mutex for printf
SemaphoreHandle_t printMutex;

// Task 1: Read DHT sensor
void vTaskDHT(void *pvParameters)
{
    DHT_Handle_t *dht = (DHT_Handle_t*)pvParameters;
    
    while (1)
    {
        float temp, humidity;
        if (DHT_Read(dht, &temp, &humidity) == DHT_OK)
        {
            xSemaphoreTake(printMutex, portMAX_DELAY);
            printf("[DHT] Temp: %.1f°C, Humidity: %.1f%%\r\n", temp, humidity);
            xSemaphoreGive(printMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));  // 2 seconds
    }
}

// Task 2: Monitor CAN bus
void vTaskCAN(void *pvParameters)
{
    CAN_Frame rx_frame;
    
    while (1)
    {
        if (CAN_Receive(&rx_frame, 100) == HAL_OK)
        {
            xSemaphoreTake(printMutex, portMAX_DELAY);
            printf("[CAN] ID: 0x%03lX\r\n", rx_frame.id);
            xSemaphoreGive(printMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task 3: Update LCD display
void vTaskDisplay(void *pvParameters)
{
    while (1)
    {
        // Update LCD with latest sensor values
        LCD_Clear(LCD_COLOR_BLACK);
        LCD_DisplayStringAt(10, 50, (uint8_t*)"Sensor Console", CENTER_MODE);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // Initialize peripherals
    DHT_Init(&dht_sensor, &dht_config);
    CAN_Init(&can_config);
    LTDC_Init();
    
    // Create mutex
    printMutex = xSemaphoreCreateMutex();
    
    // Create tasks
    xTaskCreate(vTaskDHT, "DHT", 256, &dht_sensor, 2, NULL);
    xTaskCreate(vTaskCAN, "CAN", 256, NULL, 2, NULL);
    xTaskCreate(vTaskDisplay, "Display", 512, NULL, 1, NULL);
    
    // Start scheduler
    vTaskStartScheduler();
    
    while (1);  // Should never reach here
}
```

**Success Criteria:** All tasks run concurrently without blocking

---

## 📊 Complete Integration Roadmap

```
┌──────────────────────────────────────────────────────────────┐
│ PHASE 1: Basic Setup (30 min)                                │
│ ├─ Build system verification                                 │
│ ├─ LED blink test                                            │
│ └─ Flash programming                                         │
├──────────────────────────────────────────────────────────────┤
│ PHASE 2: Serial Console (1 hour)                             │
│ ├─ UART initialization                                       │
│ ├─ FT232RL connection                                        │
│ └─ Debug output via printf                                   │
├──────────────────────────────────────────────────────────────┤
│ PHASE 3: GPIO Control (1 hour)                               │
│ ├─ Relay on/off control                                      │
│ ├─ Active buzzer beep                                        │
│ └─ Passive buzzer tone generation                            │
├──────────────────────────────────────────────────────────────┤
│ PHASE 4: DHT Sensor (2 hours)                                │
│ ├─ DHT22 wiring with pullup                                  │
│ ├─ Temperature/humidity reading                              │
│ └─ Error handling & validation                               │
├──────────────────────────────────────────────────────────────┤
│ PHASE 5: RTC Module (1.5 hours)                              │
│ ├─ DS3231 I2C communication                                  │
│ ├─ Time/date setting                                         │
│ └─ Timestamp for data logging                                │
├──────────────────────────────────────────────────────────────┤
│ PHASE 6: CAN Bus (2 hours)                                   │
│ ├─ Waveshare CAN module setup                                │
│ ├─ TX/RX message handling                                    │
│ └─ Filter configuration                                      │
├──────────────────────────────────────────────────────────────┤
│ PHASE 7: LCD Display (2 hours)                               │
│ ├─ LTDC framebuffer setup                                    │
│ ├─ Display sensor data                                       │
│ └─ Touchscreen input                                         │
├──────────────────────────────────────────────────────────────┤
│ PHASE 8: Flash Logging (1.5 hours)                           │
│ ├─ Sector erase/write/read                                   │
│ ├─ Circular buffer for logs                                  │
│ └─ Persistent storage test                                   │
├──────────────────────────────────────────────────────────────┤
│ PHASE 9: FreeRTOS Tasks (2 hours)                            │
│ ├─ Multi-task sensor monitoring                              │
│ ├─ Task synchronization (mutex/semaphore)                    │
│ └─ Real-time display updates                                 │
└──────────────────────────────────────────────────────────────┘

TOTAL ESTIMATED TIME: 14 hours (spread over 2-3 days)
```

---

## 🔧 Common Issues & Solutions

### Build Errors

**Error:** `arm-none-eabi-gcc: command not found`
```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-none-eabi  # Ubuntu/Debian
brew install gcc-arm-embedded           # macOS
```

**Error:** `CMake cannot find compiler`
```bash
# Add to PATH or specify manually
export PATH=$PATH:/usr/bin/arm-none-eabi-gcc
```

---

### Flash Programming Errors

**Error:** `No ST-LINK detected`
```bash
# Check USB connection
lsusb | grep STM

# Install udev rules (Linux)
sudo cp 49-stlinkv2.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```

**Error:** `Target voltage too low`
- Check USB cable connection
- Verify board has power LED on
- Try different USB port

---

### Sensor Errors

**DHT Read Timeout:**
- Add 10kΩ pullup resistor
- Use shorter wires (<30cm)
- Check GPIO pin configuration

**I2C No ACK:**
- Verify SDA/SCL connections
- Check I2C address (DS3231: 0x68)
- Add 4.7kΩ pullup resistors if needed

**CAN Bus-Off:**
- Check 120Ω termination resistors
- Verify CAN_H and CAN_L not swapped
- Reduce baud rate to 125kbps for testing

---

## 📚 Next Steps

After completing all phases:

1. **Create HMI Dashboard:** Design touchscreen UI with buttons/graphs
2. **Add MPU6050:** Implement vibration monitoring
3. **Modbus Integration:** Add RS485 module for industrial protocols
4. **Alarm System:** Use buzzer + relay for threshold alerts
5. **Web Interface:** Add Ethernet for remote monitoring (optional)

---

## 📖 Additional Resources

- **Peripheral Drivers:** See `Peripherals/*/README.md` for detailed API docs
- **Hardware Inventory:** Check `HARDWARE_INVENTORY.md` for wiring diagrams
- **Shopping List:** See `SHOPPING_LIST.md` for missing components
- **STM32 Reference:** `docs/REFERENCEMANUAL-dis-1.pdf`
- **Datasheet:** `docs/DATASHEET.pdf`

---

## 🎓 Learning Path

**If you're new to embedded systems, follow this order:**

```
1. LED Blink       → Understand GPIO & HAL basics
2. UART Console    → Debug output & printf
3. Relay Control   → Digital output control
4. Buzzer          → PWM & timer basics
5. DHT Sensor      → Single-wire protocol
6. I2C RTC         → I2C communication
7. CAN Bus         → Industrial protocols
8. LCD Display     → Framebuffer graphics
9. FreeRTOS        → Multi-tasking & RTOS
```

**Estimated learning time:** 1-2 weeks for complete project

---

Good luck with your sensor console project! 🚀
