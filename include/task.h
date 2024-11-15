#ifndef TASK_H
#define TASK_H

#include <pthread.h>
#include <time.h>

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_CRITICAL = 3
} TaskPriority;

typedef enum {
    STATUS_PENDING,
    STATUS_RUNNING,
    STATUS_COMPLETED,
    STATUS_FAILED
} TaskStatus;

typedef struct {
    int task_id;
    TaskPriority priority;
    void (*function)(void*);
    void* args;
    pthread_t thread;
    int assigned_cpu;
    TaskStatus status;
    struct timespec create_time;
    struct timespec start_time;
    struct timespec end_time;
    double cpu_usage;
    double memory_usage;
} Task;

Task* create_task(void (*function)(void*), void* args, TaskPriority priority);
void free_task(Task* task);

#endif