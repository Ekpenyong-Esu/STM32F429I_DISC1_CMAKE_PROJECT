# ============================================================================
# FatFS - FAT Filesystem - CMake Integration
# ============================================================================
# This file automatically downloads and configures FatFS library.
#
# What it does:
# 1. Downloads FatFS from GitHub (ChaN's official FatFS)
# 2. Makes FatFS available as a CMake target
# 3. Provides FAT12/16/32 filesystem support
#
# For beginners:
# - FatFS is the de facto standard for SD card filesystems
# - Supports FAT12, FAT16, FAT32 formats
# - Essential for SD card data logging, file storage
# - Complements LittleFS (wear-leveling) for different use cases
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Download FatFS from GitHub
# ----------------------------------------------------------------------------
FetchContent_Declare(
  fatfs
  GIT_REPOSITORY https://github.com/abbrev/fatfs.git
  GIT_TAG R0.15
)

# Actually download and make FatFS available to CMake
FetchContent_MakeAvailable(fatfs)

# ----------------------------------------------------------------------------
# Create FatFS library target
# ----------------------------------------------------------------------------
if(NOT TARGET fatfs)
  # Get the source directory from FetchContent
  FetchContent_GetProperties(fatfs SOURCE_DIR FATFS_SOURCE_DIR)

  add_library(fatfs STATIC
    ${FATFS_SOURCE_DIR}/source/ff.c
    ${FATFS_SOURCE_DIR}/source/ffsystem.c
    ${FATFS_SOURCE_DIR}/source/ffunicode.c
  )

  target_include_directories(fatfs INTERFACE
    ${FATFS_SOURCE_DIR}/source
  )

  target_compile_definitions(fatfs PUBLIC
    # FatFS configuration - adjust based on your needs
    FF_FS_READONLY=0        # Read/Write mode
    FF_FS_MINIMIZE=0        # Full featured
    FF_USE_STRFUNC=1        # Enable string functions
    FF_USE_FIND=1           # Enable find functions
    FF_USE_MKFS=1           # Enable mkfs function
    FF_USE_FASTSEEK=0       # Disable fast seek (saves RAM)
    FF_USE_EXPAND=0         # Disable expand (saves RAM)
    FF_USE_CHMOD=1          # Enable chmod functions
    FF_USE_LABEL=1          # Enable volume label functions
    FF_USE_FORWARD=0        # Disable forward functions
    FF_CODE_PAGE=437        # Default code page
  )
endif()

# ----------------------------------------------------------------------------
# Result: 'fatfs' target is now available for linking
# ----------------------------------------------------------------------------
message(STATUS "FatFS: FAT filesystem library ready")
message(STATUS "FatFS: Include <ff.h> in your code")
message(STATUS "FatFS: Link with 'fatfs' target in CMakeLists.txt")
message(STATUS "FatFS: Requires disk I/O functions (ff_diskio.h implementation)")
