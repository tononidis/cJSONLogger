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
)

add_subdirectory(examples)

add_subdirectory(tests)