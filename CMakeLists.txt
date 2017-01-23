cmake_minimum_required(VERSION 3.3)
project(lua_profiler)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories(${LUA_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_EXTRA})

include_directories(json-builder)
include_directories(json-parser)

set(SOURCE_FILES
        profiler.c
        profiler.h
        measure.h
        measure.c
        stack.h
        stack.c
        render.c
        render.h
        utils.c
        utils.h
        clocks.c
        clocks.h
        json-builder/json-builder.c
        json-parser/json.c)

add_library(lua_profiler MODULE ${SOURCE_FILES})

# =====================
set_target_properties(lua_profiler PROPERTIES
        COMPILE_FLAGS "-m32"
        LINK_FLAGS    "-m32")
# =====================

target_link_libraries(lua_profiler ${LUA_LIBRARY})