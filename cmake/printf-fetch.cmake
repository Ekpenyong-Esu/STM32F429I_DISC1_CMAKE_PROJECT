# ============================================================================
# printf - Lightweight printf Implementation - CMake Integration
# ============================================================================
# This file automatically downloads and configures printf for your project.
#
# What it does:
# 1. Downloads lightweight printf from GitHub (only once, then cached)
# 2. Makes printf available as a CMake target
# 3. Provides sprintf, snprintf without full stdio overhead
#
# For beginners:
# - printf is a minimal printf implementation for embedded
# - Much smaller than standard library printf
# - Perfect for logging, data formatting, UART output
# - CMake handles everything automatically during build
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download printf from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  printf
  GIT_REPOSITORY https://github.com/eyalroz/printf.git
  GIT_TAG v6.0.0
)

# Actually download and make printf available to CMake
FetchContent_MakeAvailable(printf)

# ----------------------------------------------------------------------------
# Create printf library target
# ----------------------------------------------------------------------------
if(NOT TARGET printf)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(printf SOURCE_DIR PRINTF_SOURCE_DIR)

  add_library(printf STATIC
    ${PRINTF_SOURCE_DIR}/printf/printf.c
  )

  target_include_directories(printf INTERFACE
    ${PRINTF_SOURCE_DIR}/printf
  )

  # printf configuration - adjust based on your needs
  target_compile_definitions(printf PUBLIC
    PRINTF_SUPPORT_DECIMAL_SPECIFIERS=1    # Enable %d, %u
    PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS=0  # Disable %e, %g (saves space)
    PRINTF_SUPPORT_WRITEBACK_SPECIFIER=0     # Disable %n
    PRINTF_SUPPORT_MSVC_STYLE_INTEGER_SPECIFIERS=0  # Disable MSVC extensions
    # Expose standard names (printf, vprintf, snprintf, ...) to consumers so
    # including <printf/printf.h> lets user call the standard names instead of
    # the library-internal names with trailing underscore.
    PRINTF_ALIAS_STANDARD_FUNCTION_NAMES=1
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'printf' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "printf: Lightweight printf implementation ready")
message(STATUS "printf: Include <printf.h> in your code")
message(STATUS "printf: Link with 'printf' target in CMakeLists.txt")
message(STATUS "printf: Use sprintf(), snprintf() for formatting")
