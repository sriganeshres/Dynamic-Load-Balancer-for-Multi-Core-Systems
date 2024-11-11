#ifndef TASK_H
#define TASK_H

#include <time.h>

typedef struct {
    int task_id;
    double workload;         // Simulated workload in seconds
    int assigned_core;
    struct timespec start_time;
    struct timespec completion_time;
} Task;

Task* task_create(int task_id, double workload);
void task_destroy(Task* task);
double task_get_duration(Task* task);

#endif