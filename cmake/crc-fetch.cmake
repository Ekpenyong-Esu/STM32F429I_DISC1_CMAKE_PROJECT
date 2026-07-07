# ============================================================================
# CRC - Cyclic Redundancy Check - CMake Integration
# ============================================================================
# This file automatically downloads and configures CRC library.
#
# What it does:
# 1. Downloads CRC library from GitHub
# 2. Makes CRC available as a CMake target
# 3. Provides various CRC algorithms for data integrity
#
# For beginners:
# - CRC ensures data integrity in communication protocols
# - Essential for Modbus, CAN, RS-485, file transfers
# - Common standards: CRC-8, CRC-16, CRC-32
# - Hardware accelerated on many MCUs
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download CRC library from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  crc
  GIT_REPOSITORY https://github.com/FlyLu/Embedded-CRC-Library.git
  GIT_TAG master
)

# Actually download and make CRC available to CMake
FetchContent_MakeAvailable(crc)

# ----------------------------------------------------------------------------
# Create CRC library target
# ----------------------------------------------------------------------------
if(NOT TARGET crc)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(crc SOURCE_DIR CRC_SOURCE_DIR)

  add_library(crc STATIC
    ${CRC_SOURCE_DIR}/crc.c
  )

  target_include_directories(crc INTERFACE
    ${CRC_SOURCE_DIR}
  )

  target_compile_definitions(crc PUBLIC
    # CRC configuration
    CRC_USE_LOOKUP_TABLE=1    # Use lookup tables for speed
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'crc' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "CRC: Cyclic Redundancy Check library ready")
message(STATUS "CRC: Include <crc.h> in your code")
message(STATUS "CRC: Link with 'crc' target in CMakeLists.txt")
message(STATUS "CRC: Supports CRC-8, CRC-16, CRC-32 algorithms")
