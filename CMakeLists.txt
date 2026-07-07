# ==============================================================================
# STM32F429I-Discovery CMake Configuration
# ==============================================================================
# Main build configuration for the Sensor Console embedded project.
# This file orchestrates the build of STM32 HAL, FreeRTOS, LVGL, and
# peripheral drivers for the STM32F429I-DISC1 development board.
#
# For Beginners:
# - This file is the entry point for the entire build system
# - It coordinates STM32CubeMX generated code, peripherals, and libraries
# - Modify carefully - incorrect changes can break the build
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# Compiler Settings - C Language Standard
# ------------------------------------------------------------------------------
# Configure C language standard before project() command
set(CMAKE_C_STANDARD 11)              # Use C11 standard (modern C features)
set(CMAKE_C_STANDARD_REQUIRED ON)     # Enforce C11, fail if not available
set(CMAKE_C_EXTENSIONS ON)            # Allow GNU extensions (needed for embedded)

# ------------------------------------------------------------------------------
# Build Configuration
# ------------------------------------------------------------------------------
# Set build type if not specified (Debug includes debugging symbols)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# ------------------------------------------------------------------------------
# Project Definition
# ------------------------------------------------------------------------------
set(CMAKE_PROJECT_NAME Sensor_Console)

# ------------------------------------------------------------------------------
# Toolchain Configuration
# ------------------------------------------------------------------------------
# Load ARM GCC cross-compiler toolchain for STM32 (must be before project())
include("cmake/gcc-arm-none-eabi.cmake")

# Generate compile_commands.json for IDE tools (clangd, IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# ------------------------------------------------------------------------------
# Project Declaration
# ------------------------------------------------------------------------------
project(${CMAKE_PROJECT_NAME})
message(STATUS "Project: ${CMAKE_PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Enable language support for C, C++, and Assembly (needed for startup files and C++ libraries)
enable_language(C CXX ASM)

# ------------------------------------------------------------------------------
# Create Executable Target
# ------------------------------------------------------------------------------
# Create the main executable - sources will be added below
add_executable(${CMAKE_PROJECT_NAME})

# ------------------------------------------------------------------------------
# STM32CubeMX Generated Code
# ------------------------------------------------------------------------------
# Include HAL drivers, middleware, and application sources from CubeMX
# This creates the 'stm32cubemx' library target
add_subdirectory(cmake/stm32cubemx)

# ------------------------------------------------------------------------------
# LVGL Graphics Library
# ------------------------------------------------------------------------------
# Allow choosing between local LVGL checkout or the automatic FetchContent
# Default is to use the fetched LVGL. To use a local copy set LVGL_USE_LOCAL
# to ON and optionally set LVGL_LOCAL_DIR to the path of your local lvgl.
option(LVGL_USE_LOCAL "Use a local LVGL checkout instead of FetchContent" ON)

if(LVGL_USE_LOCAL)
    message(STATUS "Config: Using local LVGL checkout")
    include("cmake/lvgl-local.cmake")
else()
    message(STATUS "Config: Using FetchContent to download LVGL")
    include("cmake/lvgl-fetch.cmake")
endif()

# ------------------------------------------------------------------------------
# External Libraries - Choose which ones to include
# ------------------------------------------------------------------------------
# Core utility libraries
option(USE_LITTLEFS "Include LittleFS filesystem" ON)
option(LITTLEFS_USE_LOCAL "Use local LittleFS checkout instead of FetchContent" OFF)
option(USE_FATFS "Include FatFS filesystem" OFF)
option(USE_UNITY "Include Unity testing framework" ON)
option(USE_PRINTF "Include lightweight printf" ON)
option(USE_MININI "Include minIni configuration parser" OFF)
option(USE_CMOCK "Include CMock mocking framework" OFF)
option(USE_CJSON "Include cJSON JSON parser" OFF)
option(USE_CLI "Include CLI command line interface" OFF)
option(USE_LOGGING "Include ulog lightweight logging framework" OFF)

# Sensor Libraries (enable based on your hardware)
option(USE_BMP280 "Include BMP280 pressure/temperature sensor" OFF)
option(USE_HTS221 "Include HTS221 humidity/temperature sensor" OFF)
option(USE_LSM6DSL "Include LSM6DSL 6-axis IMU" OFF)
option(USE_LIS3MDL "Include LIS3MDL 3-axis magnetometer" OFF)
option(USE_VL53L1X "Include VL53L1X time-of-flight sensor" OFF)
option(USE_LPS22HB "Include LPS22HB barometric pressure sensor" OFF)
option(USE_APDS9960 "Include APDS-9960 gesture/proximity sensor" OFF)

# Communication Libraries
option(USE_CAN "Include CAN bus communication" OFF)
option(USE_RS_COMM "Include RS-485 serial communication" OFF)

# System Libraries
option(USE_EEPROM "Include EEPROM emulation" OFF)
option(USE_RTC "Include RTC timekeeping" OFF)
option(USE_CRC "Include CRC data integrity library" OFF)

# Display/TFT Libraries (enable based on your display hardware)
option(USE_ILI9341 "Include ILI9341 TFT display driver" OFF)
option(USE_ST7735 "Include ST7735 TFT display driver" OFF)
option(USE_SSD1306 "Include SSD1306 OLED display driver" OFF)
option(USE_ST7789 "Include ST7789 IPS TFT display driver" OFF)
option(USE_ILI9488 "Include ILI9488 high-res TFT display driver" OFF)
option(USE_XPT2046 "Include XPT2046 resistive touchscreen" OFF)
option(USE_U8G2 "Include U8g2 universal graphics library" OFF)
option(USE_STMPE "Include STMPE touchscreen controller" OFF)

# Motor Libraries
option(USE_STEPPER "Include StepperDriver stepper motor library" OFF)
option(USE_SERVO "Include ServoMotor servo library" OFF)
# Include External Library Fetch Scripts
# ------------------------------------------------------------------------------
# Core utility libraries
if(USE_LITTLEFS)
    if(LITTLEFS_USE_LOCAL)
        message(STATUS "Config: Using local LittleFS checkout")
        include("cmake/littlefs-local.cmake")
    else()
        message(STATUS "Config: Using FetchContent to download LittleFS")
        include("cmake/littlefs-fetch.cmake")
    endif()
endif()

if(USE_FATFS)
    include("cmake/fatfs-fetch.cmake")
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

if(USE_CJSON)
    include("cmake/cjson-fetch.cmake")
endif()

if(USE_CLI)
    include("cmake/cli-fetch.cmake")
endif()

if(USE_LOGGING)
    include("cmake/logging-fetch.cmake")
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
if(USE_RS_COMM)
    include("cmake/rs_comm-fetch.cmake")
endif()

# System Libraries
if(USE_EEPROM)
    include("cmake/eeprom-fetch.cmake")
endif()

if(USE_RTC)
    include("cmake/rtc-fetch.cmake")
endif()

if(USE_CRC)
    include("cmake/crc-fetch.cmake")
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

# Motor Libraries
if(USE_STEPPER)
    include("cmake/stepper-fetch.cmake")
endif()

if(USE_SERVO)
    include("cmake/servo-fetch.cmake")
endif()

# ------------------------------------------------------------------------------
# User Application Sources (Add Your Main Code Here)
# ------------------------------------------------------------------------------
# Add your custom application .c files that aren't in Peripherals/
# Example: target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/my_app.c)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user application sources here
    # Example: ${CMAKE_SOURCE_DIR}/App/my_module.c
)





# ------------------------------------------------------------------------------
# Compile Definitions (Preprocessor Macros)
# ------------------------------------------------------------------------------
# Add preprocessor defines for conditional compilation
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # LVGL configuration: Use simplified include paths
    LV_CONF_INCLUDE_SIMPLE

    # Add your custom defines here:
    # MY_CUSTOM_FEATURE=1
    # APP_VERSION_MAJOR=1
)

# ------------------------------------------------------------------------------
# Link Libraries to Executable
# ------------------------------------------------------------------------------
# Link all required libraries to the final executable
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx      # STM32 HAL, middleware (FreeRTOS, USB), startup code
    ${LVGL_TARGET}   # LVGL graphics library (auto-resolved target name)

    # Core utility libraries
    $<$<BOOL:${USE_LITTLEFS}>:littlefs>
    $<$<BOOL:${USE_FATFS}>:fatfs>
    $<$<BOOL:${USE_UNITY}>:unity>
    $<$<BOOL:${USE_PRINTF}>:printf>
    $<$<BOOL:${USE_MININI}>:minini>
    $<$<BOOL:${USE_CMOCK}>:cmock>
    $<$<BOOL:${USE_CJSON}>:cjson>
    $<$<BOOL:${USE_CLI}>:cli>
    $<$<BOOL:${USE_LOGGING}>:logging>

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
    $<$<BOOL:${USE_RS_COMM}>:rs_comm>

    # System libraries
    $<$<BOOL:${USE_EEPROM}>:EEPROM>
    $<$<BOOL:${USE_RTC}>:RTC>
    $<$<BOOL:${USE_CRC}>:crc>

    # Display/TFT libraries
    $<$<BOOL:${USE_ILI9341}>:ILI9341>
    $<$<BOOL:${USE_ST7735}>:ST7735>
    $<$<BOOL:${USE_SSD1306}>:SSD1306>
    $<$<BOOL:${USE_ST7789}>:ST7789>
    $<$<BOOL:${USE_ILI9488}>:ILI9488>
    $<$<BOOL:${USE_XPT2046}>:XPT2046>
    $<$<BOOL:${USE_U8G2}>:U8G2>
    $<$<BOOL:${USE_STMPE}>:STMPE>

    # Motor libraries
    $<$<BOOL:${USE_STEPPER}>:stepperdriver>
    $<$<BOOL:${USE_SERVO}>:servomotor>
)

# ------------------------------------------------------------------------------
# Peripheral Drivers - Automatic Discovery
# ------------------------------------------------------------------------------
# This section automatically includes all peripheral drivers from Peripherals/
# Each peripheral folder (ADC, GPIO, LVGL, etc.) is added to include paths

# Step 1: Find all peripheral directories
file(GLOB PERIPHERAL_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/*")

# Step 2: Add each peripheral folder to include paths
foreach(dir ${PERIPHERAL_DIRS})
    if(IS_DIRECTORY ${dir})
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${dir})
    endif()
endforeach()

# Step 3: Find all .c source files in peripheral subdirectories
# CONFIGURE_DEPENDS re-runs the glob when files are added/removed
file(GLOB_RECURSE PERIPHERALS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/*/*.c"
)

# Step 4: Add peripheral sources to the executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PERIPHERALS_SOURCES})

list(LENGTH PERIPHERAL_DIRS PERIPHERAL_DIR_COUNT)
message(STATUS "Found ${PERIPHERAL_DIR_COUNT} peripheral directories")
message(STATUS "Added peripheral sources from: Peripherals/*/")

# ------------------------------------------------------------------------------
# Optional: Example Projects (Currently Disabled)
# ------------------------------------------------------------------------------
# Uncomment to include example code from Examples/ directory
# file(GLOB EXAMPLE_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Examples/*")
# foreach(dir ${EXAMPLE_DIRS})
#     if(IS_DIRECTORY ${dir})
#         target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${dir})
#     endif()
# endforeach()
#
# file(GLOB_RECURSE EXAMPLES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Examples/*/*.c")
# target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${EXAMPLES_SOURCES})

# ------------------------------------------------------------------------------
# Optional: Copy compile_commands.json to Root (For Better IDE Support)
# ------------------------------------------------------------------------------
# Uncomment to auto-copy compile_commands.json after each build
# This helps IDE tools find the file without configuring build directory
# add_custom_command(
#     TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${CMAKE_BINARY_DIR}/compile_commands.json
#         ${CMAKE_SOURCE_DIR}/compile_commands.json
#     COMMENT "Copying compile_commands.json to project root for IDE integration"
# )

# ==============================================================================
# Build Complete - Binary Output
# ==============================================================================
# After successful build, the output files are located in:
#   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf  (Executable)
#   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin  (Binary for flashing)
#   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex  (Intel HEX format)
# ==============================================================================

message(STATUS "Configuration complete - ready to build!")
