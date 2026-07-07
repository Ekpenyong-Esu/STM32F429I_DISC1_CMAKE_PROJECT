# ============================================================================
# LVGL (Light and Versatile Graphics Library) - CMake Integration
# ============================================================================
# This file automatically downloads and configures LVGL for your project.
#
# What it does:
# 1. Downloads LVGL v8.3.11 from GitHub (only once, then cached)
# 2. Makes LVGL available as a CMake target
# 3. Handles different LVGL versions with compatible target names
#
# For beginners:
# - You don't need to manually download LVGL
# - CMake handles everything automatically during build
# - LVGL configuration is in: Peripherals/LVGL/lv_conf.h
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download LVGL from GitHub
# ----------------------------------------------------------------------------
# This tells CMake to fetch LVGL source code from the official repository
# GIT_TAG: Specifies version v9.4.0 (latest stable release with modern API)
# The download happens once and is cached in your build directory

# Set the path to lv_conf.h BEFORE calling FetchContent
# LVGL v9 needs to know where to find the configuration file
set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/Peripherals/LVGL_App/lv_conf.h CACHE STRING "" FORCE)

FetchContent_Declare(
  lvgl                                          # Project name used internally
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.4.0                               # Latest LVGL v9.x version
)

# Set the path to lv_conf.h so LVGL can find it during configuration
set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/Peripherals/LVGL_App/lv_conf.h CACHE STRING "" FORCE)
set(LV_BUILD_CONF_PATH ${CMAKE_SOURCE_DIR}/Peripherals/LVGL_App/lv_conf.h CACHE STRING "" FORCE)

# Actually download and make LVGL available to CMake
FetchContent_MakeAvailable(lvgl)

# ----------------------------------------------------------------------------
# Find the correct LVGL target name
# ----------------------------------------------------------------------------
# Different LVGL versions use different CMake target names.
# This section automatically detects which one is available.
#
# Possible targets:
#   - lvgl::lvgl (newer versions with namespace)
#   - lvgl       (older versions without namespace)

if(TARGET lvgl::lvgl)
  set(LVGL_TARGET lvgl::lvgl)
  message(STATUS "LVGL: Found target 'lvgl::lvgl'")
elseif(TARGET lvgl)
  set(LVGL_TARGET lvgl)
  message(STATUS "LVGL: Found target 'lvgl'")
else()
  # If neither target exists, something went wrong
  message(FATAL_ERROR "LVGL CMake target not found after FetchContent. "
                      "Check lvgl version/tag or internet connection.")
endif()

# ----------------------------------------------------------------------------
# Result: ${LVGL_TARGET} variable now points to the correct LVGL library
# This variable is used in the main CMakeLists.txt to link LVGL to your project
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# Add lv_conf.h location to LVGL include path
# ----------------------------------------------------------------------------
# LVGL needs to find lv_conf.h (your custom configuration file)
# Add the directory containing lv_conf.h to the main project target so user
# code can find lv_conf.h regardless of whether LVGL targets are namespaced
# or ALIASed (calling target_* on ALIAS targets is not allowed).
if(TARGET ${CMAKE_PROJECT_NAME})
  target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Peripherals/LVGL)
else()
  # Fallback for very early inclusion cases: add the directory globally
  include_directories(${CMAKE_SOURCE_DIR}/Peripherals/LVGL)
endif()
