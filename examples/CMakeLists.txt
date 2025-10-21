file(GLOB EXAMPLES "*.c")

add_executable(cJSONLoggerExample ${EXAMPLES})

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

target_link_libraries(cJSONLoggerExample cJSONLogger pthread)