# ============================================================================
# cJSON - JSON Parser - CMake Integration
# ============================================================================
# This file automatically downloads and configures cJSON library.
#
# What it does:
# 1. Downloads cJSON from GitHub (Dave Gamble's cJSON)
# 2. Makes cJSON available as a CMake target
# 3. Provides JSON parsing and generation capabilities
#
# For beginners:
# - cJSON is a lightweight JSON parser in C
# - Essential for IoT, configuration files, data exchange
# - Used in REST APIs, MQTT payloads, configuration storage
# - Memory efficient, no dependencies
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download cJSON from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  cjson
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG v1.7.16
)

# Actually download and make cJSON available to CMake
FetchContent_MakeAvailable(cjson)

# ----------------------------------------------------------------------------
# Create cJSON library target
# ----------------------------------------------------------------------------
if(NOT TARGET cjson)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(cjson SOURCE_DIR CJSON_SOURCE_DIR)

  add_library(cjson STATIC
    ${CJSON_SOURCE_DIR}/cJSON.c
  )

  target_include_directories(cjson INTERFACE
    ${CJSON_SOURCE_DIR}
  )

  target_compile_definitions(cjson PUBLIC
    # cJSON configuration
    cJSON_API_VISIBILITY=    # No visibility restrictions
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'cjson' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "cJSON: JSON parser library ready")
message(STATUS "cJSON: Include <cJSON.h> in your code")
message(STATUS "cJSON: Link with 'cjson' target in CMakeLists.txt")
message(STATUS "cJSON: Use for JSON parsing, configuration, IoT data")
