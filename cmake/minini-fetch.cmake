# ============================================================================
# minIni - INI File Parser - CMake Integration
# ============================================================================
# This file automatically downloads and configures minIni for your project.
#
# What it does:
# 1. Downloads minIni from GitHub (only once, then cached)
# 2. Makes minIni available as a CMake target
# 3. Provides INI file parsing for configuration storage
#
# For beginners:
# - minIni is a minimal INI file parser for embedded systems
# - Perfect for storing sensor calibration data, system settings
# - Works with LittleFS or any filesystem
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download minIni from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  minini
  GIT_REPOSITORY https://github.com/compuphase/minIni.git
  GIT_TAG v1.5
)

# Actually download and make minIni available to CMake
FetchContent_MakeAvailable(minini)

# ----------------------------------------------------------------------------
# Create minIni library target
# ----------------------------------------------------------------------------
if(NOT TARGET minini)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(minini SOURCE_DIR MININI_SOURCE_DIR)

  add_library(minini STATIC
    ${MININI_SOURCE_DIR}/minIni.c
    ${MININI_SOURCE_DIR}/minGlue.c
  )

  target_include_directories(minini INTERFACE
    ${MININI_SOURCE_DIR}
  )

  # minIni configuration - adjust based on your filesystem
  target_compile_definitions(minini PUBLIC
    INI_FILETYPE=SFLASH   # Use SPI Flash filesystem
    INI_READONLY=0       # Allow writing config files
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'minini' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "minIni: INI file parser ready")
message(STATUS "minIni: Include <minIni.h> in your code")
message(STATUS "minIni: Link with 'minini' target in CMakeLists.txt")
message(STATUS "minIni: Use ini_gets(), ini_puts() for config files")
