#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* cfg;

void read_cfg()
{
    FILE* cfg_file = fopen(cfg, "r");

    char* line = calloc(100, sizeof(char));
    char* tokens = calloc(100, sizeof(char));
    int value;

    fscanf(cfg_file, "%s", line);
    // Comment

    fscanf(cfg_file, "%s", line);
    // [perf_model/l1_icache]

    fscanf(cfg_file, "%s", line);
    // perfect = false

    fscanf(cfg_file, "%s", line);
    // cache_size = 8
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_SIZE_I, value);

    fscanf(cfg_file, "%s", line);
    // associativity = 1
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ASSOC_I, value);

    fscanf(cfg_file, "%s", line);
    // replacement_policy = lru
    
    fscanf(cfg_file, "%s", line);
    // writethrough = 0
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_WRITETHROUGH_I, value);

    fscanf(cfg_file, "%s", line);
    // block_size = 16
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_BLOCK_SIZE_I, value);
    
    fscanf(cfg_file, "%s", line);
    // [perf_model/l1_dcache]

    fscanf(cfg_file, "%s", line);
    // perfect = false

    fscanf(cfg_file, "%s", line);
    // cache_size = 8
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_SIZE_D, value);

    fscanf(cfg_file, "%s", line);
    // associativity = 1
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ASSOC_D, value);

    fscanf(cfg_file, "%s", line);
    // replacement_policy = lru
    
    fscanf(cfg_file, "%s", line);
    // writethrough = 0
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_WRITETHROUGH_D, value);

    fscanf(cfg_file, "%s", line);
    // block_size = 16
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    set_cache_param(CACHE_PARAM_BLOCK_SIZE_D, value);

    fscanf(cfg_file, "%s", line);
    // [perf_model/core]
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    // set_cache_param(CACHE_PARAM_BLOCK_SIZE_D, value);


    fscanf(cfg_file, "%s", line);
    // [perf_model/dram]
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    value = atoi(tokens);
    // set_cache_param(CACHE_PARAM_BLOCK_SIZE_D, value);

} 