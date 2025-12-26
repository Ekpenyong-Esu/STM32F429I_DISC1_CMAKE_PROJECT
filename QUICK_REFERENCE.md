# Quick Reference - Implementation Checklist

## ⚡ Fast Track Guide (For Experienced Developers)

This is a condensed version of [GETTING_STARTED.md](GETTING_STARTED.md) showing only the essential steps.

---

## 🎯 Implementation Order (Copy-Paste Ready)

### ✅ Phase 1: Verify Build (5 min)
```bash
cd /home/mahonri/Desktop/BareMetal/Sensor_Cons
mkdir -p build/Debug && cd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../.. && make -j4
cd ../..
STM32_Programmer_CLI --connect port=swd --download build/Debug/Sensor_Console.elf -hardRst -rst --start
```

---

### ✅ Phase 2: UART Console (10 min)

**Hardware:** FT232RL → PA10(RX), PA9(TX), GND

**Code:**
```c
#include "uart.h"
UART_Init();
printf("STM32 Ready!\r\n");
```

**Test:** `screen /dev/ttyUSB0 115200`

---

### ✅ Phase 3: Relay (5 min)

**Hardware:** Relay → PB0, VCC, GND

**Code:**
```c
#include "relay.h"
Relay_Config_t cfg = {.GPIO_Port = GPIOB, .GPIO_Pin = GPIO_PIN_0, .Polarity = RELAY_ACTIVE_HIGH};
Relay_Handle_t relay;
Relay_Init(&relay, &cfg);
Relay_On(&relay);  // Test
```

---

### ✅ Phase 4: Buzzer (10 min)

**Active Buzzer (GPIO):**
```c
#include "buzzer.h"
Buzzer_Config_t cfg = {.Type = BUZZER_TYPE_ACTIVE, .GPIO_Port = GPIOC, .GPIO_Pin = GPIO_PIN_0};
Buzzer_Handle_t buz;
Buzzer_Init(&buz, &cfg);
Buzzer_On(&buz);
HAL_Delay(100);
Buzzer_Off(&buz);
```

**Passive Buzzer (PWM):**
```c
Buzzer_Config_t pwm_cfg = {.Type = BUZZER_TYPE_PASSIVE, .Timer_Handle = &htim3, .Timer_Channel = TIM_CHANNEL_1};
Buzzer_Init(&pwm_buz, &pwm_cfg);
Buzzer_PlayTone(&pwm_buz, 1000, 50);  // 1kHz tone
```

---

### ✅ Phase 5: DHT22 Sensor (15 min)

**Hardware:** DHT22 → PA1(DATA) + 10kΩ pullup, 3.3V, GND

**Code:**
```c
#include "dht.h"
DHT_Config_t cfg = {.Type = DHT_TYPE_DHT22, .GPIO_Port = GPIOA, .GPIO_Pin = GPIO_PIN_1};
DHT_Handle_t dht;
DHT_Init(&dht, &cfg);

float temp, hum;
if (DHT_Read(&dht, &temp, &hum) == DHT_OK) {
    printf("Temp: %.1f°C, Humidity: %.1f%%\r\n", temp, hum);
}
```

**⚠️ Important:** 
- Add 10kΩ resistor between DATA and VCC
- Minimum 2-second interval between reads

---

### ✅ Phase 6: DS3231 RTC (15 min)

**Hardware:** DS3231 → PB6(SCL), PB9(SDA), 3.3V, GND

**Code:**
```c
#include "i2c.h"
#include "rtc.h"

I2C_Init();  // I2C1 on PB6/PB9

RTC_Time_t time = {.Hours = 14, .Minutes = 30, .Seconds = 0, .TimeFormat = RTC_HOURFORMAT_24};
RTC_Date_t date = {.Year = 25, .Month = RTC_MONTH_DECEMBER, .Date = 11, .WeekDay = RTC_WEEKDAY_WEDNESDAY};

RTC_Init();
RTC_SetTime(&time);
RTC_SetDate(&date);

// Read current time
RTC_GetTime(&time);
RTC_GetDate(&date);
printf("%02d/%02d/%02d %02d:%02d:%02d\r\n", 
       date.Date, date.Month, date.Year,
       time.Hours, time.Minutes, time.Seconds);
```

---

### ✅ Phase 7: CAN Bus (20 min)

**Hardware:** Waveshare CAN → PA12(TX), PA11(RX), VCC, GND + 120Ω termination

**Code:**
```c
#include "can.h"

CAN_Config cfg = {
    .mode = CAN_MODE_NORMAL,
    .baud_rate = CAN_BAUDRATE_500KBPS,
    .auto_retransmission = true,
    .auto_bus_off_recovery = true
};
CAN_Init(&cfg);

// Configure filter (accept all)
CAN_Filter filter = {
    .filter_id = 0,
    .filter_mask = 0,
    .filter_mode = CAN_FILTER_MASK_MODE,
    .filter_scale = CAN_FILTER_32BIT,
    .filter_fifo = CAN_FILTER_FIFO0,
    .filter_bank = 0
};
CAN_ConfigFilter(&filter);

// TX
CAN_Frame tx = {.id = 0x123, .id_type = CAN_ID_STANDARD, .frame_type = CAN_FRAME_DATA, .dlc = 8};
tx.data[0] = 0x01; // ... fill data
CAN_Transmit(&tx);

// RX
CAN_Frame rx;
if (CAN_Receive(&rx, 100) == HAL_OK) {
    printf("CAN RX: ID=0x%03lX\r\n", rx.id);
}
```

---

### ✅ Phase 8: LCD Display (10 min)

**Hardware:** Built-in on STM32F429I-DISC1

**Code:**
```c
#include "ltdc.h"

LTDC_Init();
LCD_Clear(LCD_COLOR_BLACK);
LCD_DisplayStringAt(10, 50, (uint8_t*)"Temp: 25.5C", LEFT_MODE);
LCD_DisplayStringAt(10, 80, (uint8_t*)"Humidity: 60%", LEFT_MODE);
```

---

### ✅ Phase 9: Flash Data Logging (10 min)

**Code:**
```c
#include "flash.h"

#define LOG_SECTOR    FLASH_SECTOR_11
#define LOG_ADDRESS   0x080E0000

typedef struct {
    uint32_t timestamp;
    float temperature;
    float humidity;
} Log_t;

Log_t log = {.timestamp = HAL_GetTick(), .temperature = 25.5, .humidity = 60.3};

FLASH_EraseSector(LOG_SECTOR);
FLASH_Write(LOG_ADDRESS, (uint8_t*)&log, sizeof(Log_t));

// Read back
Log_t read_log;
FLASH_Read(LOG_ADDRESS, (uint8_t*)&read_log, sizeof(Log_t));
printf("Logged: %.1f°C\r\n", read_log.temperature);
```

---

### ✅ Phase 10: FreeRTOS Multi-Tasking (20 min)

**Code:**
```c
#include "FreeRTOS.h"
#include "task.h"

void vTaskDHT(void *param) {
    DHT_Handle_t *dht = (DHT_Handle_t*)param;
    while (1) {
        float temp, hum;
        if (DHT_Read(dht, &temp, &hum) == DHT_OK) {
            printf("[DHT] %.1f°C, %.1f%%\r\n", temp, hum);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vTaskCAN(void *param) {
    CAN_Frame rx;
    while (1) {
        if (CAN_Receive(&rx, 100) == HAL_OK) {
            printf("[CAN] ID: 0x%lX\r\n", rx.id);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    DHT_Init(&dht, &dht_cfg);
    CAN_Init(&can_cfg);
    
    xTaskCreate(vTaskDHT, "DHT", 256, &dht, 2, NULL);
    xTaskCreate(vTaskCAN, "CAN", 256, NULL, 2, NULL);
    
    vTaskStartScheduler();
    while (1);
}
```

---

## 📊 Pin Assignment Summary

| Peripheral | Pin(s) | Notes |
|-----------|--------|-------|
| **UART** | PA9(TX), PA10(RX) | FT232RL console |
| **Relay** | PB0 | GPIO output |
| **Buzzer (Active)** | PC0 | GPIO output |
| **Buzzer (Passive)** | PA6 | TIM3_CH1 PWM |
| **DHT22** | PA1 | 10kΩ pullup required |
| **I2C (DS3231)** | PB6(SCL), PB9(SDA) | I2C1 |
| **CAN** | PA12(TX), PA11(RX) | 120Ω termination |
| **LCD** | Built-in | LTDC peripheral |
| **LED (Green)** | PG13 | Built-in |

---

## 🔍 Quick Troubleshooting

| Issue | Fix |
|-------|-----|
| **Build fails** | `sudo apt-get install gcc-arm-none-eabi cmake` |
| **Flash fails** | Check USB cable, verify ST-LINK: `STM32_Programmer_CLI --list` |
| **UART no output** | Check TX/RX crossover, baud rate 115200 |
| **DHT timeout** | Add 10kΩ pullup resistor |
| **I2C no ACK** | Check SDA/SCL wiring, verify address 0x68 |
| **CAN bus-off** | Add 120Ω termination resistors |
| **Task not running** | Check stack size (min 128 words), increase heap in FreeRTOSConfig.h |

---

## 📚 Full Documentation

For detailed explanations, wiring diagrams, and troubleshooting:
- **[GETTING_STARTED.md](GETTING_STARTED.md)** - Complete step-by-step guide
- **[HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md)** - Hardware integration details
- **[Peripherals/*/README.md](Peripherals/)** - API documentation

---

## ⏱️ Time Estimates

| Phase | Component | Time |
|-------|-----------|------|
| 1 | Build verification | 5 min |
| 2 | UART console | 10 min |
| 3 | Relay | 5 min |
| 4 | Buzzer | 10 min |
| 5 | DHT22 | 15 min |
| 6 | DS3231 RTC | 15 min |
| 7 | CAN bus | 20 min |
| 8 | LCD display | 10 min |
| 9 | Flash logging | 10 min |
| 10 | FreeRTOS | 20 min |
| **Total** | | **~2 hours** |

*Times assume no hardware issues and familiarity with STM32 development*

---

## 🎓 Recommended Learning Order

If you're new to embedded systems:
1. Start with UART (enables debugging)
2. Add simple outputs (Relay, Buzzer)
3. Read sensors (DHT22)
4. Try communication protocols (I2C, CAN)
5. Add display (LCD)
6. Enable multi-tasking (FreeRTOS)

**Skip to:** [GETTING_STARTED.md](GETTING_STARTED.md) for beginner-friendly explanations
