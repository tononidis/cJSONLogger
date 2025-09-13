/**
 * @file example.c
 *
 * @brief This file contains an example usage of the cJSON logger library.
 *
 * @author Stefanos Tononidis
 *
 * @date 2025-08-26
 */

#include <cJSONLogger.h>
#include <stdio.h>

/**
 * @brief Main entry point for the example application.
 *
 * @return int always 0.
 */
int main(void)
{
    if (cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, "log.json") != 0) {
        return -1;
    }

    CJSON_LOG_DEBUG("%" JNO "value %d", "foo", 1); // Will not log since the log level starts from INFO.
    CJSON_LOG_INFO("%" JNO "%" JNO "value %d", "foo", "bar", 2); // Log with two node levels.
    CJSON_LOG_WARN("%" JNO "%" JNO "value %d", "foo", "bar2", 3); // Log with two node levels.
    CJSON_LOG_ERROR("%" JNO "%" JNO "%" JNO "value %d", "foo", "bar", "baz", 4); // Log with three node levels.
    CJSON_LOG_CRITICAL("%" JNO "value %d", "qix", 5); // Log with one node level.

    CJSON_LOG_INFO("value %d", 6); // Log the the root node.

    // The cJSONLogger lib registers an atexit function that will handle this automatically
    // cJSONLoggerDump();
    // cJSONLoggerDestroy();

    return 0;
}