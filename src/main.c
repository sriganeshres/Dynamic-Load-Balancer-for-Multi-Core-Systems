#include "load_balancer.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

static LoadBalancer* lb = NULL;
static volatile int running = 1;

void signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nReceived Ctrl+C, initiating graceful shutdown...\n");
        if (lb) {
            log_message(LOG_INFO, "Received shutdown signal, initiating graceful shutdown");
            stop_load_balancer(lb);
        }
        running = 0;
    }
}

// Modified CPU task that runs for 1-3 seconds
void cpu_task(void* arg) {
    int task_id = *(int*)arg;
    time_t start_time = time(NULL);
    
    // Generate random duration between 1-3 seconds
    int duration = (rand() % 3) + 1;
    double result = 0.0;
    
    time_t current_time;
    do {
        // Perform some CPU-intensive calculations
        for (int i = 0; i < 10000; i++) {
            result += rand() / (double)RAND_MAX;
        }
        current_time = time(NULL);
    } while (difftime(current_time, start_time) < duration);
    
    log_message(LOG_INFO, "Task %d completed after %d seconds", task_id, duration);
    free(arg);
}

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s <num_cores> <num_tasks>\n", program_name);
    fprintf(stderr, "  num_cores: Number of CPU cores to use (1-%ld)\n", sysconf(_SC_NPROCESSORS_ONLN));
    fprintf(stderr, "  num_tasks: Number of tasks to generate\n");
}

int main(int argc, char** argv) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    int num_cores = atoi(argv[1]);
    int num_tasks = atoi(argv[2]);
    
    // Validate number of cores
    int max_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores < 1 || num_cores > max_cores) {
        fprintf(stderr, "Error: Invalid number of cores. Must be between 1 and %d\n", max_cores);
        return 1;
    }
    
    // Validate number of tasks
    if (num_tasks < 1) {
        fprintf(stderr, "Error: Invalid number of tasks. Must be greater than 0\n");
        return 1;
    }
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize load balancer with custom configuration
    LoadBalancerConfig* config = init_default_config();
    if (!config) {
        fprintf(stderr, "Failed to initialize configuration\n");
        return 1;
    }
    
    // Modify configuration for our needs
    config->max_tasks = num_tasks;
    config->monitoring_interval_ms = 500;  // Monitor every 500ms
    config->enable_detailed_logging = 1;
    config->num_cpus = num_cores;

    lb = init_load_balancer(config);
    if (!lb) {
        fprintf(stderr, "Failed to initialize load balancer\n");
        fflush(stderr);
        free_config(config);
        return 1;
    }
    
    // Start the load balancer
    start_load_balancer(lb);
    fprintf(stdout, "Started load balancer with %d cores and %d tasks\n", num_cores, num_tasks);
    fflush(stdout);
    
    // Submit tasks
    for (int i = 0; i < num_tasks && running; i++) {
        int* task_id = malloc(sizeof(int));
        *task_id = i + 1;
        
        // Assign random priority
        TaskPriority priority = (TaskPriority)(rand() % 3);  // 0=LOW, 1=MEDIUM, 2=HIGH
        
        if (submit_task(lb, cpu_task, task_id, priority) == 0) {
            log_message(LOG_INFO, "Submitted task %d with priority %d", *task_id, priority);
            printf("Submitted task %d\n", *task_id);
        } else {
            log_message(LOG_ERROR, "Failed to submit task %d", *task_id);
            free(task_id);
        }
        
        // Small delay between task submissions to prevent overwhelming the system
        usleep(100000);  // 100ms delay
    }
    
    // Wait for completion or interrupt
    fprintf(stdout, "All tasks submitted. Press Ctrl+C to stop...\n");
    fflush(stdout);
    while (running) {
        sleep(1);
    }
    if(!running) { 
        log_message(LOG_INFO, "Load balancer stopped running gracefully");
    }
    // Cleanup
    printf("Cleaning up...\n");
    // stop_load_balancer(lb);
    // cleanup_load_balancer(lb);
    free_config(config);
    
    printf("Successfully terminated.\n");
    return 0;
}