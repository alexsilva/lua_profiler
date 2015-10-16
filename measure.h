//
// Created by alex on 14/10/2015.
//

#ifndef LUA_PROFILER_MEASURE_H
#define LUA_PROFILER_MEASURE_H

#include <time.h>
#include <stdbool.h>

struct MEASURE {
    clock_t begin, end;
    float time_spent;
};

typedef struct MEASURE Measure;

struct META {
    Measure *measure;

    char *fun_name;
    char *fun_scope;
    char *func_file;

    int stack_level;
    int line;
};

typedef struct META Meta;

float calc_time_spent(Measure *measure);
float calc_elapsed_time(clock_t start, clock_t end);

extern int META_REF;
extern int STACK_INDEX;
extern int STACK_SIZE;
extern int MEM_BLOCKSIZE;
extern bool PROFILE_INIT;
extern clock_t PROFILE_START_TIME;

#endif //LUA_PROFILER_MEASURE_H
