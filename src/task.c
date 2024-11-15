#include "task.h"
#include <stdlib.h>
#include <string.h>

static int next_task_id = 0;

Task* create_task(void (*function)(void*), void* args, TaskPriority priority) {
    Task* task = malloc(sizeof(Task));
    if (!task) return NULL;
    
    task->task_id = __atomic_fetch_add(&next_task_id, 1, __ATOMIC_SEQ_CST);
    task->function = function;
    task->args = args;
    task->priority = priority;
    task->status = STATUS_PENDING;
    task->assigned_cpu = -1;
    task->cpu_usage = 0.0;
    task->memory_usage = 0.0;
    
    clock_gettime(CLOCK_MONOTONIC, &task->create_time);
    
    return task;
}

void free_task(Task* task) {
    if (task) {
        // Note: task->args should be freed by the caller if necessary
        free(task);
    }
}
