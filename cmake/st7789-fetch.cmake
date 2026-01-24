# ============================================================================
# ST7789 TFT Display Driver - CMake Integration
# ============================================================================
# This file automatically downloads and configures ST7789 driver for your project.
#
# What it does:
# 1. Downloads ST7789 TFT driver from GitHub (only once, then cached)
# 2. Makes ST7789 driver available as a CMake target
# 3. Provides IPS TFT display support
#
# For beginners:
# - ST7789 is popular for IPS TFT displays (1.3"-2.0" screens)
# - 240x240 resolution, SPI interface
# - Better color reproduction than TN panels
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download ST7789 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  st7789
  GIT_REPOSITORY https://github.com/martnak/STM32-ST7789.git
  GIT_TAG master
)

# Actually download and make ST7789 available to CMake
FetchContent_MakeAvailable(st7789)

# ----------------------------------------------------------------------------
# Create ST7789 library target
# ----------------------------------------------------------------------------
if(NOT TARGET st7789)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(st7789 SOURCE_DIR ST7789_SOURCE_DIR)

  # Find all source files in the driver
  file(GLOB ST7789_SOURCES
    "${ST7789_SOURCE_DIR}/*.c"
    "${ST7789_SOURCE_DIR}/*.cpp"
  )

  add_library(st7789 STATIC ${ST7789_SOURCES})

  target_include_directories(st7789 INTERFACE
    ${ST7789_SOURCE_DIR}
  )

  # Link against STM32 HAL (assumes stm32cubemx target exists)
  target_link_libraries(st7789 PUBLIC stm32cubemx)

  # ST7789 configuration
  target_compile_definitions(st7789 PUBLIC
    ST7789_SPI_PORT=hspi1  # Adjust based on your SPI port
    ST7789_RES_Pin=GPIO_PIN_7
    ST7789_RES_GPIO_Port=GPIOC
    ST7789_CS_Pin=GPIO_PIN_2
    ST7789_CS_GPIO_Port=GPIOD
    ST7789_DC_Pin=GPIO_PIN_13
    ST7789_DC_GPIO_Port=GPIOD
    ST7789_WIDTH=240
    ST7789_HEIGHT=240
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'st7789' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "ST7789: IPS TFT display driver ready")
message(STATUS "ST7789: Include <st7789.h> in your code")
message(STATUS "ST7789: Link with 'st7789' target in CMakeLists.txt")
message(STATUS "ST7789: 240x240 IPS display, SPI interface")
