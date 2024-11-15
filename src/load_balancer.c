#include "load_balancer.h"
#include "logger.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>
#include <bits/cpu-set.h>

static pthread_mutex_t active_tasks_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t active_tasks_cond = PTHREAD_COND_INITIALIZER;
static int total_active_tasks = 0;

LoadBalancer* init_load_balancer(LoadBalancerConfig* config) {
    LoadBalancer* lb = malloc(sizeof(LoadBalancer));
    if (!lb) return NULL;
    
    lb->config = config;
    lb->cpu_monitor = init_cpu_monitor(config);
    lb->task_queue = init_task_queue(config->max_tasks);
    fprintf(stdout, "%d", config->max_tasks);
    fflush(stdout);
    char x;
    scanf("%c", &x);
    lb->running = 0;
    
    if (!lb->cpu_monitor || !lb->task_queue) {
        return NULL;
    }
    
    init_logger(config->log_file_path, config->enable_detailed_logging);
    return lb;
}

void start_load_balancer(LoadBalancer* lb) {
    lb->running = 1;
    pthread_create(&lb->monitor_thread, NULL, monitor_thread_func, lb);
    pthread_create(&lb->scheduler_thread, NULL, scheduler_thread_func, lb);
    log_message(LOG_INFO, "Load balancer started");
}

void* monitor_thread_func(void* arg) {
    LoadBalancer* lb = (LoadBalancer*)arg;
    
    while (lb->running) {
        update_cpu_stats(lb->cpu_monitor);
        
        if (lb->config->enable_detailed_logging) {
            print_cpu_stats(lb->cpu_monitor);
        }
        
        usleep(lb->config->monitoring_interval_ms * 1000);
    }
    
    return NULL;
}

int find_best_cpu(CPUMonitor* monitor) {
    int best_cpu = -1;
    double lowest_load = 999.9;
    log_message(LOG_INFO, "%d", monitor->num_cpus);
    
    for (int i = 0; i < monitor->num_cpus; i++) {
        double effective_load = monitor->stats[i].current_usage;
        
        if (monitor->config->enable_load_prediction) {
            effective_load = (effective_load + monitor->stats[i].predicted_load) / 2;
        }
        
        // Consider active tasks in the decision
        effective_load += (monitor->stats[i].active_tasks * 10);
        
        if (effective_load < lowest_load) {
            lowest_load = effective_load;
            best_cpu = i;
        }
    }
    
    return best_cpu;
}

int submit_task(LoadBalancer* lb, void (*function)(void*), void* args, TaskPriority priority) {
    Task* task = create_task(function, args, priority);
    if (!task) return -1;
    
    int result = enqueue_task(lb->task_queue, task);
    if (result != 0) {
        free_task(task);
        return -1;
    }
    
    return 0;
}

static void track_task_start() {
    pthread_mutex_lock(&active_tasks_mutex);
    total_active_tasks++;
    pthread_mutex_unlock(&active_tasks_mutex);
}

static void track_task_complete() {
    pthread_mutex_lock(&active_tasks_mutex);
    total_active_tasks--;
    if (total_active_tasks == 0) {
        pthread_cond_broadcast(&active_tasks_cond);
    }
    pthread_mutex_unlock(&active_tasks_mutex);
}

// Wrapper for task execution
static void* task_wrapper(void* arg) {
    Task* task = (Task*)arg;
    track_task_start();
    
    task->function(task->args);
    
    task->status = STATUS_COMPLETED;
    clock_gettime(CLOCK_MONOTONIC, &task->end_time);
    
    // Update CPU stats
    if (task->assigned_cpu >= 0) {
        pthread_mutex_lock(&active_tasks_mutex);
        task->cpu_usage = task->end_time.tv_sec - task->start_time.tv_sec +
                         (task->end_time.tv_nsec - task->start_time.tv_nsec) / 1e9;
        pthread_mutex_unlock(&active_tasks_mutex);
    }
    
    track_task_complete();
    return NULL;
}

void* scheduler_thread_func(void* arg) {
    LoadBalancer* lb = (LoadBalancer*)arg;
    sigset_t set;
    
    // Block SIGINT in this thread
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    
    while (lb->running) {
        Task* task = dequeue_task(lb->task_queue);
        if (!task) continue;  // Queue might be empty after shutdown signal
        
        if (!lb->running) {
            // If we're shutting down, mark task as failed and continue
            task->status = STATUS_FAILED;
            free_task(task);
            continue;
        }
        
        int cpu_id = find_best_cpu(lb->cpu_monitor);
        if (cpu_id >= 0) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu_id, &cpuset);
            
            task->assigned_cpu = cpu_id;
            task->status = STATUS_RUNNING;
            clock_gettime(CLOCK_MONOTONIC, &task->start_time);
            
            pthread_create(&task->thread, NULL, task_wrapper, task);
            pthread_setaffinity_np(task->thread, sizeof(cpu_set_t), &cpuset);
            
            // Detach the thread so we don't need to join it
            pthread_detach(task->thread);
            
            lb->cpu_monitor->stats[cpu_id].active_tasks++;
            log_message(LOG_INFO, "Task %d assigned to CPU %d", task->task_id, cpu_id);
        }
    }
    
    return NULL;
}

void wait_for_tasks_completion(LoadBalancer* lb) {
    pthread_mutex_lock(&active_tasks_mutex);
    while (total_active_tasks > 0) {
        pthread_cond_wait(&active_tasks_cond, &active_tasks_mutex);
    }
    pthread_mutex_unlock(&active_tasks_mutex);
}

void cancel_pending_tasks(LoadBalancer* lb) {
    pthread_mutex_lock(&lb->task_queue->mutex);
    log_message(LOG_INFO,"cancelling tasks started");
    while (lb->task_queue->size > 0) {
        Task* task = dequeue_task(lb->task_queue);
        if (task) {
            task->status = STATUS_FAILED;
            free_task(task);
        }
    }
    pthread_mutex_unlock(&lb->task_queue->mutex);
    log_message(LOG_INFO,"cancelling tasks completed %d", lb->task_queue->size);
}

void stop_load_balancer(LoadBalancer* lb) {
    if (!lb) return;
    
    log_message(LOG_INFO, "Initiating load balancer shutdown 1");
    
    // Set shutdown flag first
    pthread_mutex_lock(&lb->task_queue->mutex);
    lb->running = 0;
    pthread_mutex_unlock(&lb->task_queue->mutex);
    
    log_message(LOG_INFO, "Initiating load balancer shutdown 2");
    
    // Wake up ALL waiting threads with multiple broadcasts
    for (int i = 0; i < 3; i++) {  // Try multiple times to ensure threads wake up
        pthread_mutex_lock(&lb->task_queue->mutex);
        pthread_cond_broadcast(&lb->task_queue->not_empty);
        pthread_cond_broadcast(&lb->task_queue->not_full);
        pthread_mutex_unlock(&lb->task_queue->mutex);
        usleep(1000);  // Short sleep between attempts
    }
    
    log_message(LOG_INFO, "Initiating load balancer shutdown 3");
    
    // Cancel pending tasks before joining threads
    cancel_pending_tasks(lb);
    
    log_message(LOG_INFO, "Initiating load balancer shutdown 4");
    
    // Set a reasonable timeout for thread joining
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 5;  // 5 second timeout
    
    // Try to join monitor thread with timeout
    int monitor_ret = pthread_timedjoin_np(lb->monitor_thread, NULL, &timeout);
    if (monitor_ret != 0) {
        log_message(LOG_WARNING, "Monitor thread join timed out, forcing cancellation");
        pthread_cancel(lb->monitor_thread);
    }
    
    log_message(LOG_INFO, "Initiating load balancer shutdown 5");
    
    // Update timeout for scheduler thread
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 5;
    
    // Try to join scheduler thread with timeout
    int scheduler_ret = pthread_timedjoin_np(lb->scheduler_thread, NULL, &timeout);
    if (scheduler_ret != 0) {
        log_message(LOG_WARNING, "Scheduler thread join timed out, forcing cancellation");
        pthread_cancel(lb->scheduler_thread);
    }
    
    log_message(LOG_INFO, "Initiating load balancer shutdown 6");
    
    // Set another timeout for remaining tasks
    struct timespec wait_start, wait_current;
    clock_gettime(CLOCK_REALTIME, &wait_start);
    
    // Wait for tasks with timeout
    while (1) {
        pthread_mutex_lock(&lb->task_queue->mutex);
        int active_tasks = total_active_tasks;
        pthread_mutex_unlock(&lb->task_queue->mutex);

        if (active_tasks == 0) break;
        
        clock_gettime(CLOCK_REALTIME, &wait_current);
        if (wait_current.tv_sec - wait_start.tv_sec > 5) {  // 5 second timeout
            log_message(LOG_WARNING, "Task completion timed out, forcing shutdown");
            break;
        }
        usleep(100000);  // 100ms sleep between checks
    }
    
    log_message(LOG_INFO, "Load balancer stopped successfully");
}