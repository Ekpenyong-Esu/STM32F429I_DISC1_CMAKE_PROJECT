# ==============================================================================
# STM32F429I-Discovery CMake Configuration
# ==============================================================================
# Main build configuration for the Sensor Console embedded project.
# This file orchestrates the build of STM32 HAL, FreeRTOS, LVGL, and
# peripheral drivers for the STM32F429I-DISC1 development board.
#
# For Beginners:
# - This file is the entry point for the entire build system
# - It coordinates STM32CubeMX generated code, peripherals, and libraries
# - Modify carefully - incorrect changes can break the build
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# ------------------------------------------------------------------------------
# Compiler Settings - C Language Standard
# ------------------------------------------------------------------------------
# Configure C language standard before project() command
set(CMAKE_C_STANDARD 11)              # Use C11 standard (modern C features)
set(CMAKE_C_STANDARD_REQUIRED ON)     # Enforce C11, fail if not available
set(CMAKE_C_EXTENSIONS ON)            # Allow GNU extensions (needed for embedded)

# ------------------------------------------------------------------------------
# Build Configuration
# ------------------------------------------------------------------------------
# Set build type if not specified (Debug includes debugging symbols)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# ------------------------------------------------------------------------------
# Project Definition
# ------------------------------------------------------------------------------
set(CMAKE_PROJECT_NAME Sensor_Console)

# ------------------------------------------------------------------------------
# Toolchain Configuration
# ------------------------------------------------------------------------------
# Load ARM GCC cross-compiler toolchain for STM32 (must be before project())
include("cmake/gcc-arm-none-eabi.cmake")

# Generate compile_commands.json for IDE tools (clangd, IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# ------------------------------------------------------------------------------
# Project Declaration
# ------------------------------------------------------------------------------
project(${CMAKE_PROJECT_NAME})
message(STATUS "Project: ${CMAKE_PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Enable language support for C and Assembly (needed for startup files)
enable_language(C ASM)

# ------------------------------------------------------------------------------
# Create Executable Target
# ------------------------------------------------------------------------------
# Create the main executable - sources will be added below
add_executable(${CMAKE_PROJECT_NAME})

# ------------------------------------------------------------------------------
# STM32CubeMX Generated Code
# ------------------------------------------------------------------------------
# Include HAL drivers, middleware, and application sources from CubeMX
# This creates the 'stm32cubemx' library target
add_subdirectory(cmake/stm32cubemx)

# ------------------------------------------------------------------------------
# LVGL Graphics Library
# ------------------------------------------------------------------------------
# Automatically fetch and configure LVGL v8.3.11 from GitHub
# See cmake/lvgl.cmake for details
include("cmake/lvgl.cmake")

# ------------------------------------------------------------------------------
# Additional Link Directories (Currently None)
# ------------------------------------------------------------------------------
# Uncomment to add custom library search paths if needed
# target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
#     ${CMAKE_SOURCE_DIR}/lib
# )

# ------------------------------------------------------------------------------
# User Application Sources (Add Your Main Code Here)
# ------------------------------------------------------------------------------
# Add your custom application .c files that aren't in Peripherals/
# Example: target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/my_app.c)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user application sources here
    # Example: ${CMAKE_SOURCE_DIR}/App/my_module.c
)





# ------------------------------------------------------------------------------
# Compile Definitions (Preprocessor Macros)
# ------------------------------------------------------------------------------
# Add preprocessor defines for conditional compilation
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # LVGL configuration: Use simplified include paths
    LV_CONF_INCLUDE_SIMPLE

    # Add your custom defines here:
    # MY_CUSTOM_FEATURE=1
    # APP_VERSION_MAJOR=1
)

# ------------------------------------------------------------------------------
# Link Libraries to Executable
# ------------------------------------------------------------------------------
# Link all required libraries to the final executable
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx      # STM32 HAL, middleware (FreeRTOS, USB), startup code
    ${LVGL_TARGET}   # LVGL graphics library (auto-resolved target name)

    # Add additional libraries here:
    # my_custom_lib
    # m  # Math library (if using floating point math functions)
)

# ------------------------------------------------------------------------------
# Peripheral Drivers - Automatic Discovery
# ------------------------------------------------------------------------------
# This section automatically includes all peripheral drivers from Peripherals/
# Each peripheral folder (ADC, GPIO, LVGL, etc.) is added to include paths

# Step 1: Find all peripheral directories
file(GLOB PERIPHERAL_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/*")

# Step 2: Add each peripheral folder to include paths
foreach(dir ${PERIPHERAL_DIRS})
    if(IS_DIRECTORY ${dir})
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${dir})
    endif()
endforeach()

# Step 3: Find all .c source files in peripheral subdirectories
file(GLOB_RECURSE PERIPHERALS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/*/*.c"
)

# Step 4: Add peripheral sources to the executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PERIPHERALS_SOURCES})

message(STATUS "Found ${CMAKE_MATCH_COUNT} peripheral directories")
message(STATUS "Added peripheral sources from: Peripherals/*/")

# ------------------------------------------------------------------------------
# Optional: Example Projects (Currently Disabled)
# ------------------------------------------------------------------------------
# Uncomment to include example code from Examples/ directory
# file(GLOB EXAMPLE_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Examples/*")
# foreach(dir ${EXAMPLE_DIRS})
#     if(IS_DIRECTORY ${dir})
#         target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${dir})
#     endif()
# endforeach()
#
# file(GLOB_RECURSE EXAMPLES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Examples/*/*.c")
# target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${EXAMPLES_SOURCES})

# ------------------------------------------------------------------------------
# Optional: Copy compile_commands.json to Root (For Better IDE Support)
# ------------------------------------------------------------------------------
# Uncomment to auto-copy compile_commands.json after each build
# This helps IDE tools find the file without configuring build directory
# add_custom_command(
#     TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${CMAKE_BINARY_DIR}/compile_commands.json
#         ${CMAKE_SOURCE_DIR}/compile_commands.json
#     COMMENT "Copying compile_commands.json to project root for IDE integration"
# )

# ==============================================================================
# Build Complete - Binary Output
# ==============================================================================
# After successful build, the output files are located in:
#   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf  (Executable)
#   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin  (Binary for flashing)
#   ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex  (Intel HEX format)
# ==============================================================================

message(STATUS "Configuration complete - ready to build!")
