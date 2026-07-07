# ============================================================================
# STM32duino APDS-9960 - APDS-9960 Gesture/Proximity Sensor - CMake Integration
# ============================================================================
# This file automatically downloads and configures APDS-9960 sensor driver.
#
# What it does:
# 1. Downloads APDS-9960 driver from GitHub (STM32duino library)
# 2. Makes APDS-9960 available as a CMake target
# 3. Provides gesture recognition, proximity, and color sensing
#
# For beginners:
# - APDS-9960 is a digital proximity, ambient light, RGB and gesture sensor
# - I2C interface, gesture recognition capabilities
# - Perfect for touchless interfaces, proximity detection
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download APDS-9960 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  apds9960
  GIT_REPOSITORY https://github.com/stm32duino/APDS-9960.git
  GIT_TAG main
)

# Actually download and make APDS-9960 available to CMake
FetchContent_MakeAvailable(apds9960)

# ----------------------------------------------------------------------------
# Create APDS-9960 library target
# ----------------------------------------------------------------------------
if(NOT TARGET apds9960)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(apds9960 SOURCE_DIR APDS9960_SOURCE_DIR)

  add_library(apds9960 STATIC
    ${APDS9960_SOURCE_DIR}/APDS9960.cpp
  )

  target_include_directories(apds9960 INTERFACE
    ${APDS9960_SOURCE_DIR}
  )

  target_compile_definitions(apds9960 PUBLIC
    # APDS-9960 specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'apds9960' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "APDS-9960: Gesture/proximity/color sensor driver ready")
message(STATUS "APDS-9960: Include <APDS9960.h> in your code")
message(STATUS "APDS-9960: Link with 'apds9960' target in CMakeLists.txt")
message(STATUS "APDS-9960: Requires I2C peripheral driver")
