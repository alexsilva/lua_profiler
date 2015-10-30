//
// Created by alex on 14/10/2015.
//

#ifndef LUA_PROFILER_PROFILER_H
#define LUA_PROFILER_PROFILER_H

#include <lua.h>
#include <stdbool.h>

#if defined(_WIN32) //  Microsoft
#define LUA_API __declspec(dllexport)
#else //  GCC
#define LUA_API __attribute__((visibility("default")))
#endif

extern int PCONFIG_REF;

typedef struct STACK_INFO {
    int index;
    int size;
} StackInfo;

typedef struct META_INFO {
    int ref;
    int mbuffsize;
} MetaInfo;

typedef struct PROFILE_CONFIG {
    char *resource_dir;
    bool started;

    MetaInfo *meta_info;

    float record_limit;

    StackInfo *stack_info;
} ProfileConfig;


LUA_API int luaopen_python(lua_State *L);

#endif //LUA_PROFILER_PROFILER_H
