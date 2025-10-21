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
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -g>
    $<$<CONFIG:Dist>:-O3>
)

target_compile_definitions(cJSONLoggerTests PRIVATE
    $<$<CONFIG:Debug>:CJSONLOGGER_TEST_DEBUG>
    $<$<CONFIG:Release>:CJSONLOGGER_TEST_RELEASE>
    $<$<CONFIG:Dist>:CJSONLOGGER_TEST_DIST>
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
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -g>
    $<$<CONFIG:Dist>:-O3>
)

target_compile_definitions(cJSONLoggerMultiThreadTests PRIVATE
    $<$<CONFIG:Debug>:CJSONLOGGER_TEST_DEBUG>
    $<$<CONFIG:Release>:CJSONLOGGER_TEST_RELEASE>
    $<$<CONFIG:Dist>:CJSONLOGGER_TEST_DIST>
)

target_link_libraries(cJSONLoggerMultiThreadTests cJSONLogger pthread)