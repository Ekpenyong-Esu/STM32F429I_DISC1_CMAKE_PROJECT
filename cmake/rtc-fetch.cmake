# ============================================================================
# STM32duino RTC - Real-Time Clock - CMake Integration
# ============================================================================
# This file automatically downloads and configures RTC library.
#
# What it does:
# 1. Downloads RTC library from GitHub (STM32duino library)
# 2. Makes RTC available as a CMake target
# 3. Provides real-time clock functionality
#
# For beginners:
# - RTC functionality for STM32
# - Timekeeping, alarms, calendar functions
# - Essential for data logging with timestamps
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download RTC library from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  rtc
  GIT_REPOSITORY https://github.com/stm32duino/STM32RTC.git
  GIT_TAG main
)

# Actually download and make RTC available to CMake
FetchContent_MakeAvailable(rtc)

# ----------------------------------------------------------------------------
# Create RTC library target
# ----------------------------------------------------------------------------
if(NOT TARGET rtc)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(rtc SOURCE_DIR RTC_SOURCE_DIR)

  add_library(rtc STATIC
    ${RTC_SOURCE_DIR}/STM32RTC.cpp
  )

  target_include_directories(rtc INTERFACE
    ${RTC_SOURCE_DIR}
  )

  target_compile_definitions(rtc PUBLIC
    # RTC specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'rtc' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "RTC: Real-time clock library ready")
message(STATUS "RTC: Include <STM32RTC.h> in your code")
message(STATUS "RTC: Link with 'rtc' target in CMakeLists.txt")
message(STATUS "RTC: Requires RTC peripheral driver")
