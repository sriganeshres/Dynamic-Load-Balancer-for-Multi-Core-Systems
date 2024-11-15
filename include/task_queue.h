#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "task.h"

typedef struct {
    Task** tasks;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} TaskQueue;

TaskQueue* init_task_queue(int capacity);
int enqueue_task(TaskQueue* queue, Task* task);
Task* dequeue_task(TaskQueue* queue);
void cleanup_task_queue(TaskQueue* queue);

#endif