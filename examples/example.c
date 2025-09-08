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
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, "log.json");

    CJSON_LOG_WARN("%" JO "%%", "node");
    // CJSON_LOG_WARN("%" JO "msg  s%%", "node");
    // CJSON_LOG_WARN("%" JO "msg dsa", "node");
    // cJSONLoggerLog(CJSON_LOG_LEVEL_INFO, "fooc1 %s %" JO "1sbar %" JO "stef %s sa", "1var", "test", "1node", "2node");

    // cJSONLoggerDestroy();

    return 0;
}