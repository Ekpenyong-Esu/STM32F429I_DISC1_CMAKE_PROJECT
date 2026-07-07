# ============================================================================
# LittleFS - Embedded Filesystem - CMake Integration
# ============================================================================
# This file automatically downloads and configures LittleFS for your project.
#
# What it does:
# 1. Downloads LittleFS from GitHub (only once, then cached)
# 2. Makes LittleFS available as a CMake target
# 3. Provides a lightweight filesystem for flash storage
#
# For beginners:
# - LittleFS provides wear-leveling and power-loss recovery
# - Perfect for data logging, configuration storage, firmware updates
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download LittleFS from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  littlefs
  GIT_REPOSITORY https://github.com/littlefs-project/littlefs.git
  GIT_TAG v2.9.3
)

# Actually download and make LittleFS available to CMake
FetchContent_MakeAvailable(littlefs)

# ----------------------------------------------------------------------------
# Create LittleFS library target
# ----------------------------------------------------------------------------
# LittleFS is header-only, so we create an interface library
if(NOT TARGET littlefs)
  add_library(littlefs INTERFACE)

  # Get the source directory from FetchContent
  FetchContent_GetProperties(littlefs SOURCE_DIR LITTLEFS_SOURCE_DIR)

  target_include_directories(littlefs INTERFACE
    ${LITTLEFS_SOURCE_DIR}
  )

  # Add compile definitions if needed
  target_compile_definitions(littlefs INTERFACE
    LFS_CONFIG=lfs_config.h  # Custom config can be provided
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'littlefs' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "LittleFS: Ready to use (header-only library)")
message(STATUS "LittleFS: Include <lfs.h> in your code")
message(STATUS "LittleFS: Link with 'littlefs' target in CMakeLists.txt")
