# ============================================================================
# ILI9488 TFT Display Driver - CMake Integration
# ============================================================================
# This file automatically downloads and configures ILI9488 driver for your project.
#
# What it does:
# 1. Downloads ILI9488 TFT driver from GitHub (only once, then cached)
# 2. Makes ILI9488 driver available as a CMake target
# 3. Provides high-resolution TFT display support
#
# For beginners:
# - ILI9488 supports higher resolutions (320x480, 480x320)
# - 18-bit color depth, SPI/parallel interface
# - Great for applications needing more screen real estate
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download ILI9488 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  ili9488
  GIT_REPOSITORY https://github.com/martnak/STM32-ILI9488.git
  GIT_TAG master
)

# Actually download and make ILI9488 available to CMake
FetchContent_MakeAvailable(ili9488)

# ----------------------------------------------------------------------------
# Create ILI9488 library target
# ----------------------------------------------------------------------------
if(NOT TARGET ili9488)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(ili9488 SOURCE_DIR ILI9488_SOURCE_DIR)

  # Find all source files in the driver
  file(GLOB ILI9488_SOURCES
    "${ILI9488_SOURCE_DIR}/*.c"
    "${ILI9488_SOURCE_DIR}/*.cpp"
  )

  add_library(ili9488 STATIC ${ILI9488_SOURCES})

  target_include_directories(ili9488 INTERFACE
    ${ILI9488_SOURCE_DIR}
  )

  # Link against STM32 HAL (assumes stm32cubemx target exists)
  target_link_libraries(ili9488 PUBLIC stm32cubemx)

  # ILI9488 configuration
  target_compile_definitions(ili9488 PUBLIC
    ILI9488_SPI_PORT=hspi5  # Adjust based on your SPI port
    ILI9488_RES_Pin=GPIO_PIN_7
    ILI9488_RES_GPIO_Port=GPIOC
    ILI9488_CS_Pin=GPIO_PIN_2
    ILI9488_CS_GPIO_Port=GPIOD
    ILI9488_DC_Pin=GPIO_PIN_13
    ILI9488_DC_GPIO_Port=GPIOD
    ILI9488_WIDTH=320
    ILI9488_HEIGHT=480
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'ili9488' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "ILI9488: High-resolution TFT display driver ready")
message(STATUS "ILI9488: Include <ili9488.h> in your code")
message(STATUS "ILI9488: Link with 'ili9488' target in CMakeLists.txt")
message(STATUS "ILI9488: 320x480 resolution, SPI interface")
