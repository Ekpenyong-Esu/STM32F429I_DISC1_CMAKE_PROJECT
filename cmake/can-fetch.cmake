# ============================================================================
# STM32duino CAN - CAN Bus Communication - CMake Integration
# ============================================================================
# This file automatically downloads and configures CAN bus library.
#
# What it does:
# 1. Downloads CAN library from GitHub (STM32duino library)
# 2. Makes CAN available as a CMake target
# 3. Provides CAN bus communication capabilities
#
# For beginners:
# - CAN bus communication for automotive/industrial applications
# - High reliability, multi-master network
# - Perfect for vehicle networks, industrial control
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download CAN library from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  can
  GIT_REPOSITORY https://github.com/stm32duino/STM32_CAN.git
  GIT_TAG main
)

# Actually download and make CAN available to CMake
FetchContent_MakeAvailable(can)

# ----------------------------------------------------------------------------
# Create CAN library target
# ----------------------------------------------------------------------------
if(NOT TARGET can)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(can SOURCE_DIR CAN_SOURCE_DIR)

  add_library(can STATIC
    ${CAN_SOURCE_DIR}/STM32_CAN.cpp
  )

  target_include_directories(can INTERFACE
    ${CAN_SOURCE_DIR}
  )

  target_compile_definitions(can PUBLIC
    # CAN specific configuration
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'can' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "CAN: CAN bus communication library ready")
message(STATUS "CAN: Include <STM32_CAN.h> in your code")
message(STATUS "CAN: Link with 'can' target in CMakeLists.txt")
message(STATUS "CAN: Requires CAN peripheral driver")
