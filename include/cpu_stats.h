#ifndef CPU_STATS_H
#define CPU_STATS_H

#include <stdint.h>
#include "config.h"

typedef struct {
    int cpu_id;
    double current_usage;
    double *usage_history;
    int history_index;
    uint64_t user_time;
    uint64_t nice_time;
    uint64_t system_time;
    uint64_t idle_time;
    uint64_t iowait_time;
    uint64_t irq_time;
    uint64_t softirq_time;
    uint64_t steal_time;
    double temperature;
    double predicted_load;
    int active_tasks;
} CPUStats;

typedef struct {
    CPUStats* stats;
    int num_cpus;
    LoadBalancerConfig* config;
} CPUMonitor;

CPUMonitor* init_cpu_monitor(LoadBalancerConfig* config);
void update_cpu_stats(CPUMonitor* monitor);
double predict_cpu_load(CPUStats* cpu);
void print_cpu_stats(CPUMonitor* monitor);
void cleanup_cpu_monitor(CPUMonitor* monitor);

#endif