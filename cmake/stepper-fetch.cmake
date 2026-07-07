# ============================================================================
# StepperDriver - Stepper Motor Control Library - CMake Integration
# ============================================================================
# This file automatically downloads and configures StepperDriver for your project.
#
# What it does:
# 1. Downloads StepperDriver from GitHub (only once, then cached)
# 2. Makes StepperDriver available as a CMake target
# 3. Provides stepper motor control with acceleration/deceleration
#
# For beginners:
# - Supports A4988, DRV8825, DRV8834, DRV8880 drivers
# - Features constant speed, linear acceleration, microstepping
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download StepperDriver from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  stepperdriver
  GIT_REPOSITORY https://github.com/laurb9/StepperDriver.git
  GIT_TAG v1.5.0
)

# Actually download and make StepperDriver available to CMake
FetchContent_MakeAvailable(stepperdriver)

# ----------------------------------------------------------------------------
# Create StepperDriver library target
# ----------------------------------------------------------------------------
if(NOT TARGET stepperdriver)
  add_library(stepperdriver STATIC)

  # Get the source directory from FetchContent
  FetchContent_GetProperties(stepperdriver SOURCE_DIR STEPPER_SOURCE_DIR)

  target_sources(stepperdriver PRIVATE
    ${STEPPER_SOURCE_DIR}/src/StepperDriver.cpp
  )

  target_include_directories(stepperdriver PUBLIC
    ${STEPPER_SOURCE_DIR}/src
  )

  # Add compile definitions if needed
  target_compile_definitions(stepperdriver PUBLIC
    # Add any required defines here
  )
endif()</content>
<parameter name="filePath">/home/mahonri/Desktop/BareMetal/Sensor_Cons/cmake/stepper-fetch.cmake
