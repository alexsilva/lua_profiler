//
// Created by alex on 14/10/2015.
//

#include "profiler.h"
#include <lua.h>
#include <luadebug.h>
#include <stdio.h>
#include <stdlib.h>
#include <lauxlib.h>
#include "measure.h"
#include "stack.h"

bool PROFILE_INIT = false;

int META_REF = 0;
int STACK_INDEX = 0;
int STACK_SIZE = 0;
int MEM_BLOCKSIZE = 100;
clock_t PROFILE_START_TIME;

/* the stack */
static STACK stack = NULL;
static STACK_RECORD stack_record;
static STACK_RECORD stack_recordout;

/* METADATA */
static Meta *get_metadata_array(lua_State *L) {
    return lua_getuserdata(L, lua_getref(L, META_REF));
}

static void free_array(Meta *array) {
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
    if (!array) return; // check if exists (call profile_end ?)

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

    char *func_name;
    char *func_scope;

    if (lua_isfunction(L, func)) {
        func_scope = lua_getobjname(L, func, &func_name);

        Measure *measure = malloc(sizeof(Measure));
        measure->begin = clock();

        Meta *meta = malloc(sizeof(Meta));

        meta->fun_name = func_name;
        meta->fun_scope = func_scope;
        meta->func_file = file;
        meta->stack_level = STACK_SIZE;
        meta->line = line;

        meta->measure = measure;
        stack_record.meta = meta;
        stack_record.index = STACK_INDEX;

        push(&stack, stack_record);
        //printf("push (%p)\n", stack);

        STACK_SIZE++;
        STACK_INDEX++;

    } else if (STACK_SIZE > 0) {
        stack_recordout = pop(&stack);
        //printf("stack_recordout (%p)\n", stack_recordout);

        Meta meta = *stack_recordout.meta;
        array[stack_recordout.index] = meta;
        free(stack_recordout.meta);

        meta.measure->end = clock();
        meta.measure->time_spent = calc_time_spent(meta.measure);

        STACK_SIZE--;
    }
}

static void profile_start(lua_State *L) {
    if (PROFILE_INIT)
        return; // already started.
    Meta *meta = malloc(MEM_BLOCKSIZE * sizeof(Meta));

    lua_pushuserdata(L, meta);
    META_REF = lua_ref(L, 1);

    lua_setcallhook(L, callhook);
    PROFILE_START_TIME = clock();
    PROFILE_INIT = true;
}

static void profile_end(lua_State *L) {
    check_start(L);
    lua_setcallhook(L, NULL); // disable hook
    free_array(get_metadata_array(L));
    lua_unref(L, META_REF); // Remove the old reference (new block of memory).
    PROFILE_INIT = false;
}

char *repeat_str(char *str, size_t count) {
    if (count == 0) return NULL;
    char *ret = malloc((strlen(str) * count) + 1);
    if (ret == NULL) return NULL;
    strcpy(ret, str);
    while (--count > 0) {
        strcat(ret, str);
    }
    return ret;
}

static void profile_show_text(lua_State *L) {
    check_start(L);
    // Spent runtime so far.
    float total_spent = calc_elapsed_time(PROFILE_START_TIME, clock());

    // Lua args
    lua_Object lobj = lua_getparam(L, 1);
    char *breakln = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\n";

    lobj = lua_getparam(L, 2);
    char *offsetc = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\t";
    char *offsettext;

    Meta meta;
    Meta *array = get_metadata_array(L);
    int index;

    for (index = 0; index < STACK_INDEX - 1; index++) {
        meta = array[index];

        offsettext = repeat_str(offsetc, (size_t) meta.stack_level);
        if (!offsettext) offsettext = ""; //  security

        printf("%s %i | %s (%s) source: (%s) time: (%.3fs)%s",
               offsettext,
               meta.line,
               meta.fun_name,
               meta.fun_scope,
               meta.func_file,
               meta.measure->time_spent,
               breakln
        );
        free(offsettext);
    }
    printf("TOTAL TIME SPENT: %.3f%s", total_spent, breakln);
}

LUA_API int luaopen_profiler(lua_State *L) {
    lua_register(L, "profile_start", profile_start);
    lua_register(L, "profile_end", profile_end);
    lua_register(L, "profile_show_text", profile_show_text);
    return 0;
}
