cmake_minimum_required(VERSION 3.3)
project(lua_profiler)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories(${LUA_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_EXTRA})

set(SOURCE_FILES
        profiler.c
        profiler.h
        measure.h
        measure.c
        stack.h
        stack.c)
add_library(lua_profiler SHARED ${SOURCE_FILES})

# =====================
set_target_properties(lua_profiler PROPERTIES
        COMPILE_FLAGS "-m32"
        LINK_FLAGS    "-m32")
# =====================

target_link_libraries(lua_profiler ${LUA_LIBRARY})