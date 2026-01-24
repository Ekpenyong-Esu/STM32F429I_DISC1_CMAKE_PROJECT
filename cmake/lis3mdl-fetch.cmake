# ============================================================================
# STM32duino LIS3MDL - LIS3MDL Magnetometer - CMake Integration
# ============================================================================
# This file automatically downloads and configures LIS3MDL magnetometer driver.
#
# What it does:
# 1. Downloads LIS3MDL driver from GitHub (STM32duino library)
# 2. Makes LIS3MDL available as a CMake target
# 3. Provides 3-axis magnetic field sensing
#
# For beginners:
# - LIS3MDL is a 3-axis magnetometer (compass sensor)
# - I2C/SPI interface, high sensitivity
# - Perfect for compass applications, navigation
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download LIS3MDL driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  lis3mdl
  GIT_REPOSITORY https://github.com/stm32duino/LIS3MDL.git
  GIT_TAG main
)

# Actually download and make LIS3MDL available to CMake
FetchContent_MakeAvailable(lis3mdl)

# ----------------------------------------------------------------------------
# Create LIS3MDL library target
# ----------------------------------------------------------------------------
if(NOT TARGET lis3mdl)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(lis3mdl SOURCE_DIR LIS3MDL_SOURCE_DIR)

  add_library(lis3mdl STATIC
    ${LIS3MDL_SOURCE_DIR}/LIS3MDL.cpp
  )

  target_include_directories(lis3mdl INTERFACE
    ${LIS3MDL_SOURCE_DIR}
  )

  target_compile_definitions(lis3mdl PUBLIC
    # LIS3MDL specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'lis3mdl' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "LIS3MDL: 3-axis magnetometer sensor driver ready")
message(STATUS "LIS3MDL: Include <LIS3MDL.h> in your code")
message(STATUS "LIS3MDL: Link with 'lis3mdl' target in CMakeLists.txt")
message(STATUS "LIS3MDL: Requires I2C or SPI peripheral driver")
