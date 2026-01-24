# ============================================================================
# STM32duino HTS221 - HTS221 Humidity/Temperature Sensor - CMake Integration
# ============================================================================
# This file automatically downloads and configures HTS221 sensor driver.
#
# What it does:
# 1. Downloads HTS221 driver from GitHub (STM32duino library)
# 2. Makes HTS221 available as a CMake target
# 3. Provides humidity and temperature sensing
#
# For beginners:
# - HTS221 measures relative humidity and temperature
# - I2C interface, low power consumption
# - Perfect for environmental monitoring
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download HTS221 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  hts221
  GIT_REPOSITORY https://github.com/stm32duino/HTS221.git
  GIT_TAG main
)

# Actually download and make HTS221 available to CMake
FetchContent_MakeAvailable(hts221)

# ----------------------------------------------------------------------------
# Create HTS221 library target
# ----------------------------------------------------------------------------
if(NOT TARGET hts221)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(hts221 SOURCE_DIR HTS221_SOURCE_DIR)

  add_library(hts221 STATIC
    ${HTS221_SOURCE_DIR}/HTS221.cpp
  )

  target_include_directories(hts221 INTERFACE
    ${HTS221_SOURCE_DIR}
  )

  target_compile_definitions(hts221 PUBLIC
    # HTS221 specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'hts221' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "HTS221: Humidity/Temperature sensor driver ready")
message(STATUS "HTS221: Include <HTS221.h> in your code")
message(STATUS "HTS221: Link with 'hts221' target in CMakeLists.txt")
message(STATUS "HTS221: Requires I2C peripheral driver")
