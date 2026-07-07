# Example: How to integrate external libraries into your main CMakeLists.txt
#
# Add this after your LVGL configuration section in CMakeLists.txt

# ------------------------------------------------------------------------------
# External Libraries - Choose which ones to include
# ------------------------------------------------------------------------------
# High priority for sensor projects:
option(USE_LITTLEFS "Include LittleFS filesystem" ON)
option(LITTLEFS_USE_LOCAL "Use local LittleFS checkout instead of FetchContent" OFF)

# Essential for development:
option(USE_UNITY "Include Unity testing framework" ON)
option(USE_PRINTF "Include lightweight printf" ON)

# Nice to have for configuration:
option(USE_MININI "Include minIni configuration parser" OFF)

# Advanced testing (requires Unity):
option(USE_CMOCK "Include CMock mocking framework" OFF)

# ------------------------------------------------------------------------------
# Include External Library Scripts
# ------------------------------------------------------------------------------
if(USE_LITTLEFS)
    if(LITTLEFS_USE_LOCAL)
        message(STATUS "Config: Using local LittleFS checkout")
        include("cmake/littlefs-local.cmake")
    else()
        message(STATUS "Config: Using FetchContent to download LittleFS")
        include("cmake/littlefs-fetch.cmake")
    endif()
endif()

if(USE_UNITY)
    include("cmake/unity-fetch.cmake")
endif()

if(USE_PRINTF)
    include("cmake/printf-fetch.cmake")
endif()

if(USE_MININI)
    include("cmake/minini-fetch.cmake")
endif()

if(USE_CMOCK)
    include("cmake/cmock-fetch.cmake")
endif()

# ------------------------------------------------------------------------------
# Link External Libraries to Executable
# ------------------------------------------------------------------------------
# Add these targets to your target_link_libraries() section:
#
# target_link_libraries(${CMAKE_PROJECT_NAME}
#     stm32cubemx
#     ${LVGL_TARGET}
#
#     # External libraries (add these lines):
#     $<$<BOOL:${USE_LITTLEFS}>:littlefs>
#     $<$<BOOL:${USE_UNITY}>:unity>
#     $<$<BOOL:${USE_PRINTF}>:printf>
#     $<$<BOOL:${USE_MININI}>:minini>
#     $<$<BOOL:${USE_CMOCK}>:cmock>
# )
