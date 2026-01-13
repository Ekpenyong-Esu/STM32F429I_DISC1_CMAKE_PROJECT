# External Libraries - FetchContent Integration

This directory contains CMake scripts for automatically downloading and integrating 23 popular embedded libraries using CMake's FetchContent module. These libraries follow the same separation of concerns pattern as the LVGL integration.

## Available Libraries

### Core Utility Libraries

#### 1. LittleFS (`littlefs-fetch.cmake`)
**Purpose**: Lightweight embedded filesystem with wear-leveling
**Best for**: Data logging, configuration storage, firmware updates
**Size**: ~3KB flash, minimal RAM
**Usage**: Perfect for your sensor console - log distance measurements to flash

#### 2. printf (`printf-fetch.cmake`)
**Purpose**: Lightweight printf implementation
**Best for**: Logging, data formatting, UART output
**Size**: ~1KB flash (much smaller than stdlib printf)
**Usage**: Format sensor data for logging or display

#### 3. Unity (`unity-fetch.cmake`)
**Purpose**: Unit testing framework for C code
**Best for**: Testing peripheral drivers (ADC, GPIO, sensors)
**Size**: ~5KB flash
**Usage**: Test your ultrasonic, IR, and laser distance drivers

#### 4. minIni (`minini-fetch.cmake`)
**Purpose**: INI file parser for configuration storage
**Best for**: Storing sensor calibration data, system settings
**Size**: ~2KB flash
**Usage**: Store sensor calibration curves, device configuration

#### 5. CMock (`cmock-fetch.cmake`)
**Purpose**: Mock framework for unit testing
**Best for**: Testing code that depends on HAL functions
**Size**: ~8KB flash
**Usage**: Mock HAL functions when testing drivers

### Sensor Libraries (STM32duino Compatible)

#### 6. BMP280 (`bmp280-fetch.cmake`)
**Purpose**: High-precision pressure/temperature sensor
**Interface**: I2C
**Range**: Pressure: 300-1100 hPa, Temp: -40°C to +85°C
**Usage**: Weather stations, altimeters, altitude measurement

#### 7. HTS221 (`hts221-fetch.cmake`)
**Purpose**: Humidity and temperature sensor
**Interface**: I2C
**Range**: Humidity: 0-100% RH, Temp: -40°C to +120°C
**Usage**: Environmental monitoring, HVAC systems

#### 8. LSM6DSL (`lsm6dsl-fetch.cmake`)
**Purpose**: 6-axis IMU (Accelerometer + Gyroscope)
**Interface**: I2C/SPI
**Features**: Motion detection, pedometer, tilt detection
**Usage**: Robotics, drones, motion tracking, gesture recognition

#### 9. LIS3MDL (`lis3mdl-fetch.cmake`)
**Purpose**: 3-axis magnetometer (digital compass)
**Interface**: I2C/SPI
**Range**: ±4/±8/±12/±16 gauss
**Usage**: Compass applications, navigation, orientation sensing

#### 10. VL53L1X (`vl53l1x-fetch.cmake`)
**Purpose**: Time-of-flight distance sensor
**Interface**: I2C
**Range**: Up to 4 meters, millimeter precision
**Usage**: Robotics, drones, presence detection, obstacle avoidance

#### 11. LPS22HB (`lps22hb-fetch.cmake`)
**Purpose**: Barometric pressure sensor
**Interface**: I2C/SPI
**Range**: 260-1260 hPa
**Usage**: Altimeters, weather stations, navigation

#### 12. APDS-9960 (`apds9960-fetch.cmake`)
**Purpose**: Gesture, proximity, and color sensor
**Interface**: I2C
**Features**: Gesture recognition, proximity detection, RGB sensing
**Usage**: Touchless interfaces, proximity detection, color measurement

### Communication Libraries

#### 13. CAN (`can-fetch.cmake`)
**Purpose**: CAN bus communication
**Interface**: CAN peripheral
**Features**: Multi-master network, high reliability
**Usage**: Automotive networks, industrial control systems

### System Libraries

#### 14. EEPROM (`eeprom-fetch.cmake`)
**Purpose**: EEPROM emulation using flash memory
**Interface**: Flash memory
**Features**: Wear-leveling, persistent storage
**Usage**: Configuration storage, calibration data, settings

#### 15. RTC (`rtc-fetch.cmake`)
**Purpose**: Real-time clock functionality
**Interface**: RTC peripheral
**Features**: Timekeeping, alarms, calendar
**Usage**: Timestamps for data logging, scheduling, time-based events

### Display/TFT Libraries

#### 16. ILI9341 (`ili9341-fetch.cmake`)
**Purpose**: Popular 320x240 TFT display controller
**Interface**: SPI
**Resolution**: 320x240 pixels
**Usage**: Color TFT displays (2.2"-3.2" screens), graphics applications

#### 17. ST7735 (`st7735-fetch.cmake`)
**Purpose**: Small TFT display controller
**Interface**: SPI
**Resolution**: 128x160 pixels
**Usage**: Small color displays (1.8" screens), portable devices

#### 18. SSD1306 (`ssd1306-fetch.cmake`)
**Purpose**: Monochrome OLED display controller
**Interface**: I2C/SPI
**Resolution**: 128x64 pixels
**Usage**: Text displays, low-power applications, status screens

#### 19. ST7789 (`st7789-fetch.cmake`)
**Purpose**: IPS TFT display controller
**Interface**: SPI
**Resolution**: 240x240 pixels
**Usage**: High-quality IPS displays (1.3"-2.0" screens), better color reproduction

#### 20. ILI9488 (`ili9488-fetch.cmake`)
**Purpose**: High-resolution TFT display controller
**Interface**: SPI
**Resolution**: 320x480 pixels
**Usage**: Large displays needing more screen real estate

#### 21. XPT2046 (`xpt2046-fetch.cmake`)
**Purpose**: Resistive touchscreen controller
**Interface**: SPI
**Features**: Touch coordinate detection, interrupt support
**Usage**: Touch input for TFT displays, resistive touch panels

#### 22. U8g2 (`u8g2-fetch.cmake`)
**Purpose**: Universal graphics library
**Interface**: Various (SPI, I2C, parallel)
**Features**: Supports 200+ displays, optimized graphics operations
**Usage**: Cross-platform graphics, text rendering, drawing primitives

#### 23. STMPE (`stmpe-fetch.cmake`)
**Purpose**: STMicroelectronics touchscreen controller
**Interface**: I2C
**Features**: Capacitive/resistive touch, gesture recognition
**Usage**: Touch input for displays, STMPE610/STMPE811 controllers

## Recommended for Your Project

Based on your STM32F429 Sensor Console with distance sensors and TFT display, I recommend:

### High Priority (Essential for sensor projects):
1. **LittleFS** - For logging sensor data to flash memory
2. **printf** - For formatted logging output
3. **Unity** - For testing your distance sensor drivers

### Medium Priority (Nice to have):
4. **minIni** - For storing sensor calibration data
5. **ILI9341** - For your main TFT display (if using 320x240 resolution)
6. **XPT2046** or **STMPE** - For touchscreen support (XPT2046 for resistive, STMPE for capacitive)

### Low Priority (Advanced testing):
7. **CMock** - For comprehensive unit testing with mocks

### Display Options (Choose based on your hardware):
- **ILI9341** - Most popular choice for 2.2"-3.2" TFT displays
- **ST7789** - Better choice for IPS displays with improved color
- **SSD1306** - Good for monochrome OLED displays (low power)
- **U8g2** - Universal graphics library supporting many displays
- **XPT2046** - Resistive touchscreen controller
- **STMPE** - STMicroelectronics touchscreen controller (STMPE610/STMPE811)

## How to Use

### 1. Choose Your Libraries

Edit your main `CMakeLists.txt` and add options for the libraries you want:

```cmake
# External Libraries - Choose which ones to include
option(USE_LITTLEFS "Include LittleFS filesystem" ON)
option(LITTLEFS_USE_LOCAL "Use local LittleFS checkout instead of FetchContent" OFF)
option(USE_UNITY "Include Unity testing framework" ON)
option(USE_PRINTF "Include lightweight printf" ON)
option(USE_MININI "Include minIni configuration parser" OFF)
option(USE_CMOCK "Include CMock mocking framework" OFF)

# Sensor Libraries
option(USE_BMP280 "Include BMP280 pressure/temperature sensor" OFF)
option(USE_HTS221 "Include HTS221 humidity/temperature sensor" OFF)
option(USE_LSM6DSL "Include LSM6DSL 6-axis IMU" OFF)
option(USE_LIS3MDL "Include LIS3MDL 3-axis magnetometer" OFF)
option(USE_VL53L1X "Include VL53L1X time-of-flight sensor" OFF)
option(USE_LPS22HB "Include LPS22HB barometric pressure sensor" OFF)
option(USE_APDS9960 "Include APDS-9960 gesture/proximity sensor" OFF)

# Communication Libraries
option(USE_CAN "Include CAN bus communication" OFF)

# System Libraries
option(USE_EEPROM "Include EEPROM emulation" OFF)
option(USE_RTC "Include RTC timekeeping" OFF)

# Display/TFT Libraries
option(USE_ILI9341 "Include ILI9341 TFT display driver" OFF)
option(USE_ST7735 "Include ST7735 TFT display driver" OFF)
option(USE_SSD1306 "Include SSD1306 OLED display driver" OFF)
option(USE_ST7789 "Include ST7789 IPS TFT display driver" OFF)
option(USE_ILI9488 "Include ILI9488 high-res TFT display driver" OFF)
option(USE_XPT2046 "Include XPT2046 resistive touchscreen" OFF)
option(USE_U8G2 "Include U8g2 universal graphics library" OFF)
option(USE_STMPE "Include STMPE touchscreen controller" OFF)
```

### 2. Include the Fetch/Local Scripts

Add the include statements after your LVGL configuration:

```cmake
# ------------------------------------------------------------------------------
# External Libraries
# ------------------------------------------------------------------------------
if(USE_LITTLEFS)
    if(LITTLEFS_USE_LOCAL)
        message(STATUS "Config: Using local LittleFS checkout")
        include("cmake/littlefs-local.cmake")
    else()
        message(STATUS "Config: Using FetchContent to download LittleFS")
        include("cmake/littlefs-fetch.cmake")
    endif()
endif()

if(USE_UNITY)
    include("cmake/unity-fetch.cmake")
endif()

if(USE_PRINTF)
    include("cmake/printf-fetch.cmake")
endif()

if(USE_MININI)
    include("cmake/minini-fetch.cmake")
endif()

if(USE_CMOCK)
    include("cmake/cmock-fetch.cmake")
endif()

# Sensor Libraries
if(USE_BMP280)
    include("cmake/bmp280-fetch.cmake")
endif()

if(USE_HTS221)
    include("cmake/hts221-fetch.cmake")
endif()

if(USE_LSM6DSL)
    include("cmake/lsm6dsl-fetch.cmake")
endif()

if(USE_LIS3MDL)
    include("cmake/lis3mdl-fetch.cmake")
endif()

if(USE_VL53L1X)
    include("cmake/vl53l1x-fetch.cmake")
endif()

if(USE_LPS22HB)
    include("cmake/lps22hb-fetch.cmake")
endif()

if(USE_APDS9960)
    include("cmake/apds9960-fetch.cmake")
endif()

# Communication Libraries
if(USE_CAN)
    include("cmake/can-fetch.cmake")
endif()

# System Libraries
if(USE_EEPROM)
    include("cmake/eeprom-fetch.cmake")
endif()

if(USE_RTC)
    include("cmake/rtc-fetch.cmake")
endif()

# Display/TFT Libraries
if(USE_ILI9341)
    include("cmake/ili9341-fetch.cmake")
endif()

if(USE_ST7735)
    include("cmake/st7735-fetch.cmake")
endif()

if(USE_SSD1306)
    include("cmake/ssd1306-fetch.cmake")
endif()

if(USE_ST7789)
    include("cmake/st7789-fetch.cmake")
endif()

if(USE_ILI9488)
    include("cmake/ili9488-fetch.cmake")
endif()

if(USE_XPT2046)
    include("cmake/xpt2046-fetch.cmake")
endif()

if(USE_U8G2)
    include("cmake/u8g2-fetch.cmake")
endif()

if(USE_STMPE)
    include("cmake/stmpe-fetch.cmake")
endif()
```

### 3. Link the Libraries

Add the library targets to your `target_link_libraries()`:

```cmake
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    ${LVGL_TARGET}

    # Core utility libraries
    $<$<BOOL:${USE_LITTLEFS}>:littlefs>
    $<$<BOOL:${USE_UNITY}>:unity>
    $<$<BOOL:${USE_PRINTF}>:printf>
    $<$<BOOL:${USE_MININI}>:minini>
    $<$<BOOL:${USE_CMOCK}>:cmock>

    # Sensor libraries
    $<$<BOOL:${USE_BMP280}>:BMP280>
    $<$<BOOL:${USE_HTS221}>:HTS221>
    $<$<BOOL:${USE_LSM6DSL}>:LSM6DSL>
    $<$<BOOL:${USE_LIS3MDL}>:LIS3MDL>
    $<$<BOOL:${USE_VL53L1X}>:VL53L1X>
    $<$<BOOL:${USE_LPS22HB}>:LPS22HB>
    $<$<BOOL:${USE_APDS9960}>:APDS9960>

    # Communication libraries
    $<$<BOOL:${USE_CAN}>:CAN>

    # System libraries
    $<$<BOOL:${USE_EEPROM}>:EEPROM>
    $<$<BOOL:${USE_RTC}>:RTC>

    # Display/TFT libraries
    $<$<BOOL:${USE_ILI9341}>:ILI9341>
    $<$<BOOL:${USE_ST7735}>:ST7735>
    $<$<BOOL:${USE_SSD1306}>:SSD1306>
    $<$<BOOL:${USE_ST7789}>:ST7789>
    $<$<BOOL:${USE_ILI9488}>:ILI9488>
    $<$<BOOL:${USE_XPT2046}>:XPT2046>
    $<$<BOOL:${USE_U8G2}>:U8G2>
    $<$<BOOL:${USE_STMPE}>:STMPE>
)
```

### 4. Use in Your Code

```c
// Example: Using LittleFS for data logging
#include <lfs.h>

// Example: Using printf for formatted output
#include <printf.h>

// Example: Using Unity for testing
#include <unity.h>
```

## Library Integration Examples

### LittleFS + printf for Sensor Logging

```c
#include <lfs.h>
#include <printf.h>

// In your sensor reading code:
char buffer[64];
snprintf(buffer, sizeof(buffer), "Distance: %d mm, ADC: %d\n",
         distance_mm, adc_value);

// Write to LittleFS file
lfs_file_t file;
lfs_file_open(&lfs, &file, "sensor_log.txt", LFS_O_APPEND | LFS_O_CREAT);
lfs_file_write(&lfs, &file, buffer, strlen(buffer));
lfs_file_close(&lfs, &file);
```

### Unity for Testing Distance Sensors

```c
#include <unity.h>
#include "ultrasonic.h"

// Test function
void test_ultrasonic_measurement(void) {
    ULTRASONIC_Handle_t hultra;
    // Initialize and test your ultrasonic driver
    TEST_ASSERT_EQUAL(ULTRASONIC_OK, ULTRASONIC_Init(&hultra, &htim4, TIM_CHANNEL_1, &pins));
    TEST_ASSERT_GREATER_THAN(0, ULTRASONIC_MeasureDistance(&hultra));
}
```

### Sensor Integration Examples

#### BMP280 Pressure/Temperature Sensor

```c
#include <BMP280.h>

// Initialize I2C first (using your existing I2C driver)
I2C_Handle_t hi2c1;
I2C_Init(&hi2c1, I2C1, 400000); // 400kHz

// Initialize BMP280
BMP280 bmp280;
bmp280.begin(0x76, &hi2c1); // Default I2C address

// Read pressure and temperature
float pressure = bmp280.readPressure();
float temperature = bmp280.readTemperature();

// Log to console
LOG_INFO("BMP280 - Pressure: %.2f hPa, Temp: %.2f°C", pressure, temperature);
```

#### LSM6DSL 6-Axis IMU

```c
#include <LSM6DSL.h>

// Initialize I2C
I2C_Handle_t hi2c1;
I2C_Init(&hi2c1, I2C1, 400000);

// Initialize IMU
LSM6DSL imu;
imu.begin(0x6A, &hi2c1); // Default I2C address

// Configure accelerometer (±2g) and gyroscope (±250 dps)
imu.setAccelRange(LSM6DSL_ACCEL_RANGE_2G);
imu.setGyroRange(LSM6DSL_GYRO_RANGE_250DPS);

// Read sensor data
float accelX = imu.readAccelX();
float gyroZ = imu.readGyroZ();

LOG_INFO("IMU - Accel X: %.2f m/s², Gyro Z: %.2f dps", accelX, gyroZ);
```

#### VL53L1X Time-of-Flight Distance Sensor

```c
#include <VL53L1X.h>

// Initialize I2C
I2C_Handle_t hi2c1;
I2C_Init(&hi2c1, I2C1, 400000);

// Initialize VL53L1X
VL53L1X sensor;
sensor.setBus(&hi2c1);
sensor.setAddress(0x29); // Default I2C address
sensor.init();

// Configure for short range (up to 1.3m)
sensor.setDistanceMode(VL53L1X::Short);
sensor.setMeasurementTimingBudget(50000); // 50ms timing budget
sensor.startContinuous(50); // 50ms between measurements

// Read distance
uint16_t distance = sensor.read();
if (sensor.timeoutOccurred()) {
    LOG_ERROR("VL53L1X timeout occurred");
} else {
    LOG_INFO("VL53L1X Distance: %d mm", distance);
}
```

#### HTS221 Humidity/Temperature Sensor

```c
#include <HTS221.h>

// Initialize I2C
I2C_Handle_t hi2c1;
I2C_Init(&hi2c1, I2C1, 400000);

// Initialize HTS221
HTS221 hts221;
hts221.begin(0x5F, &hi2c1); // Default I2C address

// Read humidity and temperature
float humidity = hts221.readHumidity();
float temperature = hts221.readTemperature();

LOG_INFO("HTS221 - Humidity: %.1f%%, Temp: %.2f°C", humidity, temperature);
```

#### LIS3MDL 3-Axis Magnetometer

```c
#include <LIS3MDL.h>

// Initialize I2C
I2C_Handle_t hi2c1;
I2C_Init(&hi2c1, I2C1, 400000);

// Initialize magnetometer
LIS3MDL mag;
mag.begin(0x1C, &hi2c1); // Default I2C address

// Configure for ±4 gauss range
mag.setRange(LIS3MDL_RANGE_4GAUSS);

// Read magnetic field
float magX = mag.readMagneticX();
float magY = mag.readMagneticY();
float magZ = mag.readMagneticZ();

LOG_INFO("Magnetometer - X: %.2f, Y: %.2f, Z: %.2f gauss", magX, magY, magZ);
```

#### APDS-9960 Gesture/Proximity Sensor

```c
#include <APDS9960.h>

// Initialize I2C
I2C_Handle_t hi2c1;
I2C_Init(&hi2c1, I2C1, 400000);

// Initialize APDS-9960
APDS9960 apds;
apds.begin(0x39, &hi2c1); // Default I2C address

// Enable proximity sensing
apds.enableProximity(true);

// Read proximity
uint8_t proximity = apds.readProximity();

LOG_INFO("APDS-9960 Proximity: %d", proximity);
```

### CAN Bus Communication

```c
#include <CAN.h>

// Initialize CAN peripheral (using your existing CAN driver)
CAN_Handle_t hcan1;
CAN_Init(&hcan1, CAN1, 500000); // 500kbps

// Initialize CAN library
CAN.begin(500E3); // 500kbps
CAN.setPins(PA11, PA12); // CAN RX, TX pins

// Send a message
CAN.beginPacket(0x123); // CAN ID
CAN.write('H');
CAN.write('e');
CAN.write('l');
CAN.write('l');
CAN.write('o');
CAN.endPacket();

// Receive messages (in interrupt or polling loop)
if (CAN.parsePacket()) {
    uint32_t canId = CAN.packetId();
    LOG_INFO("Received CAN message ID: 0x%X", canId);
    while (CAN.available()) {
        char c = CAN.read();
        // Process received data
    }
}
```

### EEPROM Emulation

```c
#include <EEPROM.h>

// Initialize EEPROM (uses flash memory internally)
EEPROM.begin();

// Write data
uint16_t address = 0;
float sensor_calibration = 1.234f;
EEPROM.put(address, sensor_calibration);

// Read data back
float read_value;
EEPROM.get(address, read_value);
LOG_INFO("EEPROM Read: %.3f", read_value);

// Commit changes to flash
EEPROM.commit();
```

### RTC Timekeeping

```c
#include <RTC.h>

// Initialize RTC peripheral (using your existing RTC driver)
RTC_Handle_t hrtc;
RTC_Init(&hrtc, RTC);

// Initialize RTC library
RTC.begin();

// Set current time (year, month, day, hour, minute, second)
RTC.setTime(2024, 1, 15, 14, 30, 0);

// Read current time
uint16_t year = RTC.getYear();
uint8_t month = RTC.getMonth();
uint8_t day = RTC.getDay();
uint8_t hour = RTC.getHour();
uint8_t minute = RTC.getMinute();
uint8_t second = RTC.getSecond();

LOG_INFO("RTC Time: %04d-%02d-%02d %02d:%02d:%02d",
         year, month, day, hour, minute, second);
```

## Separation of Concerns

Each library follows the same pattern as your LVGL integration:
- **Fetch**: Downloads and makes library available
- **Local**: Alternative using local checkout
- **Clean separation**: Each library is independent
- **Optional inclusion**: Enable/disable via CMake options

## Building with External Libraries

```bash
# Configure with desired libraries
cmake -B build -S . \
    -DUSE_LITTLEFS=ON \
    -DUSE_PRINTF=ON \
    -DUSE_UNITY=ON \
    -DUSE_BMP280=ON \
    -DUSE_LSM6DSL=ON \
    -DUSE_VL53L1X=ON

# Build as usual
cmake --build build
```

The libraries will be automatically downloaded on first build and cached for subsequent builds.
