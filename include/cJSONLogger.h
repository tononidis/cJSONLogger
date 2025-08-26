/**
 * @file cJSONLogger.h
 * @brief This file contains the interface for the cJSON logger library.
 *
 * @author Stefanos Tononidis
 * @date 2025-08-26
 */

#ifndef CJSON_LOGGER_H
#define CJSON_LOGGER_H

#include <string.h>

typedef enum CJSON_LOG_LEVEL {
    __CJSON_LOG_LEVEL_START = 0,
    CJSON_LOG_LEVEL_CRITICAL,
    CJSON_LOG_LEVEL_ERROR,
    CJSON_LOG_LEVEL_WARN,
    CJSON_LOG_LEVEL_INFO,
    CJSON_LOG_LEVEL_DEBUG,
    __CJSON_LOG_LEVEL_END
} CJSON_LOG_LEVEL_E;

/**
 * @brief Initialize the cJSON logger and setup the resources.
 *
 * @warning This function must be called before any logging can occur.
 *
 * @param logLevel The starting log level severity threshold.
 * @param filePath Path to file where JSON logs will be stored.
 *
 */
void cJSONLoggerInit(CJSON_LOG_LEVEL_E logLevel, const char* filePath);

/**
 * @brief Delete the cJSON logger and clean up resources.
 *
 * @note This function is registered with atexit() during initialization, so it will be called automatically at program exit.
 *
 */
void cJSONLoggerDestroy();

/**
 * @brief Log a message to the cJSON logger.
 *
 * @param jsonPath An array of strings that indicates the hierarchical path of JSON nodes.
 * @param size The number of elements in the jsonPath array.
 * @param logLevel The log level severity.
 * @param fmt The log message format.
 * @param ... Additional arguments for the format.
 *
 * @warning The "$$" delimiter is reserved and used to separate the file name, line number, and log message in the CJSON_LOG* macros.
 *
 * @note Prefer to use the CJSON_LOG* macros instead.
 *
 */
void cJSONLoggerLog(char* jsonPath[], unsigned int size, CJSON_LOG_LEVEL_E logLevel, const char* fmt, ...);

/**
 * @brief Dump the contents of the cJSONLogger into a file.
 *
 */
void cJSONLoggerDump();

/**
 * @brief Dump the contents of the cJSONLogger into a file and rotate.
 *
 */
void cJSONLoggerRotate();

/**
 * @brief Sets the log level for the cJSON logger.
 *
 * @param logLevel The new log level severity threshold.
 */
void cJSONLoggerSetLogLevel(CJSON_LOG_LEVEL_E logLevel);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define CJSON_LOG(jsonPath, logLevel, fmt, ...)                                                                                                              \
    do {                                                                                                                                                     \
        cJSONLoggerLog(jsonPath, sizeof(jsonPath) / sizeof(jsonPath[0]), logLevel, "%s$$%s$$%d$$" fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

#define CJSON_LOG_CRITICAL(jsonPath, fmt, ...) \
    CJSON_LOG(jsonPath, CJSON_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__);

#define CJSON_LOG_CRITICAL(jsonPath, fmt, ...) \
    CJSON_LOG(jsonPath, CJSON_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__);

#define CJSON_LOG_ERROR(jsonPath, fmt, ...) \
    CJSON_LOG(jsonPath, CJSON_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__);

#define CJSON_LOG_WARN(jsonPath, fmt, ...) \
    CJSON_LOG(jsonPath, CJSON_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__);

#define CJSON_LOG_INFO(jsonPath, fmt, ...) \
    CJSON_LOG(jsonPath, CJSON_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__);

#define CJSON_LOG_DEBUG(jsonPath, fmt, ...) \
    CJSON_LOG(jsonPath, CJSON_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__);

#endif // CJSON_LOGGER_H