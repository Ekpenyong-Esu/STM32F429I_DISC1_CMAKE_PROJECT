# ============================================================================
# LVGL - Local Integration
# ============================================================================
# Use this file when you have a local checkout of LVGL inside your project
# (for example at ${CMAKE_SOURCE_DIR}/lvgl). It mirrors the behaviour of
# cmake/lvgl-fetch.cmake but uses the local source tree instead of
# downloading LVGL via FetchContent.
#
# Usage:
# - Turn on the option `LVGL_USE_LOCAL` in your top-level CMakeLists.txt
# - Optionally set `LVGL_LOCAL_DIR` to point to your local lvgl directory
#   (default: ${CMAKE_SOURCE_DIR}/lvgl)
# ============================================================================

# Default path to local LVGL (can be overridden by the user)
if(NOT DEFINED LVGL_LOCAL_DIR)
  set(LVGL_LOCAL_DIR ${CMAKE_SOURCE_DIR}/lvgl CACHE PATH "Path to local LVGL source")
endif()

message(STATUS "LVGL (local): looking for LVGL at ${LVGL_LOCAL_DIR}")

# LVGL needs to find lv_conf.h before it is configured
set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/Peripherals/LVGL/lv_conf.h CACHE STRING "Path to lv_conf.h" FORCE)
set(LV_BUILD_CONF_PATH ${CMAKE_SOURCE_DIR}/Peripherals/LVGL/lv_conf.h CACHE STRING "Path to lv_conf.h" FORCE)

# Validate that the directory exists and appears to be LVGL
if(NOT EXISTS "${LVGL_LOCAL_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "Local LVGL not found at ${LVGL_LOCAL_DIR}. Please set LVGL_LOCAL_DIR to the path of LVGL source containing CMakeLists.txt")
endif()

# Add LVGL sources as a subdirectory so that it creates its CMake targets
# Put its build tree under ${CMAKE_BINARY_DIR}/lvgl_local_build to avoid
# interfering with other builds
add_subdirectory(${LVGL_LOCAL_DIR} ${CMAKE_BINARY_DIR}/lvgl_local_build)

# Determine the available LVGL target (same logic as fetch variant)
if(TARGET lvgl::lvgl)
  set(LVGL_TARGET lvgl::lvgl)
  message(STATUS "LVGL (local): Found target 'lvgl::lvgl'")
elseif(TARGET lvgl)
  set(LVGL_TARGET lvgl)
  message(STATUS "LVGL (local): Found target 'lvgl'")
else()
  message(FATAL_ERROR "LVGL CMake target not found after adding local LVGL. Ensure the LVGL sources are a compatible version that defines a CMake target named 'lvgl' or 'lvgl::lvgl'.")
endif()

# Ensure LVGL can find the user's lv_conf.h (Peripherals/LVGL)
# Add the directory to the main project target instead of the LVGL target
# (calling target_* on ALIAS targets is not allowed). This ensures
# user code sees lv_conf.h during compilation.
if(TARGET ${CMAKE_PROJECT_NAME})
  target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Peripherals/LVGL)
else()
  include_directories(${CMAKE_SOURCE_DIR}/Peripherals/LVGL)
endif()

message(STATUS "LVGL (local): configured with target ${LVGL_TARGET}")
