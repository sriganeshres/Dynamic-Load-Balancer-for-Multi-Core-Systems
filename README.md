# Dynamic Load Balancer for Multi-Core Systems

## Introduction
The dynamic load balancer project aims to create an efficient and intelligent system for distributing tasks across multiple CPU cores. This load balancer ensures that system resources are utilized optimally by allocating tasks to cores based on their real-time workload, preventing any single core from becoming overburdened while others remain underutilized.

## Project Structure
The project is organized into the following directories and files:

### include/
This directory contains the header files that define the project's main components and interfaces.

1. **task.h**: Defines the `Task` structure and related functions for creating, destroying, and calculating the duration of tasks.
2. **core.h**: Defines the `Core` structure and functions for managing individual CPU cores, including adding tasks, retrieving the next task, and getting the current load.
3. **task_queue.h**: Defines a thread-safe queue implementation for managing tasks.
4. **load_balancer.h**: Defines the main `LoadBalancer` structure and functions for creating, running, and managing the load balancer.

### src/
This directory contains the implementation files for the project's components.

1. **task.c**: Implements the functions defined in `task.h`.
2. **core.c**: Implements the functions defined in `core.h`.
3. **task_queue.c**: Implements the functions defined in `task_queue.h`.
4. **load_balancer.c**: Implements the functions defined in `load_balancer.h`.
5. **main.c**: Provides a sample usage example for the load balancer.

### Makefile
The Makefile is used to build the project and manage the compilation process.

## Key Components

### Task
The `Task` structure represents a unit of work to be processed by the load balancer. Each task has the following attributes:

- `task_id`: A unique identifier for the task.
- `workload`: The simulated workload of the task, measured in seconds.
- `assigned_core`: The ID of the CPU core that the task is currently assigned to.
- `start_time`: The time when the task started processing.
- `completion_time`: The time when the task finished processing.

The `task.c` file provides functions for creating, destroying, and calculating the duration of a task.

### Core
The `Core` structure represents a single CPU core and its associated properties. Each core has the following attributes:

- `core_id`: A unique identifier for the core.
- `current_load`: The current workload of the core, measured in seconds.
- `tasks`: A thread-safe queue of tasks assigned to the core.
- `total_tasks_processed`: The total number of tasks processed by the core.
- `is_active`: A flag indicating whether the core is currently active and processing tasks.
- `lock`: A mutex lock for thread-safe access to the core's properties.

The `core.c` file provides functions for creating, destroying, adding tasks, retrieving the next task, and getting the current load of a core.

### Task Queue
The `TaskQueue` structure is a thread-safe queue implementation used to manage the tasks. It has the following attributes:

- `head`: A pointer to the first node in the queue.
- `tail`: A pointer to the last node in the queue.
- `size`: The number of tasks currently in the queue.
- `lock`: A mutex lock for thread-safe access to the queue.

The `task_queue.c` file provides functions for creating, destroying, pushing, popping, and checking the emptiness of the queue.

### Load Balancer
The `LoadBalancer` structure is the main component that manages the overall load balancing process. It has the following attributes:

- `cores`: An array of `Core` pointers representing the available CPU cores.
- `num_cores`: The number of CPU cores in the system.
- `task_queue`: A queue of tasks waiting to be assigned to cores.
- `completed_tasks`: A queue of tasks that have been completed.
- `total_tasks`: The total number of tasks generated during the simulation.
- `core_threads`: An array of thread IDs for the core processing threads.
- `balancer_thread`: The thread ID for the load balancing thread.
- `is_running`: A flag indicating whether the load balancer is currently running.

The `load_balancer.c` file provides functions for creating, destroying, running the simulation, and printing the performance statistics.

## Load Balancing Algorithm
The load balancing algorithm in this project uses a round-robin approach to select the next core for task assignment. The `select_optimal_core` function in `load_balancer.c` is responsible for this task selection:

```c
static int current_core_index = 0;

static Core* select_optimal_core(LoadBalancer* balancer) {
    Core* optimal_core = balancer->cores[current_core_index];
    current_core_index = (current_core_index + 1) % balancer->num_cores;
    return optimal_core;
}
```

The `current_core_index` variable keeps track of the next core to be selected. The `select_optimal_core` function simply returns the core at the `current_core_index` position and then increments the index to the next core, wrapping around to the beginning of the `cores` array if necessary.

This round-robin approach ensures that tasks are distributed evenly across all available cores, regardless of their current workload. While this is a simple and effective load balancing strategy, more sophisticated algorithms could be implemented to account for factors such as core utilization, task priority, or task dependencies.

## Simulation Execution
The main entry point of the project is the `main.c` file, which demonstrates how to use the load balancer. Here's a breakdown of the steps in the `main()` function:

1. Seed the random number generator with the current time.
2. Create a `LoadBalancer` instance with 4 CPU cores.
3. Start the load balancer simulation for 30 seconds, with a task generation rate of approximately 2 tasks per second.
4. Print the performance statistics, including the total tasks generated, the number of tasks processed by each core, and the total number of tasks completed.
5. Destroy the `LoadBalancer` instance and exit the program.

The `balancer_run_simulation` function is responsible for the main simulation logic:

1. Start the core processing threads, each of which continuously checks the core's task queue and processes any available tasks.
2. Start the load balancing thread, which continuously checks the task queue and assigns new tasks to the least loaded core using the `select_optimal_core` function.
3. Generate new tasks at the specified rate and add them to the task queue.
4. After the specified duration, signal the core and load balancing threads to stop, and wait for them to finish.

The `balancer_print_stats` function is used to print the performance statistics at the end of the simulation.

## Building and Running the Project
To build and run the project, follow these steps:

1. Navigate to the project directory in your terminal.
2. Run the following commands to build the project:
   ```
   mkdir obj bin
   make
   ```
3. Run the load balancer simulation:
   ```
   ./bin/load_balancer
   ```

This will output the simulation results, including the total tasks generated, the number of tasks processed by each core, and the total number of tasks completed.

## Conclusion
The dynamic load balancer project provides a robust and customizable framework for distributing tasks across multiple CPU cores. The modular design, with clear separation of concerns, makes it easy to extend or modify the implementation to suit specific requirements. The detailed documentation should help you understand the project's architecture and implementation, enabling you to build upon it or adapt it to your own needs.