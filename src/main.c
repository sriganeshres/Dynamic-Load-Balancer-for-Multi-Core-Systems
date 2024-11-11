#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "load_balancer.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    // Create load balancer with 4 cores
    LoadBalancer* balancer = balancer_create(4);
    
    // Run simulation for 30 seconds with task generation rate of 2
    printf("Starting load balancer simulation...\n");
    balancer_run_simulation(balancer, 30, 2.0);
    
    // Print statistics
    balancer_print_stats(balancer);
    
    // Clean up
    balancer_destroy(balancer);
    
    return 0;
}