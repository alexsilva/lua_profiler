//
// Created by alex on 16/10/2015.
//

#ifndef LUA_PROFILER_RENDER_H
#define LUA_PROFILER_RENDER_H

#include <lua.h>
#include "measure.h"
#include "profiler.h"

void render_html(lua_State *L, ProfileConfig *pconfig, Meta **array, int size);
void render_text(lua_State *L, ProfileConfig *pconfig, Meta **array, int size);
void render_json(lua_State *L, ProfileConfig *pconfig, Meta **array, int size);

#endif //LUA_PROFILER_RENDER_H
