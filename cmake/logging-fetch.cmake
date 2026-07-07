# ============================================================================
# ulog - Lightweight Embedded Logging - CMake Integration
# ============================================================================
# This file automatically downloads and configures ulog library.
#
# What it does:
# 1. Downloads ulog (lightweight embedded logging) from GitHub
# 2. Makes ulog available as a CMake target
# 3. Provides simple, efficient logging for embedded systems
#
# For beginners:
# - ulog is a minimal logging library for embedded systems
# - Very small footprint, perfect for resource-constrained devices
# - Supports multiple output methods (UART, RTT, memory)
# - Log levels: ERROR, WARN, INFO, DEBUG, TRACE
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download logging library from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  logging
  GIT_REPOSITORY https://github.com/rdpoor/ulog.git
  GIT_TAG master
)

# Actually download and make logging available to CMake
FetchContent_MakeAvailable(logging)

# ----------------------------------------------------------------------------
# Create logging library target
# ----------------------------------------------------------------------------
if(NOT TARGET logging)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(logging SOURCE_DIR LOGGING_SOURCE_DIR)

  add_library(logging STATIC
    ${LOGGING_SOURCE_DIR}/src/ulog.c
  )

  target_include_directories(logging INTERFACE
    ${LOGGING_SOURCE_DIR}/include
  )

  target_compile_definitions(logging PUBLIC
    # ulog configuration - adjust based on your needs
    ULOG_ENABLED=1                    # Enable logging
    ULOG_MAX_LEVEL=ULOG_INFO          # Default log level
    ULOG_BUFFER_SIZE=128              # Log message buffer size
    ULOG_TIMESTAMP=1                  # Include timestamps
  )

  # Link to printf for formatted output
  target_link_libraries(logging PUBLIC
    printf
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'logging' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "Logging: ulog embedded logging framework ready")
message(STATUS "Logging: Include <ulog.h> in your code")
message(STATUS "Logging: Link with 'logging' target in CMakeLists.txt")
message(STATUS "Logging: Configure output method (UART, RTT, etc.) in your code")
