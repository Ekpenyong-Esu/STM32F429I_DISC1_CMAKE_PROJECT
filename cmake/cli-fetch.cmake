# ============================================================================
# CLI - Command Line Interface - CMake Integration
# ============================================================================
# This file automatically downloads and configures CLI library.
#
# What it does:
# 1. Downloads CLI library from GitHub
# 2. Makes CLI available as a CMake target
# 3. Provides command-line interface for debugging/configuration
#
# For beginners:
# - CLI enables serial command interface for your embedded device
# - Essential for debugging, configuration, testing
# - Commands like "help", "status", "config", custom commands
# - Works with UART, USB CDC, or any character stream
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download CLI library from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  cli
  GIT_REPOSITORY https://github.com/funbiscuit/embedded-cli.git
  GIT_TAG v1.3.0
)

# Actually download and make CLI available to CMake
FetchContent_MakeAvailable(cli)

# ----------------------------------------------------------------------------
# Create CLI library target
# ----------------------------------------------------------------------------
if(NOT TARGET cli)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(cli SOURCE_DIR CLI_SOURCE_DIR)

  add_library(cli STATIC
    ${CLI_SOURCE_DIR}/src/embedded_cli.c
  )

  target_include_directories(cli INTERFACE
    ${CLI_SOURCE_DIR}/include
  )

  target_compile_definitions(cli PUBLIC
    # CLI configuration
    EMBEDDED_CLI_MAX_COMMAND_LENGTH=64
    EMBEDDED_CLI_MAX_BINDING_COUNT=16
    EMBEDDED_CLI_MAX_HISTORY_COUNT=8
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'cli' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "CLI: Command Line Interface library ready")
message(STATUS "CLI: Include <embedded_cli.h> in your code")
message(STATUS "CLI: Link with 'cli' target in CMakeLists.txt")
message(STATUS "CLI: Requires character I/O functions (UART, USB, etc.)")
