#ifndef CJSON_LOGGER_H
#define CJSON_LOGGER_H

#include <string.h>

/**
 * @brief Initialize the cJSON logger.
 *
 */
void cJsonLoggerInit();

/**
 * @brief Delete the cJSON logger and clean up resources.
 *
 */
void cJsonLoggerDelete();

/**
 * @brief Log a message to the cJSON logger.
 *
 * @param jsonPath And array of strings that indicate the names of each JSON node
 * @param size The number of elements in the jsonPath array.
 * @param fmt The format for the log message.
 * @param ... Additional arguments for the format.
 *
 * @note Do not use this function directly, use the JSON_LOGGER macro instead.
 *
 */
void cJsonLoggerLog(char* jsonPath[], int size, const char* fmt, ...);

/**
 * @brief Dump the contents of the root cJSON struct into a file.
 *
 */
void cJsonLoggerDump();

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define JSON_LOGGER(jsonPath, fmt, ...)                                                                                        \
    do {                                                                                                                       \
        cJsonLoggerLog(jsonPath, sizeof(jsonPath) / sizeof(jsonPath[0]), "%s:%d:" fmt, __FILENAME__, __LINE__, ##__VA_ARGS__); \
    } while (0);

#endif // CJSON_LOGGER_H