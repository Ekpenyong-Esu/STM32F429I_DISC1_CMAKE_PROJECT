# STMPE Touchscreen Controller FetchContent
# STMPE610/STMPE811 capacitive/resistive touchscreen controllers
# Compatible with STM32 HAL

include(FetchContent)

set(STMPE_GIT_TAG "main" CACHE STRING "STMPE git tag to fetch")
set(STMPE_GIT_URL "https://github.com/STMicroelectronics/stm32-stmpe.git" CACHE STRING "STMPE git repository URL")

FetchContent_Declare(
    stmpe
    GIT_REPOSITORY ${STMPE_GIT_URL}
    GIT_TAG ${STMPE_GIT_TAG}
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(stmpe)

# Create STMPE library target
add_library(STMPE STATIC
    ${stmpe_SOURCE_DIR}/stmpe.c
)

# Include directories
target_include_directories(STMPE INTERFACE
    ${stmpe_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(STMPE PUBLIC
    stm32cubemx  # STM32 HAL library
)

# Compiler options
target_compile_options(STMPE PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-std=c99>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    $<$<COMPILE_LANGUAGE:C>:-Wno-unused-parameter>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>
)

# Set target properties
set_target_properties(STMPE PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# Display configuration summary
message(STATUS "STMPE Touchscreen Controller:")
message(STATUS "  URL: ${STMPE_GIT_URL}")
message(STATUS "  Tag: ${STMPE_GIT_TAG}")
message(STATUS "  Source: ${stmpe_SOURCE_DIR}")
