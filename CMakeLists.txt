cmake_minimum_required(VERSION 3.3)
project(lua_profiler)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# Lua
include_directories(${LUA_INCLUDE_DIR})

if (WIN32)
    link_directories(${LUA_LIBRARIES})
endif()

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

if (WIN32)
    set_target_properties(lua_profiler PROPERTIES PREFIX "")
    target_link_libraries(lua_profiler lualib32ng lua32ng)
else()
    set_target_properties(lua_profiler PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
