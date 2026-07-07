# ============================================================================
# SSD1306 OLED Display Driver - CMake Integration
# ============================================================================
# This file automatically downloads and configures SSD1306 driver for your project.
#
# What it does:
# 1. Downloads SSD1306 OLED driver from GitHub (only once, then cached)
# 2. Makes SSD1306 driver available as a CMake target
# 3. Provides monochrome OLED display support
#
# For beginners:
# - SSD1306 is the most popular OLED controller
# - 128x64 monochrome display, I2C/SPI interface
# - Low power consumption, great for battery projects
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download SSD1306 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  ssd1306
  GIT_REPOSITORY https://github.com/afiskon/stm32-ssd1306.git
  GIT_TAG master
)

# Actually download and make SSD1306 available to CMake
FetchContent_MakeAvailable(ssd1306)

# ----------------------------------------------------------------------------
# Create SSD1306 library target
# ----------------------------------------------------------------------------
if(NOT TARGET ssd1306)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(ssd1306 SOURCE_DIR SSD1306_SOURCE_DIR)

  # Find all source files in the driver
  file(GLOB SSD1306_SOURCES
    "${SSD1306_SOURCE_DIR}/*.c"
    "${SSD1306_SOURCE_DIR}/*.cpp"
  )

  add_library(ssd1306 STATIC ${SSD1306_SOURCES})

  target_include_directories(ssd1306 INTERFACE
    ${SSD1306_SOURCE_DIR}
  )

  # Link against STM32 HAL (assumes stm32cubemx target exists)
  target_link_libraries(ssd1306 PUBLIC stm32cubemx)

  # SSD1306 configuration - I2C interface (most common)
  target_compile_definitions(ssd1306 PUBLIC
    SSD1306_I2C_PORT=hi2c1  # Adjust based on your I2C port
    SSD1306_WIDTH=128
    SSD1306_HEIGHT=64
    SSD1306_I2C_ADDR=0x3C  # Default I2C address
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'ssd1306' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "SSD1306: OLED display driver ready")
message(STATUS "SSD1306: Include <ssd1306.h> in your code")
message(STATUS "SSD1306: Link with 'ssd1306' target in CMakeLists.txt")
message(STATUS "SSD1306: 128x64 monochrome, I2C/SPI interface")
