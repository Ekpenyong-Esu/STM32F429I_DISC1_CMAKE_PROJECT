#ifndef LOG_H
#define LOG_H

#include "stm32f4xx_hal.h"


// Define these to enable/disable logging methods
// Set to 1 to enable, 0 to disable
#define LOG_USE_PRINTF 1  // Use printf for logging
#define LOG_USE_UART 0    // Use UART for logging (requires log_init)

// Show full file path or only filename in logs.
// Set to 1 to include full path (e.g., /home/.../file.c), 0 to include only the basename (file.c)
#ifndef LOG_SHOW_FULLPATH
#define LOG_SHOW_FULLPATH 0
#endif

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} log_level_t;

// Function to initialize logging (required for UART)
void log_init(UART_HandleTypeDef *huart);

// Core logging function that accepts caller location
void log_logf(log_level_t level, const char *file, int line, const char *format, ...);

// Convenience macros that automatically pass __FILE__ and __LINE__
#define log_debug(format, ...)   log_logf(LOG_LEVEL_DEBUG,   __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_info(format, ...)    log_logf(LOG_LEVEL_INFO,    __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_warning(format, ...) log_logf(LOG_LEVEL_WARNING, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_error(format, ...)   log_logf(LOG_LEVEL_ERROR,   __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // LOG_H
