#include <stdlib.h>
#include "task_queue.h"

TaskQueue* queue_create() {
    TaskQueue* queue = malloc(sizeof(TaskQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void queue_destroy(TaskQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    TaskNode* current = queue->head;
    while (current != NULL) {
        TaskNode* next = current->next;
        task_destroy(current->task);
        free(current);
        current = next;
    }
    pthread_mutex_unlock(&queue->lock);
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

void queue_push(TaskQueue* queue, Task* task) {
    TaskNode* node = malloc(sizeof(TaskNode));
    node->task = task;
    node->next = NULL;

    pthread_mutex_lock(&queue->lock);
    if (queue->tail == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
    queue->size++;
    pthread_mutex_unlock(&queue->lock);
}

Task* queue_pop(TaskQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    TaskNode* node = queue->head;
    Task* task = node->task;
    queue->head = node->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->size--;
    free(node);
    pthread_mutex_unlock(&queue->lock);
    return task;
}

int queue_is_empty(TaskQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    int empty = (queue->size == 0);
    pthread_mutex_unlock(&queue->lock);
    return empty;
}