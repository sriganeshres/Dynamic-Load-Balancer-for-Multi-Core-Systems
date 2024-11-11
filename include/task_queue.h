#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>
#include "task.h"

typedef struct TaskNode {
    Task* task;
    struct TaskNode* next;
} TaskNode;

typedef struct {
    TaskNode* head;
    TaskNode* tail;
    int size;
    pthread_mutex_t lock;
} TaskQueue;

TaskQueue* queue_create();
void queue_destroy(TaskQueue* queue);
void queue_push(TaskQueue* queue, Task* task);
Task* queue_pop(TaskQueue* queue);
int queue_is_empty(TaskQueue* queue);

#endif