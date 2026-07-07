# ============================================================================
# U8g2 Graphics Library - CMake Integration
# ============================================================================
# This file automatically downloads and configures U8g2 for your project.
#
# What it does:
# 1. Downloads U8g2 graphics library from GitHub (only once, then cached)
# 2. Makes U8g2 available as a CMake target
# 3. Provides universal graphics support for displays
#
# For beginners:
# - U8g2 supports hundreds of displays (OLED, LCD, TFT)
# - Hardware-accelerated graphics operations
# - Small memory footprint, optimized for embedded
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download U8g2 from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  u8g2
  GIT_REPOSITORY https://github.com/olikraus/u8g2.git
  GIT_TAG master
)

# Actually download and make U8g2 available to CMake
FetchContent_MakeAvailable(u8g2)

# ----------------------------------------------------------------------------
# Create U8g2 library target
# ----------------------------------------------------------------------------
if(NOT TARGET u8g2)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(u8g2 SOURCE_DIR U8G2_SOURCE_DIR)

  # U8g2 has a specific structure - use the csrc directory
  set(U8G2_SRC_DIR "${U8G2_SOURCE_DIR}/csrc")

  # Find all source files in the csrc directory
  file(GLOB U8G2_SOURCES
    "${U8G2_SRC_DIR}/*.c"
  )

  add_library(u8g2 STATIC ${U8G2_SOURCES})

  target_include_directories(u8g2 INTERFACE
    ${U8G2_SRC_DIR}
  )

  # U8g2 configuration - minimal setup
  target_compile_definitions(u8g2 PUBLIC
    # Enable basic features
    U8G2_WITH_CLIP_WINDOW_SUPPORT=1
    U8G2_WITH_FONT_ROTATION=1
    # Disable unused features to save space
    U8G2_USE_DYNAMIC_ALLOC=0
    U8G2_WITH_INTERSECTION=0
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'u8g2' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "U8g2: Universal graphics library ready")
message(STATUS "U8g2: Include <u8g2.h> in your code")
message(STATUS "U8g2: Link with 'u8g2' target in CMakeLists.txt")
message(STATUS "U8g2: Supports 200+ displays, optimized for embedded")
