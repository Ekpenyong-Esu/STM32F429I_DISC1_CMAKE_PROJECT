#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <printf/printf.h>

// UART handle for logging output
static UART_HandleTypeDef *log_uart = NULL;

// Buffer for log messages (used for UART)
#define LOG_BUFFER_SIZE 256
static char log_buffer[LOG_BUFFER_SIZE];

// Initialize logging with UART handle
void log_init(UART_HandleTypeDef *huart) {
    log_uart = huart;
}

// Internal function to log messages with optional file/line
static void log_message_loc(log_level_t level, const char *file, int line, const char *format, va_list args) {
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
    if (file) {
#if LOG_SHOW_FULLPATH
        const char *file_to_print = file;
#else
        const char *file_to_print = file;
        const char *p = strrchr(file, '/');
#ifdef _WIN32
        if (!p) p = strrchr(file, '\\');
#endif
        if (p) file_to_print = p + 1;
#endif
        printf("(%s:%d) ", file_to_print, line);
    }
    vprintf(format, args);
    printf("\r\n");
#elif LOG_USE_UART
    // Use UART for logging
    if (log_uart == NULL) return;

    // Format header into buffer
    int off = snprintf(log_buffer, LOG_BUFFER_SIZE, "%s", level_str);
    if (file && off < LOG_BUFFER_SIZE) {
#if LOG_SHOW_FULLPATH
        const char *file_to_print = file;
#else
        const char *file_to_print = file;
        const char *p = strrchr(file, '/');
#ifdef _WIN32
        if (!p) p = strrchr(file, '\\');
#endif
        if (p) file_to_print = p + 1;
#endif
        int n = snprintf(log_buffer + off, LOG_BUFFER_SIZE - off, "(%s:%d) ", file_to_print, line);
        if (n > 0) off += n;
    }

    // Format the message
    vsnprintf(log_buffer + off, LOG_BUFFER_SIZE - off, format, args);

    // Add newline
    strncat(log_buffer, "\r\n", LOG_BUFFER_SIZE - strlen(log_buffer) - 1);

    // Transmit via UART
    HAL_UART_Transmit(log_uart, (uint8_t *)log_buffer, strlen(log_buffer), HAL_MAX_DELAY);
#else
    (void)level;
    (void)file;
    (void)line;
    (void)format;
    (void)args;
#endif
}

// Public varargs logging function used by macros
void log_logf(log_level_t level, const char *file, int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message_loc(level, file, line, format, args);
    va_end(args);
}
