//
// Created by alex on 14/10/2015.
//

#include "profiler.h"
#include <lua.h>
#include <luadebug.h>
#include <stdio.h>
#include <stdlib.h>
#include "measure.h"

bool PROFILE_INIT = false;

int META_REF = 0;
int STACK_INDEX = 0;
int STACK_SIZE = 0;
int MEM_BLOCKSIZE = 100;


/* METADATA */
static Meta *get_metadata_array(lua_State *L) {
    return lua_getuserdata(L, lua_getref(L, META_REF));
}

static void free_array(Meta * array) {
    int index;
    Meta meta;
    for (index = 0; index < STACK_INDEX; index++) {
        meta = array[index];
        free(meta.measure); // internal cleaning
    }
    // free the main object
    free(array);
}

static void check_start(lua_State *L) {
    if (!PROFILE_INIT)
        lua_error(L, "profile not started. call 'profile_start' first!");
}

/* CALL FUNCTION HOOK */
static void callhook(lua_State *L, lua_Function func, char *file, int line) {
    check_start(L);
    Meta *array = get_metadata_array(L);

    if (STACK_INDEX > MEM_BLOCKSIZE - 1) {
        // Reached memory limit, relocated to double.
        int blocksize = MEM_BLOCKSIZE * 2;

        array = realloc(array, blocksize * sizeof(Meta));

        if (array) {
            lua_unref(L, META_REF); // Remove the old reference (new block of memory).
            lua_pushuserdata(L, array); // Saves the new reference.
            META_REF = lua_ref(L, 1);
            MEM_BLOCKSIZE = blocksize; // Updates the size of the memory block.
        } else {
            lua_error(L, "profiler: out of memory!");
            return; // suppress inspect
        }
    }

    if (lua_isfunction(L, func)) {
        char *func_name;
        char *func_scope;

        func_scope = lua_getobjname(L, func, &func_name);

        //printf("[%i] name: %s | func_scope: %s | source: %s | line: %i\n",
        //       STACK_SIZE, func_name, func_scope, file, line);

        Measure *measure = malloc(sizeof(Measure));
        measure->begin = clock();

        Meta meta;

        meta.fun_name = func_name;
        meta.fun_scope = func_scope;
        meta.func_file = file;
        meta.stack_level = STACK_SIZE;
        meta.line = line;

        meta.measure = measure;

        array[STACK_INDEX] = meta;

        STACK_SIZE++;
        STACK_INDEX++;

    } else if (STACK_SIZE > 0) {
        //printf("%d | %s | %d | INDEX: %d | STSIZE: %d | RINDEX: %d\n", func, file, line, STACK_INDEX, STACK_SIZE, STACK_INDEX - STACK_SIZE);
        int index = STACK_INDEX - STACK_SIZE;

        Meta meta = array[index];
        meta.measure->end = clock();
        meta.measure->time_spent = calc_time_spent(meta.measure);
        STACK_SIZE--;
    }
}

static void profile_start(lua_State *L) {
    Meta *meta = malloc(MEM_BLOCKSIZE * sizeof(Meta));

    lua_pushuserdata(L, meta);
    META_REF = lua_ref(L, 1);

    lua_setcallhook(L, callhook);
    PROFILE_INIT = true;
}

static void profile_end(lua_State *L) {
    check_start(L);
    free_array(get_metadata_array(L));
}

static char *fill_buff(char *buffer, int buffsize, char c) {
    if (buffsize > 1)
        memset(buffer, c, (size_t) buffsize);
    buffer[buffsize - 1] = '\0';
    return buffer;
}

static void profile_show_text(lua_State *L) {
    check_start(L);
    lua_Object lobj = lua_getparam(L, 1);
    char *breakln = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\n";

    Meta meta;
    Meta *array = get_metadata_array(L);

    int index, buffsize;

    for (index = 0; index < STACK_INDEX; index++) {
        meta = array[index];

        buffsize = meta.stack_level + 1;
        char buffer[buffsize];

        printf("%s(%i) name: %s (%s) source: (%s) spent: (%.3f s)%s",
               fill_buff(buffer, buffsize, '\t'),
               meta.line,
               meta.fun_name,
               meta.fun_scope,
               meta.func_file,
               meta.measure->time_spent,
               breakln
        );
    }
}

LUA_API int luaopen_profiler(lua_State *L) {
    lua_register(L, "profile_start", profile_start);
    lua_register(L, "profile_end", profile_end);
    lua_register(L, "profile_show_text", profile_show_text);
    return 0;
}
