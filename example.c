#include <cJSONLogger.h>

int main(void)
{
    cJsonLoggerInit();

    char* level11[] = { "foo" };

    char* level21[] = { "foo", "bar" };
    char* level22[] = { "foo", "bar2" };

    char* level3[] = { "foo", "bar", "baz" };

    char* level12[] = { "other" };

    JSON_LOGGER(level11, "value %d", 1);
    JSON_LOGGER(level21, "value %d", 2);
    JSON_LOGGER(level22, "value %d", 3);
    JSON_LOGGER(level3, "value %d", 4);
    JSON_LOGGER(level12, "value %d", 5);

    cJsonLoggerDump();
    cJsonLoggerDelete();

    return 0;
}