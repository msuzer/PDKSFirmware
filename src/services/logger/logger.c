#include "logger.h"
#include <stdio.h>
#include <stdarg.h>

static void log_print(const char *level, const char *fmt, va_list args)
{
    printf("[%s] ", level);
    vprintf(fmt, args);
    printf("\n");
}

void logger_init(void)
{
    // UART already initialized by ESP-IDF
    printf("\n--- Logger started ---\n");
}

void log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_print("INFO", fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_print("WARN", fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_print("ERROR", fmt, args);
    va_end(args);
}

void log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_print("DEBUG", fmt, args);
    va_end(args);
}