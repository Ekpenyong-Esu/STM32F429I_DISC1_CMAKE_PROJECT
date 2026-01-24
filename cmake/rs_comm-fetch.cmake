# ============================================================================
# RS-485 - RS-485 Serial Communication - CMake Integration
# ============================================================================
# This file automatically downloads and configures RS-485 communication library.
#
# What it does:
# 1. Downloads RS-485 communication library from GitHub
# 2. Makes RS-485 available as a CMake target
# 3. Provides RS-485 serial communication protocol
#
# For beginners:
# - RS-485: Multi-drop serial communication (up to 1200m)
# - Industrial standard for multi-device networks
# - Perfect for industrial sensors, PLCs, building automation
# - Requires UART peripheral driver (you already have this!)
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download RS communication library from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  rs_comm
  GIT_REPOSITORY https://github.com/davidfascio/rs485cominterface.git
  GIT_TAG master
)

# Actually download and make RS communication available to CMake
FetchContent_MakeAvailable(rs_comm)

# ----------------------------------------------------------------------------
# Create RS communication library target
# ----------------------------------------------------------------------------
if(NOT TARGET rs_comm)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(rs_comm SOURCE_DIR RS_COMM_SOURCE_DIR)

  add_library(rs_comm STATIC
    ${RS_COMM_SOURCE_DIR}/rs485.c
  )

  target_include_directories(rs_comm INTERFACE
    ${RS_COMM_SOURCE_DIR}
  )

  target_compile_definitions(rs_comm PUBLIC
    # RS communication specific configuration
    RS_COMM_MAX_DEVICES=32    # Maximum devices on RS-485 bus
    RS_COMM_TIMEOUT_MS=1000   # Communication timeout
  )

  # Link to your existing UART library
  target_link_libraries(rs_comm PUBLIC
    uart  # Your existing UART peripheral driver
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'rs_comm' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "RS Comm: RS-485 communication interface ready")
message(STATUS "RS Comm: Include <rs485.h> in your code")
message(STATUS "RS Comm: Link with 'rs_comm' target in CMakeLists.txt")
message(STATUS "RS Comm: Requires UART peripheral driver (already available)")
