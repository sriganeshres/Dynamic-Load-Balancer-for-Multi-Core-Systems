#ifndef CORE_H
#define CORE_H

#include <pthread.h>
#include "task_queue.h"

typedef struct {
    int core_id;
    double current_load;
    TaskQueue* tasks;
    int total_tasks_processed;
    int is_active;
    pthread_mutex_t lock;
} Core;

Core* core_create(int core_id);
void core_destroy(Core* core);
void core_add_task(Core* core, Task* task);
Task* core_get_next_task(Core* core);
double core_get_load(Core* core);

#endif