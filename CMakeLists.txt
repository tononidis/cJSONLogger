cmake_minimum_required(VERSION 3.10)

project(cJSONLogger C)

include_directories(include cJSON)

file(GLOB SRC "src/*.c" "cJSON/cJSON.c")

add_library(cJSONLogger STATIC ${SRC})

target_compile_options(cJSONLogger PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wuninitialized
    -Wunreachable-code
    -Wconversion
    -Wsign-conversion
    -fstack-protector-strong
    $<$<CONFIG:debug>:-O0 -g>
    $<$<CONFIG:release>:-O3 -g>
    $<$<CONFIG:dist>:-O3 -DNDEBUG>
)

target_compile_definitions(cJSONLogger PRIVATE
    $<$<CONFIG:debug>:CJSONLOGGER_DEBUG>
    $<$<CONFIG:release>:CJSONLOGGER_RELEASE>
    $<$<CONFIG:dist>:CJSONLOGGER_DIST>
)

add_subdirectory(examples)

add_subdirectory(tests)