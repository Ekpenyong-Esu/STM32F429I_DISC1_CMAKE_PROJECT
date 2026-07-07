# ============================================================================
# ST7735 TFT Display Driver - CMake Integration
# ============================================================================
# This file automatically downloads and configures ST7735 driver for your project.
#
# What it does:
# 1. Downloads ST7735 TFT driver from GitHub (only once, then cached)
# 2. Makes ST7735 driver available as a CMake target
# 3. Provides 128x160 TFT display support
#
# For beginners:
# - ST7735 is popular for small TFT displays (1.8" screens)
# - 128x160 resolution, SPI interface
# - Great for battery-powered projects
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download ST7735 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  st7735
  GIT_REPOSITORY https://github.com/martnak/STM32-ST7735.git
  GIT_TAG master
)

# Actually download and make ST7735 available to CMake
FetchContent_MakeAvailable(st7735)

# ----------------------------------------------------------------------------
# Create ST7735 library target
# ----------------------------------------------------------------------------
if(NOT TARGET st7735)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(st7735 SOURCE_DIR ST7735_SOURCE_DIR)

  # Find all source files in the driver
  file(GLOB ST7735_SOURCES
    "${ST7735_SOURCE_DIR}/*.c"
    "${ST7735_SOURCE_DIR}/*.cpp"
  )

  add_library(st7735 STATIC ${ST7735_SOURCES})

  target_include_directories(st7735 INTERFACE
    ${ST7735_SOURCE_DIR}
  )

  # Link against STM32 HAL (assumes stm32cubemx target exists)
  target_link_libraries(st7735 PUBLIC stm32cubemx)

  # ST7735 configuration
  target_compile_definitions(st7735 PUBLIC
    ST7735_SPI_PORT=hspi1  # Adjust based on your SPI port
    ST7735_RES_Pin=GPIO_PIN_7
    ST7735_RES_GPIO_Port=GPIOC
    ST7735_CS_Pin=GPIO_PIN_2
    ST7735_CS_GPIO_Port=GPIOD
    ST7735_DC_Pin=GPIO_PIN_13
    ST7735_DC_GPIO_Port=GPIOD
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'st7735' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "ST7735: TFT display driver ready")
message(STATUS "ST7735: Include <st7735.h> in your code")
message(STATUS "ST7735: Link with 'st7735' target in CMakeLists.txt")
message(STATUS "ST7735: 128x160 resolution, SPI interface")
