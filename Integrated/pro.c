#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* cfg;

void read_cfg()
{
    FILE* cfg_file = fopen(cfg, "r");

    char* line = calloc(100, sizeof(char));
    char* tokens = calloc(100, sizeof(char));

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
    int value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ISIZE, value);

    fscanf(cfg_file, "%s", line);
    // associativity = 1
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    int value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ASSOC, value);

    fscanf(cfg_file, "%s", line);
    // replacement_policy = lru

    fscanf(cfg_file, "%s", line);
    // writethrough = 0

    fscanf(cfg_file, "%s", line);
    // block_size = 16
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    int value = atoi(tokens);
    set_cache_param(CACHE_PARAM_BLOCK_SIZE, value);

    
    fscanf(cfg_file, "%s", line);
    // [perf_model/l1_dcache]

    fscanf(cfg_file, "%s", line);
    // perfect = false

    fscanf(cfg_file, "%s", line);
    // cache_size = 8
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    int value = atoi(tokens);
    set_cache_param(CACHE_PARAM_DSIZE, value);

    fscanf(cfg_file, "%s", line);
    // associativity = 1
    tokens = strtok(line, " = ");
    tokens = strtok(NULL, "\n");
    int value = atoi(tokens);
    set_cache_param(CACHE_PARAM_ASSOC, value);


    

} 