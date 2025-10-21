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
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -g>
    $<$<CONFIG:Dist>:-O3 -DNDEBUG>
)

target_compile_definitions(cJSONLogger PRIVATE
    $<$<CONFIG:Debug>:CJSONLOGGER_DEBUG>
    $<$<CONFIG:Release>:CJSONLOGGER_RELEASE>
    $<$<CONFIG:Dist>:CJSONLOGGER_DIST>
)

add_subdirectory(examples)

add_subdirectory(tests)