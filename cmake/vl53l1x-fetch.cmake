# ============================================================================
# STM32duino VL53L1X - VL53L1X Time-of-Flight Distance Sensor - CMake Integration
# ============================================================================
# This file automatically downloads and configures VL53L1X ToF sensor driver.
#
# What it does:
# 1. Downloads VL53L1X driver from GitHub (STM32duino library)
# 2. Makes VL53L1X available as a CMake target
# 3. Provides high-precision distance measurement
#
# For beginners:
# - VL53L1X is a laser time-of-flight distance sensor
# - I2C interface, up to 4m range, millimeter precision
# - Perfect for robotics, drones, presence detection
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download VL53L1X driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  vl53l1x
  GIT_REPOSITORY https://github.com/stm32duino/VL53L1X.git
  GIT_TAG main
)

# Actually download and make VL53L1X available to CMake
FetchContent_MakeAvailable(vl53l1x)

# ----------------------------------------------------------------------------
# Create VL53L1X library target
# ----------------------------------------------------------------------------
if(NOT TARGET vl53l1x)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(vl53l1x SOURCE_DIR VL53L1X_SOURCE_DIR)

  add_library(vl53l1x STATIC
    ${VL53L1X_SOURCE_DIR}/VL53L1X.cpp
  )

  target_include_directories(vl53l1x INTERFACE
    ${VL53L1X_SOURCE_DIR}
  )

  target_compile_definitions(vl53l1x PUBLIC
    # VL53L1X specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'vl53l1x' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "VL53L1X: Time-of-flight distance sensor driver ready")
message(STATUS "VL53L1X: Include <VL53L1X.h> in your code")
message(STATUS "VL53L1X: Link with 'vl53l1x' target in CMakeLists.txt")
message(STATUS "VL53L1X: Requires I2C peripheral driver")
