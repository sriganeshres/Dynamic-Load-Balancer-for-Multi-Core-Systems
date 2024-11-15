#include "task_queue.h"
#include "logger.h"
#include <stdlib.h>

TaskQueue* init_task_queue(int capacity) {
    TaskQueue* queue = malloc(sizeof(TaskQueue));
    if (!queue) return NULL;
    
    queue->tasks = malloc(sizeof(Task*) * capacity);
    if (!queue->tasks) {
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    
    return queue;
}

int enqueue_task(TaskQueue* queue, Task* task) {
    pthread_mutex_lock(&queue->mutex);
    fprintf(stdout, "enqueue task");
    fflush(stdout);
    while (queue->size >= queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->tasks[queue->rear] = task;
    queue->size++;
    
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    
    log_message(LOG_DEBUG, "Task %d enqueued", task->task_id);
    return 0;
}

Task* dequeue_task(TaskQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    Task* task = queue->tasks[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    
    log_message(LOG_DEBUG, "Task %d dequeued", task->task_id);
    return task;
}


void cleanup_task_queue(TaskQueue* queue) {
    if (queue == NULL) {
        return; // Nothing to clean up
    }

    // Lock the mutex before cleaning up
    pthread_mutex_lock(&queue->mutex);

    // Free all tasks in the queue
    for (int i = 0; i < queue->size; ++i) {
        int index = (queue->front + i) % queue->capacity;
        if (queue->tasks[index] != NULL) {
            free_task(queue->tasks[index]); // Free individual tasks
            queue->tasks[index] = NULL;
        }
    }

    // Free the tasks array
    if (queue->tasks != NULL) {
        free(queue->tasks);
        queue->tasks = NULL;
    }

    // Unlock and destroy the mutex
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);

    // Destroy the condition variables
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);

    // Reset fields
    queue->capacity = 0;
    queue->size = 0;
    queue->front = 0;
    queue->rear = 0;
}
