//
// Created by alex on 16/10/2015.
//

#ifndef LUA_PROFILER_RENDER_H
#define LUA_PROFILER_RENDER_H

#include <lua.h>
#include "measure.h"

char *render_html(lua_State *L, Meta *array_metadata, int array_size);

#endif //LUA_PROFILER_RENDER_H