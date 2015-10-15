//
// Created by alex on 14/10/2015.
//

#ifndef LUA_PROFILER_PROFILER_H
#define LUA_PROFILER_PROFILER_H

#include <lua.h>

#if defined(_WIN32) //  Microsoft
#define LUA_API __declspec(dllexport)
#else //  GCC
#define LUA_API __attribute__((visibility("default")))
#endif

LUA_API int luaopen_python(lua_State *L);

#endif //LUA_PROFILER_PROFILER_H
