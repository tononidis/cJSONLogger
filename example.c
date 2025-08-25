/**
 * @author Stefanos Tononidis
 */

#include <cJSONLogger.h>

int main(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, "log.json");

    char* level11[] = { "foo" };

    char* level21[] = { "foo", "bar" };
    char* level22[] = { "foo", "bar2" };

    char* level3[] = { "foo", "bar", "baz" };

    char* level12[] = { "other" };

    CJSON_LOG_DEBUG(level11, "value %d", 1);
    CJSON_LOG_INFO(level21, "value %d", 2);
    CJSON_LOG_WARN(level22, "value %d", 3);
    CJSON_LOG_ERROR(level3, "value %d", 4);
    CJSON_LOG_CRITICAL(level12, "value %d", 5);

    // The cJSONLogger lib registers an atexit function that will handle this automatically
    // cJSONLoggerDump();
    // cJSONLoggerDestroy();

    return 0;
}