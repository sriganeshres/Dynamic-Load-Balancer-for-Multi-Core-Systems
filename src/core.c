#include <stdlib.h>
#include "core.h"

Core* core_create(int core_id) {
    Core* core = malloc(sizeof(Core));
    core->core_id = core_id;
    core->current_load = 0.0;
    core->tasks = queue_create();
    core->total_tasks_processed = 0;
    core->is_active = 1;
    pthread_mutex_init(&core->lock, NULL);
    return core;
}

void core_destroy(Core* core) {
    queue_destroy(core->tasks);
    pthread_mutex_destroy(&core->lock);
    free(core);
}

void core_add_task(Core* core, Task* task) {
    pthread_mutex_lock(&core->lock);
    queue_push(core->tasks, task);
    core->current_load += task->workload;
    pthread_mutex_unlock(&core->lock);
}

Task* core_get_next_task(Core* core) {
    pthread_mutex_lock(&core->lock);
    Task* task = queue_pop(core->tasks);
    if (task != NULL) {
        core->current_load -= task->workload;
    }
    pthread_mutex_unlock(&core->lock);
    return task;
}

double core_get_load(Core* core) {
    pthread_mutex_lock(&core->lock);
    double load = core->current_load;
    pthread_mutex_unlock(&core->lock);
    return load;
}