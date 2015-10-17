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
#include "render.h"

bool PROFILE_INIT = false;

int META_REF = 0;
int STACK_INDEX = 0;
int STACK_SIZE = 0;
int MEM_BLOCKSIZE = 100;
clock_t PROFILE_START_TIME;

/* the stack */
static STACK stack = NULL;
static STACK_RECORD stack_record;

/* METADATA */
static Meta **get_metadata_array(lua_State *L) {
    return lua_getuserdata(L, lua_getref(L, META_REF));
}

/* CLEANUP */
static void free_array(Meta **array) {
    int index;
    Meta *meta;
    for (index = 0; index < STACK_INDEX; index++) {
        meta = array[index];
        free(meta->measure); // internal cleaning
        free(meta->children);
        free(meta);
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
    Meta **array = get_metadata_array(L);
    if (!array) return; // check if exists (call profile_end ?)

    if (STACK_INDEX > MEM_BLOCKSIZE - 1) {
        // Reached memory limit, relocated to double.
        int blocksize = MEM_BLOCKSIZE * 2;

        array = realloc(array, blocksize * sizeof(Meta **));

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
        Meta *meta = (Meta *) malloc(sizeof(Meta));

        meta->fun_name = func_name ? func_name : "unnamed";
        if (func_scope && strlen(func_scope) > 0) {
            meta->fun_scope = func_scope;
        } else {
            meta->fun_scope = "unknown";
        }
        meta->func_file = file ? file : "unnamed";
        meta->stack_level = STACK_SIZE;
        meta->line = line;

        Children *children = (Children *) malloc(sizeof(Children));
        meta->children = children;
        children->index = 0;
        children->list = NULL;
        children->size = 5;

        Measure *measure = (Measure *) malloc(sizeof(Measure));
        measure->begin = clock();
        meta->measure = measure;

        stack_record.meta = meta;
        push(&stack, stack_record);

        if (STACK_SIZE == 0) {
            array[STACK_INDEX] = meta;
            STACK_INDEX++;
        }
        STACK_SIZE++;

    } else if (STACK_SIZE > 0) {
        STACK_RECORD top_record = pop(&stack);
        STACK_RECORD *new_record = next(&stack);

        Meta *meta = top_record.meta;
        meta->measure->end = clock();
        meta->measure->time_spent = calc_time_spent(meta->measure);

        if (new_record != NULL) {
            Meta *_meta = new_record->meta;
            if (!_meta->children->list) { // already allocated ?
                _meta->children->size *= 2; // more
                _meta->children->list = (Meta **) malloc(_meta->children->size * sizeof(Meta **));
                if (!_meta->children->list) lua_error(L, "out of memory");
            }
            if (_meta->children->index > _meta->children->size - 1) {
                _meta->children->size *= 2; // more
                _meta->children->list = (Meta **) realloc(_meta->children->list,
                                                          _meta->children->size * sizeof(Meta **));
                if (!_meta->children->list) lua_error(L, "out of memory");
            }
            _meta->children->list[_meta->children->index] = meta;
            _meta->children->index++;
        }
        STACK_SIZE--;
    }
}

static void profile_start(lua_State *L) {
    if (PROFILE_INIT)
        return; // already started.
    Meta **meta = (Meta **) malloc(MEM_BLOCKSIZE * sizeof(Meta **));

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

static void profile_show_text(lua_State *L) {
    check_start(L);
    // Spent runtime so far.
    float total_spent = calc_elapsed_time(PROFILE_START_TIME, clock());

    // Lua args
    lua_Object lobj = lua_getparam(L, 1);
    char *breakln = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\n";

    lobj = lua_getparam(L, 2);
    char *offsetc = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\t";

    Meta **array = get_metadata_array(L);

    render_text(L, array, STACK_INDEX - 1, offsetc, breakln);

    printf("TOTAL TIME SPENT: %.3f%s", total_spent, breakln);
}

static void profile_show_html(lua_State *L) {
    Meta **array = get_metadata_array(L);
    render_html(L, array, STACK_INDEX - 1);
}

LUA_API int luaopen_profiler(lua_State *L) {
    lua_register(L, "profile_start", profile_start);
    lua_register(L, "profile_end", profile_end);
    lua_register(L, "profile_show_text", profile_show_text);
    lua_register(L, "profile_show_html", profile_show_html);
    return 0;
}
