//
// Created by alex on 30/10/2015.
//

#ifndef LUA_PROFILER_UTILS_H
#define LUA_PROFILER_UTILS_H

#include <stdbool.h>
#include <lua.h>

bool is_dir(char *path);
int lua_gettop(lua_State *L);

#endif //LUA_PROFILER_UTILS_H
