#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "core.h"
#include "task_queue.h"

typedef struct {
    Core** cores;
    int num_cores;
    TaskQueue* task_queue;
    TaskQueue* completed_tasks;
    int total_tasks;
    pthread_t* core_threads;
    pthread_t balancer_thread;
    int is_running;
} LoadBalancer;

LoadBalancer* balancer_create(int num_cores);
void balancer_destroy(LoadBalancer* balancer);
void balancer_run_simulation(LoadBalancer* balancer, int duration, double task_generation_rate);
void balancer_print_stats(LoadBalancer* balancer);

#endif