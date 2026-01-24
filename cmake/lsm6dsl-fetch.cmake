# ============================================================================
# STM32duino LSM6DSL - LSM6DSL IMU (Accelerometer/Gyroscope) - CMake Integration
# ============================================================================
# This file automatically downloads and configures LSM6DSL IMU driver.
#
# What it does:
# 1. Downloads LSM6DSL driver from GitHub (STM32duino library)
# 2. Makes LSM6DSL available as a CMake target
# 3. Provides 3-axis accelerometer and 3-axis gyroscope data
#
# For beginners:
# - LSM6DSL is a 6-axis IMU (Inertial Measurement Unit)
# - I2C/SPI interface, motion detection, pedometer
# - Essential for robotics, drones, motion tracking
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download LSM6DSL driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  lsm6dsl
  GIT_REPOSITORY https://github.com/stm32duino/LSM6DSL.git
  GIT_TAG main
)

# Actually download and make LSM6DSL available to CMake
FetchContent_MakeAvailable(lsm6dsl)

# ----------------------------------------------------------------------------
# Create LSM6DSL library target
# ----------------------------------------------------------------------------
if(NOT TARGET lsm6dsl)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(lsm6dsl SOURCE_DIR LSM6DSL_SOURCE_DIR)

  add_library(lsm6dsl STATIC
    ${LSM6DSL_SOURCE_DIR}/LSM6DSL.cpp
  )

  target_include_directories(lsm6dsl INTERFACE
    ${LSM6DSL_SOURCE_DIR}
  )

  target_compile_definitions(lsm6dsl PUBLIC
    # LSM6DSL specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'lsm6dsl' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "LSM6DSL: 6-axis IMU sensor driver ready")
message(STATUS "LSM6DSL: Include <LSM6DSL.h> in your code")
message(STATUS "LSM6DSL: Link with 'lsm6dsl' target in CMakeLists.txt")
message(STATUS "LSM6DSL: Requires I2C or SPI peripheral driver")
