#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

static FILE* log_file = NULL;
static int detailed_logging = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_logger(const char* file_path, int detailed) {
    log_file = fopen(file_path, "a");
    detailed_logging = detailed;
}

void log_message(LogLevel level, const char* format, ...) {
    if (!log_file) return;
    
    pthread_mutex_lock(&log_mutex);
    
    time_t now;
    time(&now);
    char* date = ctime(&now);
    date[strlen(date) - 1] = '\0';
    
    const char* level_str;
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG"; break;
        case LOG_INFO:    level_str = "INFO"; break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR"; break;
        default:          level_str = "UNKNOWN";
    }
    
    fprintf(log_file, "[%s] [%s] ", date, level_str);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
    
    pthread_mutex_unlock(&log_mutex);
}

void cleanup_logger(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}