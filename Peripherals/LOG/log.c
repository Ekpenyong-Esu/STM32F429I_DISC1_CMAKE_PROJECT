#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// UART handle for logging output
static UART_HandleTypeDef *log_uart = NULL;

// Buffer for log messages (used for UART)
#define LOG_BUFFER_SIZE 256
static char log_buffer[LOG_BUFFER_SIZE];

// Initialize logging with UART handle
void log_init(UART_HandleTypeDef *huart) {
    log_uart = huart;
}

// Internal function to log messages
static void log_message(log_level_t level, const char *format, va_list args) {
    const char *level_str;
    switch (level) {
        case LOG_LEVEL_DEBUG: level_str = "[DEBUG] "; break;
        case LOG_LEVEL_INFO: level_str = "[INFO] "; break;
        case LOG_LEVEL_WARNING: level_str = "[WARNING] "; break;
        case LOG_LEVEL_ERROR: level_str = "[ERROR] "; break;
        default: level_str = "[UNKNOWN] "; break;
    }

#if LOG_USE_PRINTF
    // Use printf for logging
    printf("%s", level_str);
    vprintf(format, args);
    printf("\r\n");
#elif LOG_USE_UART
    // Use UART for logging
    if (log_uart == NULL) return;

    // Copy level string to buffer
    strcpy(log_buffer, level_str);

    // Format the message
    vsnprintf(log_buffer + strlen(level_str), LOG_BUFFER_SIZE - strlen(level_str), format, args);

    // Add newline
    strncat(log_buffer, "\r\n", LOG_BUFFER_SIZE - strlen(log_buffer) - 1);

    // Transmit via UART
    HAL_UART_Transmit(log_uart, (uint8_t *)log_buffer, strlen(log_buffer), HAL_MAX_DELAY);
#else
    // Logging disabled
    (void)level;
    (void)format;
    (void)args;
#endif
}

// Convenience functions
void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void log_warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}
