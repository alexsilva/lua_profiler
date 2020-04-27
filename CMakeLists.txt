cmake_minimum_required(VERSION 3.3)
project(lua_profiler)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# Lua
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARIES})

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

add_library(lprofiler MODULE ${SOURCE_FILES})

if (WIN32)
    set_target_properties(lprofiler PROPERTIES PREFIX "")
    target_link_libraries(lprofiler lualib32ngw lua32ngw)
else()
    target_link_libraries(lprofiler lua)
endif()
