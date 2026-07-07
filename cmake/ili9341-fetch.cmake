# ============================================================================
# ILI9341 TFT Display Driver - CMake Integration
# ============================================================================
# This file automatically downloads and configures ILI9341 driver for your project.
#
# What it does:
# 1. Downloads ILI9341 TFT driver from GitHub (only once, then cached)
# 2. Makes ILI9341 driver available as a CMake target
# 3. Provides 320x240 TFT display support
#
# For beginners:
# - ILI9341 is one of the most popular TFT controllers
# - 320x240 resolution, SPI interface
# - Perfect for STM32 projects with touch screens
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download ILI9341 driver from GitHub
# ----------------------------------------------------------------------------
# Popular ILI9341 driver with STM32 HAL support
FetchContent_Declare(
  ili9341
  GIT_REPOSITORY https://github.com/martnak/ILI9341-STM32-HAL.git
  GIT_TAG master
)

# Actually download and make ILI9341 available to CMake
FetchContent_MakeAvailable(ili9341)

# ----------------------------------------------------------------------------
# Create ILI9341 library target
# ----------------------------------------------------------------------------
if(NOT TARGET ili9341)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(ili9341 SOURCE_DIR ILI9341_SOURCE_DIR)

  # Find all source files in the driver
  file(GLOB ILI9341_SOURCES
    "${ILI9341_SOURCE_DIR}/*.c"
    "${ILI9341_SOURCE_DIR}/*.cpp"
  )

  add_library(ili9341 STATIC ${ILI9341_SOURCES})

  target_include_directories(ili9341 INTERFACE
    ${ILI9341_SOURCE_DIR}
  )

  # Link against STM32 HAL (assumes stm32cubemx target exists)
  target_link_libraries(ili9341 PUBLIC stm32cubemx)

  # ILI9341 configuration
  target_compile_definitions(ili9341 PUBLIC
    ILI9341_SPI_PORT=hspi5  # Adjust based on your SPI port
    ILI9341_RES_Pin=GPIO_PIN_7
    ILI9341_RES_GPIO_Port=GPIOC
    ILI9341_CS_Pin=GPIO_PIN_2
    ILI9341_CS_GPIO_Port=GPIOD
    ILI9341_DC_Pin=GPIO_PIN_13
    ILI9341_DC_GPIO_Port=GPIOD
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'ili9341' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "ILI9341: TFT display driver ready")
message(STATUS "ILI9341: Include <ili9341.h> in your code")
message(STATUS "ILI9341: Link with 'ili9341' target in CMakeLists.txt")
message(STATUS "ILI9341: 320x240 resolution, SPI interface")
