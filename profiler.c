//
// Created by alex on 14/10/2015.
//

#include "profiler.h"
#include <lua.h>
#include <luadebug.h>
#include <stdio.h>
#include <stdlib.h>
#include "measure.h"

int META_REF = 0;
int STACK_INDEX = 0;
int STACK_SIZE = 0;
int MEM_BLOCKSIZE = 100;


/* METADATA */
Meta *get_metadata_array(lua_State *L) {
    return lua_getuserdata(L, lua_getref(L, META_REF));
}

/* CALL FUNCTION HOOK */
void callhook(lua_State *L, lua_Function func, char *file, int line) {
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

void profile_end(lua_State *L) {
    Meta *meta = get_metadata_array(L);
    free(meta);
}

static char *fill_buff(char *buffer, int buffsize, char c) {
    if (buffsize > 1)
        memset(buffer, c, (size_t) buffsize);
    buffer[buffsize - 1] = '\0';
    return buffer;
}

void profile_show(lua_State *L) {
    Meta *array = get_metadata_array(L);
    Meta meta;

    int index, buffsize;

    for (index = 0; index < STACK_INDEX; index++) {
        meta = array[index];

        buffsize = meta.stack_level + 1;
        char buffer[buffsize];

        printf("%s(%i) name: %s (%s) source: (%s) spent: (%.3f s) \n",
               fill_buff(buffer, buffsize, '\t'),
               meta.line,
               meta.fun_name,
               meta.fun_scope,
               meta.func_file,
               meta.measure->time_spent
        );
    }
}

LUA_API int luaopen_profiler(lua_State *L) {
    lua_register(L, "profile_end", profile_end);
    lua_register(L, "profile_show", profile_show);

    Meta *meta = malloc(MEM_BLOCKSIZE * sizeof(Meta));

    lua_pushuserdata(L, meta);
    META_REF = lua_ref(L, 1);

    lua_setcallhook(L, callhook);
    return 0;
}
