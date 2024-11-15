#include "cpu_stats.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

CPUMonitor* init_cpu_monitor(LoadBalancerConfig* config) {
    CPUMonitor* monitor = malloc(sizeof(CPUMonitor));
    if (!monitor) return NULL;
    
    monitor->num_cpus = config->num_cpus;
    monitor->config = config;
    monitor->stats = malloc(sizeof(CPUStats) * monitor->num_cpus);
    
    if (!monitor->stats) {
        free(monitor);
        return NULL;
    }
    
    for (int i = 0; i < monitor->num_cpus; i++) {
        monitor->stats[i].cpu_id = i;
        monitor->stats[i].current_usage = 0.0;
        monitor->stats[i].usage_history = malloc(sizeof(double) * config->load_history_size);
        monitor->stats[i].history_index = 0;
        monitor->stats[i].active_tasks = 0;
        memset(monitor->stats[i].usage_history, 0, sizeof(double) * config->load_history_size);
    }
    
    return monitor;
}

void update_cpu_stats(CPUMonitor* monitor) {
    FILE* fp = fopen("/proc/stat", "r");
    if (!fp) {
        log_message(LOG_ERROR, "Failed to open /proc/stat");
        return;
    }
    
    char line[256];
    // Skip first line (aggregate CPU stats)
    fgets(line, sizeof(line), fp);
    
    for (int i = 0; i < monitor->num_cpus; i++) {
        if (fgets(line, sizeof(line), fp)) {
            CPUStats* cpu = &monitor->stats[i];
            uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
            
            sscanf(line, "cpu%*d %lu %lu %lu %lu %lu %lu %lu %lu",
                   &user, &nice, &system, &idle,
                   &iowait, &irq, &softirq, &steal);
            
            uint64_t prev_idle = cpu->idle_time + cpu->iowait_time;
            uint64_t idle_time = idle + iowait;
            
            uint64_t prev_total = cpu->user_time + cpu->nice_time +
                                cpu->system_time + prev_idle +
                                cpu->irq_time + cpu->softirq_time +
                                cpu->steal_time;
            
            uint64_t total_time = user + nice + system + idle_time +
                                irq + softirq + steal;
            
            uint64_t total_delta = total_time - prev_total;
            uint64_t idle_delta = idle_time - prev_idle;
            
            cpu->current_usage = 100.0 * (1.0 - ((double)idle_delta / total_delta));
            
            // Update history
            cpu->usage_history[cpu->history_index] = cpu->current_usage;
            cpu->history_index = (cpu->history_index + 1) % monitor->config->load_history_size;
            
            // Update raw stats
            cpu->user_time = user;
            cpu->nice_time = nice;
            cpu->system_time = system;
            cpu->idle_time = idle;
            cpu->iowait_time = iowait;
            cpu->irq_time = irq;
            cpu->softirq_time = softirq;
            cpu->steal_time = steal;
            
            if (monitor->config->enable_load_prediction) {
                cpu->predicted_load = predict_cpu_load(cpu);
            }
        }
    }
    
    fclose(fp);
}

double predict_cpu_load(CPUStats* cpu) {
    // Simple moving average prediction
    double sum = 0.0;
    int count = 0;
    
    for (int i = 0; i < cpu->history_index; i++) {
        sum += cpu->usage_history[i];
        count++;
    }
    
    return count > 0 ? sum / count : cpu->current_usage;
}


void print_cpu_stats(CPUMonitor* monitor) {
    if (monitor == NULL || monitor->stats == NULL) {
        printf("CPUMonitor is not initialized.\n");
        return;
    }

    printf("CPU Usage Statistics:\n");
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < monitor->num_cpus; ++i) {
        CPUStats* cpu = &monitor->stats[i];
        
        printf("CPU ID: %d\n", cpu->cpu_id);
        printf("  Current Usage: %.2f%%\n", cpu->current_usage);
        printf("  User Time: %lu\n", cpu->user_time);
        printf("  Nice Time: %lu\n", cpu->nice_time);
        printf("  System Time: %lu\n", cpu->system_time);
        printf("  Idle Time: %lu\n", cpu->idle_time);
        printf("  IOWait Time: %lu\n", cpu->iowait_time);
        printf("  IRQ Time: %lu\n", cpu->irq_time);
        printf("  SoftIRQ Time: %lu\n", cpu->softirq_time);
        printf("  Steal Time: %lu\n", cpu->steal_time);
        printf("  Temperature: %.2f°C\n", cpu->temperature);
        printf("  Predicted Load: %.2f%%\n", cpu->predicted_load);
        printf("  Active Tasks: %d\n", cpu->active_tasks);
        
        if (cpu->usage_history != NULL) {
            printf("  Usage History (last 5 samples): ");
            for (int j = 0; j < 5 && j < cpu->history_index; ++j) {
                printf("%.2f%% ", cpu->usage_history[j]);
            }
            printf("\n");
        }

        printf("------------------------------------------------------------\n");
    }
}

#include <stdlib.h>

void cleanup_cpu_monitor(CPUMonitor* monitor) {
    if (monitor == NULL) {
        return; // Nothing to clean up
    }

    if (monitor->stats != NULL) {
        for (int i = 0; i < monitor->num_cpus; ++i) {
            CPUStats* cpu = &monitor->stats[i];
            
            // Free usage history if allocated
            if (cpu->usage_history != NULL) {
                free(cpu->usage_history);
                cpu->usage_history = NULL;
            }
        }
        
        // Free the stats array
        free(monitor->stats);
        monitor->stats = NULL;
    }

    // Free the configuration if allocated
    if (monitor->config != NULL) {
        free(monitor->config);
        monitor->config = NULL;
    }

    // Reset the number of CPUs
    monitor->num_cpus = 0;
}
