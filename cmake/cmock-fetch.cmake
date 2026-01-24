# ============================================================================
# CMock - Mock Framework - CMake Integration
# ============================================================================
# This file automatically downloads and configures CMock for your project.
#
# What it does:
# 1. Downloads CMock from GitHub (only once, then cached)
# 2. Makes CMock available as a CMake target
# 3. Enables mocking for unit testing embedded drivers
#
# For beginners:
# - CMock generates mock functions for testing
# - Perfect for testing drivers that depend on HAL functions
# - Works with Unity testing framework
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download CMock from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  cmock
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/CMock.git
  GIT_TAG v2.6.0
)

# Actually download and make CMock available to CMake
FetchContent_MakeAvailable(cmock)

# ----------------------------------------------------------------------------
# Create CMock library target
# ----------------------------------------------------------------------------
if(NOT TARGET cmock)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(cmock SOURCE_DIR CMOCK_SOURCE_DIR)

  add_library(cmock STATIC
    ${CMOCK_SOURCE_DIR}/src/cmock.c
  )

  target_include_directories(cmock INTERFACE
    ${CMOCK_SOURCE_DIR}/src
  )

  # CMock needs Unity for testing
  # Make sure to also include unity-fetch.cmake if using CMock
  target_link_libraries(cmock PUBLIC unity)

  # CMock configuration
  target_compile_definitions(cmock PUBLIC
    CMOCK_MEM_SIZE=32768  # Memory size for mocks (adjust as needed)
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'cmock' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "CMock: Mock framework ready")
message(STATUS "CMock: Include <cmock.h> in test files")
message(STATUS "CMock: Link with 'cmock' target in test CMakeLists.txt")
message(STATUS "CMock: Use cmock_create() to generate mocks from headers")
