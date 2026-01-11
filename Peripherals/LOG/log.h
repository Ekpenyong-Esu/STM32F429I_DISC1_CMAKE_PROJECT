#ifndef LOG_H
#define LOG_H

#include "stm32f4xx_hal.h"
#include <stdio.h>

// Define these to enable/disable logging methods
// Set to 1 to enable, 0 to disable
#define LOG_USE_PRINTF 1  // Use printf for logging
#define LOG_USE_UART 0    // Use UART for logging (requires log_init)

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} log_level_t;

// Function to initialize logging (required for UART)
void log_init(UART_HandleTypeDef *huart);

// Logging functions
void log_debug(const char *format, ...);
void log_info(const char *format, ...);
void log_warning(const char *format, ...);
void log_error(const char *format, ...);

#endif // LOG_H
