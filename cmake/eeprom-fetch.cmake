# ============================================================================
# STM32duino EEPROM - EEPROM Emulation - CMake Integration
# ============================================================================
# This file automatically downloads and configures EEPROM emulation library.
#
# What it does:
# 1. Downloads EEPROM emulation from GitHub (STM32duino library)
# 2. Makes EEPROM available as a CMake target
# 3. Provides EEPROM-like interface using flash memory
#
# For beginners:
# - EEPROM emulation for STM32 (which lacks dedicated EEPROM)
# - Uses flash memory with wear-leveling
# - Perfect for storing configuration, calibration data
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download EEPROM emulation from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  eeprom
  GIT_REPOSITORY https://github.com/stm32duino/STM32EEPROM.git
  GIT_TAG main
)

# Actually download and make EEPROM available to CMake
FetchContent_MakeAvailable(eeprom)

# ----------------------------------------------------------------------------
# Create EEPROM library target
# ----------------------------------------------------------------------------
if(NOT TARGET eeprom)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(eeprom SOURCE_DIR EEPROM_SOURCE_DIR)

  add_library(eeprom STATIC
    ${EEPROM_SOURCE_DIR}/EEPROM.cpp
  )

  target_include_directories(eeprom INTERFACE
    ${EEPROM_SOURCE_DIR}
  )

  target_compile_definitions(eeprom PUBLIC
    # EEPROM specific configuration
    EEPROM_PAGE_SIZE=0x800  # 2KB pages (adjust for your MCU)
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'eeprom' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "EEPROM: Flash-based EEPROM emulation ready")
message(STATUS "EEPROM: Include <EEPROM.h> in your code")
message(STATUS "EEPROM: Link with 'eeprom' target in CMakeLists.txt")
message(STATUS "EEPROM: Uses flash memory for persistent storage")
