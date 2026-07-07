# ============================================================================
# STM32duino LPS22HB - LPS22HB Pressure Sensor - CMake Integration
# ============================================================================
# This file automatically downloads and configures LPS22HB pressure sensor driver.
#
# What it does:
# 1. Downloads LPS22HB driver from GitHub (STM32duino library)
# 2. Makes LPS22HB available as a CMake target
# 3. Provides high-precision pressure sensing
#
# For beginners:
# - LPS22HB is a barometric pressure sensor
# - I2C/SPI interface, high accuracy
# - Perfect for altimeters, weather stations
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download LPS22HB driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  lps22hb
  GIT_REPOSITORY https://github.com/stm32duino/LPS22HB.git
  GIT_TAG main
)

# Actually download and make LPS22HB available to CMake
FetchContent_MakeAvailable(lps22hb)

# ----------------------------------------------------------------------------
# Create LPS22HB library target
# ----------------------------------------------------------------------------
if(NOT TARGET lps22hb)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(lps22hb SOURCE_DIR LPS22HB_SOURCE_DIR)

  add_library(lps22hb STATIC
    ${LPS22HB_SOURCE_DIR}/LPS22HB.cpp
  )

  target_include_directories(lps22hb INTERFACE
    ${LPS22HB_SOURCE_DIR}
  )

  target_compile_definitions(lps22hb PUBLIC
    # LPS22HB specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'lps22hb' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "LPS22HB: High-precision pressure sensor driver ready")
message(STATUS "LPS22HB: Include <LPS22HB.h> in your code")
message(STATUS "LPS22HB: Link with 'lps22hb' target in CMakeLists.txt")
message(STATUS "LPS22HB: Requires I2C or SPI peripheral driver")
