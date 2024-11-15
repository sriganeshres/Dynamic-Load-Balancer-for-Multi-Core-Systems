# CPU Load Balancer Documentation

## Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Components](#components)
4. [Configuration](#configuration)
5. [Core Features](#core-features)
6. [Building and Installation](#building-and-installation)
7. [Usage](#usage)
8. [Technical Details](#technical-details)

## Overview

The CPU Load Balancer is a sophisticated system designed to distribute computational tasks across multiple CPU cores efficiently. It provides dynamic load balancing, task prioritization, and real-time CPU monitoring capabilities.

File Structure:

```
.
├── build
│   ├── CMakeCache.txt
│   ├── cmake_install.cmake
│   ├── cpu_balancer
│   └── Makefile
├── CMakeLists.txt
├── config
│   └── cpu_balancer.conf
├── cpu_balancer.log
├── include
│   ├── config.h
│   ├── cpu_stats.h
│   ├── load_balancer.h
│   ├── logger.h
│   ├── task.h
│   └── task_queue.h
├── Makefile
├── README.md
├── Red.md
└── src
    ├── config.c
    ├── cpu_stats.c
    ├── load_balancer.c
    ├── logger.c
    ├── main.c
    ├── task.c
    └── task_queue.c
```

### Key Features
- Dynamic task distribution across multiple CPU cores
- Real-time CPU load monitoring and statistics
- Priority-based task scheduling
- Configurable load thresholds and monitoring intervals
- Detailed logging system
- Graceful shutdown handling
- Task queue management

## Architecture

The system follows a modular architecture with the following main components:

```
Load Balancer
    ├── CPU Monitor
    │   └── CPU Stats
    ├── Task Queue
    │   └── Tasks
    ├── Configuration
    └── Logger
```

### Threading Model
- Monitor Thread: Continuously monitors CPU statistics
- Scheduler Thread: Handles task distribution
- Task Threads: Individual threads for each task execution

## Components

### 1. Load Balancer (`load_balancer.h`)
The core component that orchestrates task distribution and system management.

#### Key Functions:
```c
LoadBalancer* init_load_balancer(LoadBalancerConfig* config);
int submit_task(LoadBalancer* lb, void (*function)(void*), void* args, TaskPriority priority);
void start_load_balancer(LoadBalancer* lb);
void stop_load_balancer(LoadBalancer* lb);
```

### 2. CPU Monitor (`cpu_stats.h`)
Handles CPU statistics collection and analysis.

#### CPU Stats Structure:
```c
typedef struct {
    int cpu_id;
    double current_usage;
    double *usage_history;
    int history_index;
    uint64_t user_time;
    uint64_t system_time;
    uint64_t idle_time;
    double temperature;
    double predicted_load;
    int active_tasks;
} CPUStats;
```

### 3. Task Queue (`task_queue.h`)
Manages task scheduling and queuing.

#### Features:
- Circular buffer implementation
- Thread-safe operations
- Priority-based ordering
- Dynamic capacity management

### 4. Task Management (`task.h`)
Defines task structure and handling.

#### Task Priority Levels:
```c
typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_CRITICAL = 3
} TaskPriority;
```

#### Task States:
```c
typedef enum {
    STATUS_PENDING,
    STATUS_RUNNING,
    STATUS_COMPLETED,
    STATUS_FAILED
} TaskStatus;
```

## Configuration

### Default Configuration
```c
LoadBalancerConfig* init_default_config(void) {
    LoadBalancerConfig* config = malloc(sizeof(LoadBalancerConfig));
    config->max_tasks = 10;
    config->monitoring_interval_ms = 100;
    config->high_load_threshold = 80.0;
    config->low_load_threshold = 20.0;
    config->load_history_size = 10;
    config->enable_load_prediction = 1;
    config->enable_detailed_logging = 1;
    config->log_file_path = strdup("./cpu_balancer.log");
    config->rebalance_threshold = 30;
    config->min_task_runtime_ms = 5;
    return config;
}
```

### Configuration Parameters
- `max_tasks`: Maximum number of tasks in queue
- `monitoring_interval_ms`: CPU monitoring frequency
- `high_load_threshold`: Upper CPU load threshold (%)
- `low_load_threshold`: Lower CPU load threshold (%)
- `load_history_size`: Number of historical load samples
- `enable_load_prediction`: Enable predictive load balancing
- `enable_detailed_logging`: Enable verbose logging
- `rebalance_threshold`: Load difference triggering rebalance
- `min_task_runtime_ms`: Minimum task execution time

## Core Features

### 1. CPU Load Monitoring
The system continuously monitors CPU usage through `/proc/stat`:
```c
void update_cpu_stats(CPUMonitor* monitor) {
    // Read CPU statistics from /proc/stat
    // Calculate usage percentages
    // Update history and predictions
}
```

### 2. Load Prediction
Implements simple moving average prediction:
```c
double predict_cpu_load(CPUStats* cpu) {
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < cpu->history_index; i++) {
        sum += cpu->usage_history[i];
        count++;
    }
    return count > 0 ? sum / count : cpu->current_usage;
}
```

### 3. Task Distribution Algorithm
The system finds the optimal CPU for task execution:
```c
int find_best_cpu(CPUMonitor* monitor) {
    int best_cpu = -1;
    double lowest_load = 999.9;
    
    for (int i = 0; i < monitor->num_cpus; i++) {
        double effective_load = monitor->stats[i].current_usage;
        if (monitor->config->enable_load_prediction) {
            effective_load = (effective_load + monitor->stats[i].predicted_load) / 2;
        }
        effective_load += (monitor->stats[i].active_tasks * 10);
        
        if (effective_load < lowest_load) {
            lowest_load = effective_load;
            best_cpu = i;
        }
    }
    return best_cpu;
}
```

## Building and Installation

### Prerequisites
- CMake (>= 3.14)
- C11 compatible compiler
- pthread library
- json-c library
- pkg-config

### Build Steps
```bash
mkdir build
cd build
cmake ..
make
```

### Installation
```bash
sudo make install
```

## Usage

### Basic Usage
```bash
./cpu_balancer <num_cores> <num_tasks>
```

### Example
```bash
./cpu_balancer 4 20  # Use 4 cores and create 20 tasks
```

### Task Creation Example
```c
void* cpu_task(void* arg) {
    int task_id = *(int*)arg;
    // Task implementation
    free(arg);
    return NULL;
}

// Submit task
int* task_id = malloc(sizeof(int));
*task_id = 1;
submit_task(lb, cpu_task, task_id, PRIORITY_MEDIUM);
```

## Technical Details

### Thread Safety
- Mutex protection for shared resources
- Condition variables for synchronization
- Atomic operations for task ID generation

### Memory Management
- Dynamic allocation for task queue
- Proper cleanup in destructors
- Memory leak prevention through systematic resource tracking

### Error Handling
- Return value checking
- Logging of errors
- Graceful degradation under error conditions

### Shutdown Protocol
1. Signal handler catches SIGINT
2. Sets running flag to false
3. Cancels pending tasks
4. Waits for active tasks completion
5. Joins monitor and scheduler threads
6. Cleans up resources

This documentation provides a comprehensive overview of the CPU Load Balancer system. For specific implementation details, refer to the source code and comments within each file.