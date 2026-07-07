# ============================================================================
# ServoMotor - Servo Motor Control Library - CMake Integration
# ============================================================================
# This file automatically downloads and configures ServoMotor for your project.
#
# What it does:
# 1. Downloads ServoMotor from GitHub (only once, then cached)
# 2. Makes ServoMotor available as a CMake target
# 3. Provides servo motor control with PWM positioning
#
# For beginners:
# - Controls servo motors with 0-180 degree positioning
# - Uses PWM for precise angle control
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download ServoMotor from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  servomotor
  GIT_REPOSITORY https://github.com/FilipeChagasDev/stm32-servomotor.git
  GIT_TAG master
)

# Actually download and make ServoMotor available to CMake
FetchContent_MakeAvailable(servomotor)

# ----------------------------------------------------------------------------
# Create ServoMotor library target
# ----------------------------------------------------------------------------
if(NOT TARGET servomotor)
  add_library(servomotor STATIC)

  # Get the source directory from FetchContent
  FetchContent_GetProperties(servomotor SOURCE_DIR SERVO_SOURCE_DIR)

  target_sources(servomotor PRIVATE
    ${SERVO_SOURCE_DIR}/servomotor.c
  )

  target_include_directories(servomotor PUBLIC
    ${SERVO_SOURCE_DIR}
  )

  # Add compile definitions if needed
  target_compile_definitions(servomotor PUBLIC
    # Add any required defines here
  )
endif()</content>
<parameter name="filePath">/home/mahonri/Desktop/BareMetal/Sensor_Cons/cmake/servo-fetch.cmake