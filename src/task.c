#include <stdlib.h>
#include <time.h>
#include "task.h"

Task* task_create(int task_id, double workload) {
    Task* task = malloc(sizeof(Task));
    task->task_id = task_id;
    task->workload = workload;
    task->assigned_core = -1;
    return task;
}

void task_destroy(Task* task) {
    free(task);
}

double task_get_duration(Task* task) {
    double duration = (task->completion_time.tv_sec - task->start_time.tv_sec) +
                     (task->completion_time.tv_nsec - task->start_time.tv_nsec) / 1e9;
    return duration;
}