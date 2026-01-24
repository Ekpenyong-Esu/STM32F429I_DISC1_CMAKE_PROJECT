# ============================================================================
# Unity - Unit Testing Framework - CMake Integration
# ============================================================================
# This file automatically downloads and configures Unity for your project.
#
# What it does:
# 1. Downloads Unity testing framework from GitHub
# 2. Makes Unity available as a CMake target
# 3. Enables unit testing for your embedded drivers
#
# For beginners:
# - Unity is a lightweight C testing framework
# - Perfect for testing peripheral drivers (ADC, GPIO, sensors)
# - Run tests on host machine or embedded target
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download Unity from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG v2.6.0
)

# Actually download and make Unity available to CMake
FetchContent_MakeAvailable(unity)

# ----------------------------------------------------------------------------
# Create Unity library target
# ----------------------------------------------------------------------------
if(NOT TARGET unity)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(unity SOURCE_DIR UNITY_SOURCE_DIR)

  # Unity has its own CMakeLists.txt, but we can also create our own
  add_library(unity STATIC
    ${UNITY_SOURCE_DIR}/src/unity.c
  )

  target_include_directories(unity INTERFACE
    ${UNITY_SOURCE_DIR}/src
  )

  # Unity needs to know the target platform
  # For embedded testing, you might want to define UNITY_EXCLUDE_FLOAT
  target_compile_definitions(unity PUBLIC
    UNITY_INCLUDE_CONFIG_H  # Use custom config if provided
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'unity' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "Unity: Testing framework ready")
message(STATUS "Unity: Include <unity.h> in test files")
message(STATUS "Unity: Link with 'unity' target in test CMakeLists.txt")
message(STATUS "Unity: Use RUN_TEST(func) to run individual tests")
