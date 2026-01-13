# ============================================================================
# LittleFS - Local Integration
# ============================================================================
# Use this file when you have a local checkout of LittleFS inside your project
# (for example at ${CMAKE_SOURCE_DIR}/littlefs). It mirrors the behaviour of
# cmake/littlefs-fetch.cmake but uses the local source tree instead of
# downloading LittleFS via FetchContent.
#
# Usage:
# - Turn on the option `LITTLEFS_USE_LOCAL` in your top-level CMakeLists.txt
# - Optionally set `LITTLEFS_LOCAL_DIR` to point to your local littlefs directory
#   (default: ${CMAKE_SOURCE_DIR}/littlefs)
# ============================================================================

# Default path to local LittleFS (can be overridden by the user)
if(NOT DEFINED LITTLEFS_LOCAL_DIR)
  set(LITTLEFS_LOCAL_DIR ${CMAKE_SOURCE_DIR}/littlefs CACHE PATH "Path to local LittleFS source")
endif()

message(STATUS "LittleFS (local): looking for LittleFS at ${LITTLEFS_LOCAL_DIR}")

# Validate that the directory exists and appears to be LittleFS
if(NOT EXISTS "${LITTLEFS_LOCAL_DIR}/lfs.h")
  message(FATAL_ERROR "Local LittleFS not found at ${LITTLEFS_LOCAL_DIR}. Please set LITTLEFS_LOCAL_DIR to the path of LittleFS source containing lfs.h")
endif()

# ----------------------------------------------------------------------------
# Create LittleFS library target
# ----------------------------------------------------------------------------
if(NOT TARGET littlefs)
  add_library(littlefs INTERFACE)

  target_include_directories(littlefs INTERFACE
    ${LITTLEFS_LOCAL_DIR}
  )

  # Add compile definitions if needed
  target_compile_definitions(littlefs INTERFACE
    LFS_CONFIG=lfs_config.h  # Custom config can be provided
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'littlefs' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "LittleFS (local): Ready to use (header-only library)")
message(STATUS "LittleFS (local): Include <lfs.h> in your code")
message(STATUS "LittleFS (local): Link with 'littlefs' target in CMakeLists.txt")
