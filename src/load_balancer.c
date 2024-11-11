#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "load_balancer.h"

static void* core_process_tasks(void* arg);
static void* balance_load(void* arg);
static Core* select_optimal_core(LoadBalancer* balancer);

LoadBalancer* balancer_create(int num_cores) {
    LoadBalancer* balancer = malloc(sizeof(LoadBalancer));
    balancer->num_cores = num_cores;
    balancer->cores = malloc(sizeof(Core*) * num_cores);
    balancer->core_threads = malloc(sizeof(pthread_t) * num_cores);
    
    for (int i = 0; i < num_cores; i++) {
        balancer->cores[i] = core_create(i);
    }
    
    balancer->task_queue = queue_create();
    balancer->completed_tasks = queue_create();
    balancer->total_tasks = 0;
    balancer->is_running = 0;
    
    return balancer;
}

void balancer_destroy(LoadBalancer* balancer) {
    for (int i = 0; i < balancer->num_cores; i++) {
        core_destroy(balancer->cores[i]);
    }
    free(balancer->cores);
    free(balancer->core_threads);
    queue_destroy(balancer->task_queue);
    queue_destroy(balancer->completed_tasks);
    free(balancer);
}


static int current_core_index = 0;

static Core* select_optimal_core(LoadBalancer* balancer) {
    Core* optimal_core = balancer->cores[current_core_index];
    current_core_index = (current_core_index + 1) % balancer->num_cores;
    return optimal_core;
}

static void* core_process_tasks(void* arg) {
    Core* core = (Core*)arg;
    
    while (core->is_active) {
        Task* task = core_get_next_task(core);
        if (task != NULL) {
            clock_gettime(CLOCK_REALTIME, &task->start_time);
            usleep(task->workload * 1000000);  // Convert to microseconds
            clock_gettime(CLOCK_REALTIME, &task->completion_time);
            core->total_tasks_processed++;
        } else {
            usleep(100000);  // Sleep for 100ms if no tasks
        }
    }
    
    return NULL;
}

static void* balance_load(void* arg) {
    LoadBalancer* balancer = (LoadBalancer*)arg;
    
    while (balancer->is_running) {
        Task* task = queue_pop(balancer->task_queue);
        if (task != NULL) {
            Core* optimal_core = select_optimal_core(balancer);
            core_add_task(optimal_core, task);
        } else {
            usleep(100000);  // Sleep for 100ms if no tasks
        }
    }
    
    return NULL;
}

void balancer_run_simulation(LoadBalancer* balancer, int duration, double task_generation_rate) {
    balancer->is_running = 1;
    
    // Start core threads
    for (int i = 0; i < balancer->num_cores; i++) {
        pthread_create(&balancer->core_threads[i], NULL, core_process_tasks, balancer->cores[i]);
    }
    
    // Start load balancer thread
    pthread_create(&balancer->balancer_thread, NULL, balance_load, balancer);
    
    // Generate tasks
    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        if ((double)rand() / RAND_MAX < task_generation_rate / 10.0) {
            Task* task = task_create(balancer->total_tasks++, 
                                   (double)rand() / RAND_MAX * 1.9 + 0.1);  // 0.1 to 2.0 seconds
            queue_push(balancer->task_queue, task);
        }
        usleep(100000);  // Sleep for 100ms between task generation attempts
    }
    
    // Clean up
    balancer->is_running = 0;
    for (int i = 0; i < balancer->num_cores; i++) {
        balancer->cores[i]->is_active = 0;
        pthread_join(balancer->core_threads[i], NULL);
    }
    pthread_join(balancer->balancer_thread, NULL);
}

void balancer_print_stats(LoadBalancer* balancer) {
    printf("\nSimulation Results:\n");
    printf("Total tasks generated: %d\n", balancer->total_tasks);
    
    int total_completed = 0;
    for (int i = 0; i < balancer->num_cores; i++) {
        printf("Core %d processed %d tasks\n", 
               balancer->cores[i]->core_id, 
               balancer->cores[i]->total_tasks_processed);
        total_completed += balancer->cores[i]->total_tasks_processed;
    }
    printf("Total tasks completed: %d\n", total_completed);
}