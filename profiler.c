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
#include "utils.h"
#include "clocks.h"

int PCONFIG_REF = 0;

/* the stack */
static STACK stack = NULL;
static STACK_RECORD stack_record;

static ProfileConfig *get_profile_config(lua_State *L) {
    return lua_getuserdata(L, lua_getref(L, PCONFIG_REF));
}

/* METADATA */
static Meta **get_metadata_array(lua_State *L, ProfileConfig *pconfig) {
    return lua_getuserdata(L, lua_getref(L, pconfig->meta_info->ref));
}

/* CLEANUP */
static void free_array(Meta **array, int size) {
    int index;
    Meta *meta;
    for (index = 0; index < size; index++) {
        meta = array[index]; // recursive cleaning
        if (meta->children->list)
            free_array(meta->children->list,
                       meta->children->index);
        free(meta->measure);
        free(meta->children);
        free(meta);
    }
    // free the main object
    free(array);
}

static void check_start(lua_State *L, ProfileConfig *pconfig) {
    if (!pconfig || !pconfig->started)
        lua_error(L, "profile not started. call 'profile.start' first!");
}

/* CALL FUNCTION HOOK */
static void callhook(lua_State *L, lua_Function func, char *file, int line) {
    ProfileConfig *pconfig = get_profile_config(L);
    check_start(L, pconfig);

    Meta **array = get_metadata_array(L, pconfig);
    if (!array) return; // check if exists (call profile_stop ?)

    if (pconfig->stack_info->index > pconfig->meta_info->mbuffsize - 1) {
        // Reached memory limit, relocated to double.
        // Updates the size of the memory block.
        pconfig->meta_info->mbuffsize *= 2;

        array = realloc(array, pconfig->meta_info->mbuffsize * sizeof(Meta **));

        if (array) {
            lua_unref(L, pconfig->meta_info->ref); // Remove the old reference (new block of memory).
            lua_pushuserdata(L, array); // Saves the new reference.
            pconfig->meta_info->ref = lua_ref(L, 1);
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
        meta->stack_level = pconfig->stack_info->size;
        meta->line = line;

        Children *children = (Children *) malloc(sizeof(Children));
        meta->children = children;
        children->index = 0;
        children->list = NULL;
        children->size = 20;

        Measure *measure = (Measure *) malloc(sizeof(Measure));

        measure->begin = clock();
        lprofC_start_timer(&measure->begin);
        meta->measure = measure;

        stack_record.meta = meta;
        push(&stack, stack_record);

        if (pconfig->stack_info->size == 0) {
            array[pconfig->stack_info->index] = meta;
            pconfig->stack_info->index++;
        }
        pconfig->stack_info->size++;

    } else if (pconfig->stack_info->size > 0) {
        STACK_RECORD top_record = pop(&stack);
        STACK_RECORD *next_record = next(&stack);

        Meta *meta = top_record.meta;
        meta->measure->time_spent = lprofC_get_seconds(meta->measure->begin);

        if (next_record != NULL) {
            Meta *_meta = next_record->meta;
            if (!_meta->children->list) { // already allocated ?
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
        pconfig->stack_info->size--;
    }
}

static void profile_configure(lua_State *L, ProfileConfig *pconfig) {
    /* EXTRA CONFIGS */
    int nargs = lua_gettop(L);
    lua_Object lobj = lua_getparam(L, nargs);
    if (nargs > 1)
        luaL_arg_check(L, lua_istable(L, lobj), nargs, "enter a configurations table");
    if (lobj > 0 && lua_istable(L, lobj)) {
        /* RECORD LIMIT CONFIG */
        lua_pushobject(L, lobj);
        lua_pushstring(L, "record_limit");
        lua_Object lnumber = lua_gettable(L);
        pconfig->record_limit = lua_isnumber(L, lnumber) ? ((float) lua_getnumber(L, lnumber)) : 0.001f;

        /* STDOUT FILENAME CONFIG */
        lua_pushobject(L, lobj);
        lua_pushstring(L, "stdout_filename");
        lua_Object lfilename = lua_gettable(L);
        pconfig->stdout_filename = lua_isstring(L, lfilename) ? lua_getstring(L, lfilename) : NULL;
    } else {
        pconfig->record_limit = 0.001f;
        pconfig->stdout_filename = NULL;
    }
}

static void profile_reconfigure(lua_State *L) {
    ProfileConfig *pconfig = get_profile_config(L);
    check_start(L, pconfig);
    profile_configure(L, pconfig);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
static void profile_start(lua_State *L) {
    if (PCONFIG_REF != 0)
        return; // already started.

    ProfileConfig *pconfig = (ProfileConfig *) malloc(sizeof(ProfileConfig));
    if (!pconfig) lua_error(L, "out of memory");

    /* RESOURCE DIR CONFIG */
    lua_Object lobj = lua_getparam(L, 1);
    luaL_arg_check(L, lua_isstring(L, lobj), 1,
                   "enter the path to the base directory.");
    pconfig->resource_dir = lua_getstring(L, lobj);
    if (!is_dir(pconfig->resource_dir)) {
        lua_error(L, "the base directory can not be found.");
    }
    /* EXTRA CONFIGS */
    profile_configure(L, pconfig);

    /* METADATA CONFIG */
    pconfig->meta_info = (MetaInfo *) malloc(sizeof(MetaInfo));
    if (!pconfig->meta_info) lua_error(L, "out of memory");

    pconfig->meta_info->mbuffsize = 10;
    Meta **meta = (Meta **) malloc(pconfig->meta_info->mbuffsize * sizeof(Meta **));

    lua_pushuserdata(L, meta);
    pconfig->meta_info->ref = lua_ref(L, 1);

    /* STACK INFO CONFIG */
    pconfig->stack_info = malloc(sizeof(StackInfo));
    if (!pconfig->stack_info) lua_error(L, "out of memory");
    pconfig->stack_info->index = 0;
    pconfig->stack_info->size = 0;

    pconfig->started = true; // profile started

    lua_pushuserdata(L, pconfig);
    PCONFIG_REF = lua_ref(L, 1);

    lua_setcallhook(L, callhook);
}

static void profile_stop(lua_State *L) {
    ProfileConfig *pconfig = get_profile_config(L);
    check_start(L, pconfig);

    lua_setcallhook(L, NULL); // disable hook

    free_array(get_metadata_array(L, pconfig), pconfig->stack_info->index);
    lua_unref(L, pconfig->meta_info->ref);

    free(pconfig->meta_info);
    free(pconfig->stack_info);

    lua_unref(L, PCONFIG_REF);

    free(pconfig);
}

static bool stdout_configure(ProfileConfig *pconfig) {
    if (pconfig->stdout_filename) {
        freopen(pconfig->stdout_filename, "w", stdout);
        return true;
    }
    return false;
}

static void profile_show_text(lua_State *L) {
    ProfileConfig *pconfig = get_profile_config(L);
    check_start(L, pconfig);
    stdout_configure(pconfig);

    Meta **array = get_metadata_array(L, pconfig);

    render_text(L, pconfig, array, pconfig->stack_info->index - 1);
}

static void profile_show_html(lua_State *L) {
    ProfileConfig *pconfig = get_profile_config(L);
    stdout_configure(pconfig);

    Meta **array = get_metadata_array(L, pconfig);

    render_html(L, pconfig, array, pconfig->stack_info->index - 1);
}

static char *profile_as_json(lua_State *L, ProfileConfig *pconfig) {
    Meta **array = get_metadata_array(L, pconfig);
    return render_json(L, pconfig, array, pconfig->stack_info->index - 1);
}

static void lprofile_as_json(lua_State *L) {
    ProfileConfig *pconfig = get_profile_config(L);
    char *jsonvl = profile_as_json(L, pconfig);
    lua_pushstring(L, jsonvl);
    free(jsonvl);
}

static void profile_show_json(lua_State *L) {
    ProfileConfig *pconfig = get_profile_config(L);
    char *jsonvl = profile_as_json(L, pconfig);
    stdout_configure(pconfig);
    printf("%s", jsonvl);
    free(jsonvl);
}

/* Defines the functions of the profiler api */
static struct luaL_reg _methods[] = {
    {"start",       profile_start},
    {"stop",        profile_stop},
    {"show_text",   profile_show_text},
    {"show_html",   profile_show_html},
    {"show_json",   profile_show_json},
    {"reconfigure", profile_reconfigure},
    {"get_json",    lprofile_as_json},
    {NULL, NULL}
};

/* set function */
#define set_table_fn(L, ltable, name, fn)\
    lua_pushobject(L, ltable);\
    lua_pushstring(L, name);\
    lua_pushcfunction(L, fn);\
    lua_settable(L);

/* Initialization function of api */
LUA_API int luaopen_profiler(lua_State *L) {
    lua_Object ltable = lua_createtable(L);

    lua_pushobject(L, ltable);
    lua_setglobal(L, "profile");

    int index = 0;
    while (_methods[index].name) {
        set_table_fn(L, ltable, _methods[index].name, _methods[index].func);
        index++;
    }
    return 0;
}
