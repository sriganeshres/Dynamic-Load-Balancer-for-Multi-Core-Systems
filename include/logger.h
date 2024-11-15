#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void init_logger(const char* log_file, int detailed_logging);
void log_message(LogLevel level, const char* format, ...);
void cleanup_logger(void);

#endif