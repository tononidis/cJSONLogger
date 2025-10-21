set(CMAKE_CONFIGURATION_TYPES "debug;release;dist" CACHE STRING "" FORCE)

file(GLOB TESTS "test.c")

add_executable(cJSONLoggerTests ${TESTS})

target_compile_options(cJSONLoggerTests PRIVATE
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

target_link_libraries(cJSONLoggerTests cJSONLogger pthread)

file(GLOB MULTITHREAD_TESTS "multithread_tests.c")

add_executable(cJSONLoggerMultiThreadTests ${MULTITHREAD_TESTS})

target_compile_options(cJSONLoggerMultiThreadTests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wuninitialized
    -Wunreachable-code
    -Wconversion
    -Wsign-conversion
    -Wno-unused-parameter
    -fstack-protector-strong
)

target_link_libraries(cJSONLoggerMultiThreadTests cJSONLogger pthread)