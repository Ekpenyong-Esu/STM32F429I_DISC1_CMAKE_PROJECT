# ============================================================================
# STM32duino BMP280 - BMP280 Pressure/Temperature Sensor - CMake Integration
# ============================================================================
# This file automatically downloads and configures BMP280 sensor driver.
#
# What it does:
# 1. Downloads BMP280 driver from GitHub (STM32duino library)
# 2. Makes BMP280 available as a CMake target
# 3. Provides pressure and temperature sensing
#
# For beginners:
# - BMP280 is a high-precision pressure/temperature sensor
# - I2C interface, perfect for weather stations, altimeters
# - Works with your existing I2C peripheral driver
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download BMP280 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  bmp280
  GIT_REPOSITORY https://github.com/stm32duino/BMP280.git
  GIT_TAG main
)

# Actually download and make BMP280 available to CMake
FetchContent_MakeAvailable(bmp280)

# ----------------------------------------------------------------------------
# Create BMP280 library target
# ----------------------------------------------------------------------------
if(NOT TARGET bmp280)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(bmp280 SOURCE_DIR BMP280_SOURCE_DIR)

  add_library(bmp280 STATIC
    ${BMP280_SOURCE_DIR}/BMP280.cpp
  )

  target_include_directories(bmp280 INTERFACE
    ${BMP280_SOURCE_DIR}
  )

  # BMP280 uses I2C, so it needs the I2C driver
  # You'll need to link this when using BMP280
  target_compile_definitions(bmp280 PUBLIC
    # Add any specific configuration here
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'bmp280' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "BMP280: Pressure/Temperature sensor driver ready")
message(STATUS "BMP280: Include <BMP280.h> in your code")
message(STATUS "BMP280: Link with 'bmp280' target in CMakeLists.txt")
message(STATUS "BMP280: Requires I2C peripheral driver")
