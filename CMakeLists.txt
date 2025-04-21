cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Sensor_Console)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Replace the manual include directories with automatic subdirectory discovery
file(GLOB PERIPHERAL_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/*")
foreach(dir ${PERIPHERAL_DIRS})
    if(IS_DIRECTORY ${dir})
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${dir})
    endif()
endforeach()

# Add the peripheral source files
file(GLOB_RECURSE PERIPHERALS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Peripherals/*/*.c"
)

# Add the peripheral sources to the project
target_sources(${PROJECT_NAME} PRIVATE ${PERIPHERALS_SOURCES})

#Add post-build step to copy compile_commands.json to project root
# add_custom_command(
#     TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         ${CMAKE_BINARY_DIR}/compile_commands.json
#         ${CMAKE_SOURCE_DIR}/compile_commands.json
#     COMMENT "Copying compile_commands.json to project root directory"
# )
