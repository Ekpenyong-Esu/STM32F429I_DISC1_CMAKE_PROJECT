# ============================================================================
# XPT2046 Touchscreen Controller - CMake Integration
# ============================================================================
# This file automatically downloads and configures XPT2046 driver for your project.
#
# What it does:
# 1. Downloads XPT2046 touchscreen driver from GitHub (only once, then cached)
# 2. Makes XPT2046 driver available as a CMake target
# 3. Provides resistive touchscreen support
#
# For beginners:
# - XPT2046 is the most popular resistive touchscreen controller
# - Works with most TFT displays, SPI interface
# - Perfect companion for TFT projects needing touch input
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download XPT2046 driver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  xpt2046
  GIT_REPOSITORY https://github.com/martnak/STM32-XPT2046.git
  GIT_TAG master
)

# Actually download and make XPT2046 available to CMake
FetchContent_MakeAvailable(xpt2046)

# ----------------------------------------------------------------------------
# Create XPT2046 library target
# ----------------------------------------------------------------------------
if(NOT TARGET xpt2046)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(xpt2046 SOURCE_DIR XPT2046_SOURCE_DIR)

  # Find all source files in the driver
  file(GLOB XPT2046_SOURCES
    "${XPT2046_SOURCE_DIR}/*.c"
    "${XPT2046_SOURCE_DIR}/*.cpp"
  )

  add_library(xpt2046 STATIC ${XPT2046_SOURCES})

  target_include_directories(xpt2046 INTERFACE
    ${XPT2046_SOURCE_DIR}
  )

  # Link against STM32 HAL (assumes stm32cubemx target exists)
  target_link_libraries(xpt2046 PUBLIC stm32cubemx)

  # XPT2046 configuration
  target_compile_definitions(xpt2046 PUBLIC
    XPT2046_SPI_PORT=hspi1  # Adjust based on your SPI port
    XPT2046_CS_Pin=GPIO_PIN_4
    XPT2046_CS_GPIO_Port=GPIOB
    XPT2046_IRQ_Pin=GPIO_PIN_5
    XPT2046_IRQ_GPIO_Port=GPIOB
    XPT2046_WIDTH=320   # Touch panel width
    XPT2046_HEIGHT=240  # Touch panel height
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'xpt2046' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "XPT2046: Touchscreen controller ready")
message(STATUS "XPT2046: Include <xpt2046.h> in your code")
message(STATUS "XPT2046: Link with 'xpt2046' target in CMakeLists.txt")
message(STATUS "XPT2046: Resistive touchscreen, SPI interface")
