#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

LoadBalancerConfig* init_default_config(void) {
    LoadBalancerConfig* config = malloc(sizeof(LoadBalancerConfig));
    if (!config) return NULL;
    
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

LoadBalancerConfig* load_config(const char* config_path) {
    // Implementation omitted for brevity
    return init_default_config();
}

void free_config(LoadBalancerConfig* config) {
    if (config) {
        free(config->log_file_path);
        free(config);
    }
}