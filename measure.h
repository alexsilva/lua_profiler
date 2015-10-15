//
// Created by alex on 14/10/2015.
//

#ifndef LUA_PROFILER_MEASURE_H
#define LUA_PROFILER_MEASURE_H

#include <time.h>

struct MEASURE {
    clock_t begin, end;
    double time_spent;
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

double calc_time_spent(Measure *measure);

extern int META_REF;
extern int STACK_INDEX;
extern int STACK_SIZE;

#endif //LUA_PROFILER_MEASURE_H
