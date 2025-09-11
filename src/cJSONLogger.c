/**
 * @file cJSONLogger.c
 *
 * @brief This file contains the implementation for the cJSON logger library.
 *
 * @author Stefanos Tononidis
 *
 * @date 2025-08-26
 */

#include "cJSONLogger.h"

#include <cJSON.h>

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @def MAX_TIME_STR_LEN
 *
 * @brief The maximum length of a string representation of time.
 */
#define MAX_TIME_STR_LEN 128

/**
 * @def MAX_LOG_MSG_LEN
 *
 * @brief The maximum length of a log message.
 */
#define MAX_LOG_MSG_LEN 256

/**
 * @def MAX_LOG_COUNT
 *
 * @brief The maximum number of log messages to keep in memory before rotating logs.
 */
#define MAX_LOG_COUNT 500

/**
 * @def MAX_LOG_ROTATION_FILES
 *
 * @brief The maximum number of log rotation files to keep.
 */
#define MAX_LOG_ROTATION_FILES 5

/**
 * @def CJSONLOGGER_DEBUG
 *
 * @brief When building for debugging.
 */
#ifdef CJSONLOGGER_DEBUG

/**
 * @def CJSON_LOGGER_ASSERT_EQ
 *
 * @param expr The expression to evaluate.
 * @param expected The expected value.
 *
 * @brief Does an assertion when the expression is equal to the expected value.
 */
#define CJSON_LOGGER_ASSERT_EQ(expr, expected) \
    do {                                       \
        assert(expr == expected);              \
    } while (0);

/**
 * @def CJSON_LOGGER_ASSERT_NEQ
 *
 * @param expr The expression to evaluate.
 * @param expected The expected value.
 *
 * @brief Does an assertion when the expression is not equal to the expected value.
 */
#define CJSON_LOGGER_ASSERT_NEQ(expr, expected) \
    do {                                        \
        assert(expr != expected);               \
    } while (0);

#endif

/**
 * @def CJSONLOGGER_RELEASE
 *
 * @brief When building for a release.
 */
#ifdef CJSONLOGGER_RELEASE

/**
 * @def CJSON_LOGGER_ASSERT_EQ
 *
 * @param expr The expression to evaluate.
 * @param expected The expected value.
 *
 * @brief Prints an error message if the expression is not equal to the expected value.
 */
#define CJSON_LOGGER_ASSERT_EQ(expr, expected)                                                     \
    do {                                                                                           \
        if (expr != expected) {                                                                    \
            fprintf(stderr, "Assertion at [%s:%s:%d] failed\n", __FILENAME__, __func__, __LINE__); \
        }                                                                                          \
    } while (0);

/**
 * @def CJSON_LOGGER_ASSERT_NEQ
 *
 * @param expr The expression to evaluate.
 * @param expected The expected value.
 *
 * @brief Prints an error message if the expression is equal to the expected value.
 */
#define CJSON_LOGGER_ASSERT_NEQ(expr, expected)                                                    \
    do {                                                                                           \
        if (expr == expected) {                                                                    \
            fprintf(stderr, "Assertion at [%s:%s:%d] failed\n", __FILENAME__, __func__, __LINE__); \
        }                                                                                          \
    } while (0);

#endif

/**
 * @def CJSONLOGGER_DIST
 *
 * @brief When building for a distribution.
 */
#ifdef CJSONLOGGER_DIST

/**
 * @def CJSON_LOGGER_ASSERT_EQ
 *
 * @param expr The expression to evaluate.
 * @param expected The expected value.
 *
 * @brief Does nothing.
 */
#define CJSON_LOGGER_ASSERT_EQ(expr, expected) \
    do {                                       \
        if (expr != expected) {                \
        }                                      \
    } while (0);

/**
 * @def CJSON_LOGGER_ASSERT_NEQ
 *
 * @param expr The expression to evaluate.
 * @param expected The expected value.
 *
 * @brief Does nothing.
 */
#define CJSON_LOGGER_ASSERT_NEQ(expr, expected) \
    do {                                        \
        if (expr == expected) {                 \
        }                                       \
    } while (0);

#endif

/**
 * @struct LogInfo
 *
 * @brief Structure used to store information about a log.
 *
 * @var timeStamp The logs time stamp as a string representation.
 * @var logLevel The logs log level.
 * @var fileName The logs file name.
 * @var funcName The logs function name.
 * @var fileLine The logs file line.
 */
typedef struct LogInfo {
    char timeStamp[MAX_TIME_STR_LEN];
    CJSON_LOG_LEVEL_E logLevel;
    char* fileName;
    char* funcName;
    int fileLine;
} LogInfo_s;

/**
 * @struct Queue
 *
 * @brief Queue structure for managing log rotation files.
 *
 * @var rotatedFiles Array of rotated log file paths.
 * @var head Index of the head element.
 * @var tail Index of the tail element.
 * @var currentSize Current number of elements in the queue.
 */
typedef struct Queue {
    char* rotatedFiles[MAX_LOG_ROTATION_FILES];
    int head;
    int tail;
    int currentSize;
} Queue_s;

/**
 * @brief Queue for managing rotated log files.
 */
static Queue_s* s_g_rotatedFilesQueue = NULL;

/**
 * @brief Root JSON node for the logger.
 */
static cJSON* s_g_rootNode = NULL;

/**
 * @brief File path where logs will be stored.
 */
static char* s_g_filePath = NULL;

/**
 * @brief The current log level.
 */
static CJSON_LOG_LEVEL_E s_g_logLevel = __CJSON_LOG_LEVEL_START;

/**
 * @brief Counter for how many logs exist in the logger.
 */
static unsigned int s_g_logCount = 0;

/**
 * @brief Mutex for accessing the root JSON node.
 */
static pthread_mutex_t s_g_rootNodeMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Mutex for accessing various global variables.
 */
static pthread_mutex_t s_g_cLoggerMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Get the string representation of the log level.
 *
 * @param logLevel The log level to convert.
 *
 * @return The string representation of the log level.
 */
static inline const char* cJSONLoggerGetLogLevelStr(CJSON_LOG_LEVEL_E logLevel)
{
    switch (logLevel) {
    case CJSON_LOG_LEVEL_CRITICAL:
        return "CRITICAL";
    case CJSON_LOG_LEVEL_ERROR:
        return "ERROR";
    case CJSON_LOG_LEVEL_WARN:
        return "WARN";
    case CJSON_LOG_LEVEL_INFO:
        return "INFO";
    case CJSON_LOG_LEVEL_DEBUG:
        return "DEBUG";
    default:
        return "UNKNOWN";
    }
}

/**
 * @brief Push a log message to the specified JSON node.
 *
 * @param node The JSON node to push the log message to.
 * @param logInfo The log info, such as time stamp, file name, etc.
 * @param logMsg The log message to push.
 */
static void cJSONLoggerPushLog(cJSON* node, LogInfo_s* logInfo, const char* logMsg)
{
    CJSON_LOGGER_ASSERT_NEQ(logInfo, NULL);

    pthread_mutex_lock(&s_g_rootNodeMutex);
    CJSON_LOGGER_ASSERT_NEQ(node, NULL);

    if (cJSON_HasObjectItem(node, "logs") == 0) {
        cJSON_AddItemToObject(node, "logs", cJSON_CreateArray());
    }

    cJSON* logs = cJSON_GetObjectItem(node, "logs");
    cJSON* log = cJSON_CreateObject();

    cJSON_AddItemToArray(logs, log);
    cJSON_AddItemToObject(log, "Time", cJSON_CreateString(logInfo->timeStamp));
    cJSON_AddItemToObject(log, "LogLevel", cJSON_CreateString(cJSONLoggerGetLogLevelStr(logInfo->logLevel)));

    if (logInfo->fileName != NULL) {
        cJSON_AddItemToObject(log, "FileName", cJSON_CreateString(logInfo->fileName));
    }

    if (logInfo->funcName != NULL) {
        cJSON_AddItemToObject(log, "FuncName", cJSON_CreateString(logInfo->funcName));
    }

    if (logInfo->fileLine != 0) {
        cJSON_AddItemToObject(log, "FileLine", cJSON_CreateNumber(logInfo->fileLine));
    }

    if (logMsg != NULL) {
        cJSON_AddItemToObject(log, "Log", cJSON_CreateString(logMsg));
    }

    pthread_mutex_unlock(&s_g_rootNodeMutex);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (++s_g_logCount > MAX_LOG_COUNT) {
        pthread_mutex_unlock(&s_g_cLoggerMutex);
        cJSONLoggerRotate();
    }

    else {
        pthread_mutex_unlock(&s_g_cLoggerMutex);
    }
}

/**
 * @brief Create a Json Object object
 *
 * @param node The node where the new child node will be inserted.
 * @param nodeName The name of the new child node.
 *
 * @return cJSON* ptr of the next child node.
 */
static cJSON* createJsonObject(cJSON* node, const char* nodeName)
{
    cJSON* nextNode = NULL;
    if (cJSON_HasObjectItem(node, nodeName) != 0) {
        nextNode = cJSON_GetObjectItem(node, nodeName);
        CJSON_LOGGER_ASSERT_NEQ(nextNode, NULL);
    }

    else {
        nextNode = cJSON_CreateObject();
        CJSON_LOGGER_ASSERT_NEQ(nextNode, NULL);
        cJSON_AddItemToObject(node, nodeName, nextNode);
    }

    return nextNode;
}

void cJSONLoggerInit(CJSON_LOG_LEVEL_E logLevel, const char* filePath)
{
    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (s_g_rootNode == NULL) {
        s_g_rootNode = cJSON_CreateObject();
        CJSON_LOGGER_ASSERT_NEQ(s_g_rootNode, NULL);
    }
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    cJSONLoggerSetLogLevel(logLevel);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (s_g_filePath != NULL) {
        free(s_g_filePath);
    }

    s_g_filePath = strdup(filePath);
    CJSON_LOGGER_ASSERT_NEQ(s_g_filePath, NULL);

    pthread_mutex_unlock(&s_g_cLoggerMutex);

    int ret = atexit(cJSONLoggerDestroy);
    CJSON_LOGGER_ASSERT_EQ(ret, 0);
}

void cJSONLoggerDestroy()
{
    cJSONLoggerDump();

    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (s_g_rootNode != NULL) {
        cJSON_Delete(s_g_rootNode);
    }
    s_g_rootNode = NULL;
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (s_g_filePath != NULL) {
        free(s_g_filePath);
    }
    s_g_filePath = NULL;

    if (s_g_rotatedFilesQueue != NULL) {
        for (int i = 0; i < s_g_rotatedFilesQueue->currentSize; i++) {
            if (s_g_rotatedFilesQueue->rotatedFiles[i] != NULL) {
                free(s_g_rotatedFilesQueue->rotatedFiles[i]);
                s_g_rotatedFilesQueue->rotatedFiles[i] = NULL;
            }
        }
        free(s_g_rotatedFilesQueue);
    }

    s_g_rotatedFilesQueue = NULL;
    s_g_logCount = 0;
    s_g_logLevel = __CJSON_LOG_LEVEL_START;
    pthread_mutex_unlock(&s_g_cLoggerMutex);
}

void cJSONLoggerLog(CJSON_LOG_LEVEL_E logLevel, const char* fmt, ...)
{
    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (logLevel > __CJSON_LOG_LEVEL_START && logLevel > s_g_logLevel && logLevel < __CJSON_LOG_LEVEL_END) {
        pthread_mutex_unlock(&s_g_cLoggerMutex);
        return;
    }
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    if (strlen(fmt) > MAX_LOG_MSG_LEN - 1) {
        return;
    }

    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (s_g_rootNode == NULL) {
        pthread_mutex_unlock(&s_g_rootNodeMutex);
        return;
    }
    cJSON* node = s_g_rootNode;
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    va_list args;
    va_start(args, fmt);

    LogInfo_s logInfo = { 0 };
    logInfo.logLevel = logLevel;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tmInfo;
    localtime_r(&ts.tv_sec, &tmInfo);

    snprintf(logInfo.timeStamp, sizeof(logInfo.timeStamp), "%d-%d-%d %d:%d:%d.%ld",
        tmInfo.tm_year + 1900,
        tmInfo.tm_mon + 1,
        tmInfo.tm_mday,
        tmInfo.tm_hour,
        tmInfo.tm_min,
        tmInfo.tm_sec,
        ts.tv_nsec);

    if (strncmp(fmt, "$$%s$$%s$$%d$$", strlen("$$%s$$%s$$%d$$")) == 0) {
        logInfo.fileName = va_arg(args, char*);
        logInfo.funcName = va_arg(args, char*);
        logInfo.fileLine = va_arg(args, int);
        fmt += strlen("$$%s$$%s$$%d$$");
    }

    char logMsgFmt[MAX_LOG_MSG_LEN] = { 0 };
    char* pLogMsgFmt = logMsgFmt;

    while (*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {

                // case '\n'

            case JNO_CHAR: {
                *pLogMsgFmt = '\0';
                if (strlen(logMsgFmt) != 0) {
                    char logMsg[MAX_LOG_MSG_LEN] = { 0 };
                    vsnprintf(logMsg, sizeof(logMsg) - 1, logMsgFmt, args);
                    cJSONLoggerPushLog(node, &logInfo, logMsg);

                    memset(logMsgFmt, 0, sizeof(logMsgFmt));
                    pLogMsgFmt = logMsgFmt;
                }

                char* c = va_arg(args, char*);
                node = createJsonObject(node, c);
                break;
            }

            default: {
                *pLogMsgFmt++ = '%';
                *pLogMsgFmt++ = *fmt++;
                continue;
            }
            }
        }

        else {
            *pLogMsgFmt++ = *fmt;
        }

        fmt++;
    }

    *pLogMsgFmt = '\0';
    if (strlen(logMsgFmt) > 0) {
        char logMsg[MAX_LOG_MSG_LEN] = { 0 };
        vsnprintf(logMsg, sizeof(logMsg) - 1, logMsgFmt, args);
        cJSONLoggerPushLog(node, &logInfo, logMsg);
    }

    va_end(args);
}

void cJSONLoggerDump()
{
    pthread_mutex_lock(&s_g_rootNodeMutex);
    char* string = cJSON_Print(s_g_rootNode);
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    if (string == NULL) {
        return;
    }

    pthread_mutex_lock(&s_g_cLoggerMutex);
    FILE* file = fopen(s_g_filePath, "w");
    CJSON_LOGGER_ASSERT_NEQ(file, NULL);
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    fprintf(file, "%s", string);

    fclose(file);
    free(string);
}

void cJSONLoggerRotate()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tmInfo;
    localtime_r(&ts.tv_sec, &tmInfo);

    char timeStr[MAX_TIME_STR_LEN] = { 0 };
    snprintf(timeStr, sizeof(timeStr), "%d_%d_%d_%ld",
        tmInfo.tm_hour,
        tmInfo.tm_min,
        tmInfo.tm_sec,
        ts.tv_nsec);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    s_g_logCount = 0;

    if (s_g_rotatedFilesQueue == NULL) {
        s_g_rotatedFilesQueue = (Queue_s*)calloc(1, sizeof(Queue_s));
        CJSON_LOGGER_ASSERT_NEQ(s_g_rotatedFilesQueue, NULL);
    }

    size_t rotatedFileLen = strlen(s_g_filePath) + strlen(timeStr) + 2;
    char* rotatedFilePath = (char*)malloc(rotatedFileLen);
    CJSON_LOGGER_ASSERT_NEQ(rotatedFilePath, NULL);

    snprintf(rotatedFilePath, rotatedFileLen, "%s_%s", timeStr, s_g_filePath);

    if (s_g_rotatedFilesQueue->currentSize < MAX_LOG_ROTATION_FILES) {
        s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->tail] = strdup(rotatedFilePath);

        CJSON_LOGGER_ASSERT_NEQ(s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->tail], NULL);

        s_g_rotatedFilesQueue->tail = (s_g_rotatedFilesQueue->tail + 1) % MAX_LOG_ROTATION_FILES;
        s_g_rotatedFilesQueue->currentSize++;
    }

    else {
        int res = remove(s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->head]);

        CJSON_LOGGER_ASSERT_EQ(res, 0);

        free(s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->head]);
        s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->head] = NULL;
        s_g_rotatedFilesQueue->head = (s_g_rotatedFilesQueue->head + 1) % MAX_LOG_ROTATION_FILES;
        s_g_rotatedFilesQueue->currentSize--;
    }

    FILE* file = fopen(rotatedFilePath, "w");
    CJSON_LOGGER_ASSERT_NEQ(file, NULL);

    free(rotatedFilePath);
    rotatedFilePath = NULL;
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    pthread_mutex_lock(&s_g_rootNodeMutex);
    char* string = NULL;
    if (s_g_rootNode != NULL) {
        string = cJSON_Print(s_g_rootNode);
        cJSON_Delete(s_g_rootNode);
        s_g_rootNode = cJSON_CreateObject();
    }
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    if (string == NULL) {
        return;
    }

    fprintf(file, "%s", string);

    fclose(file);
    free(string);
    string = NULL;
}

void cJSONLoggerSetLogLevel(CJSON_LOG_LEVEL_E logLevel)
{
    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (logLevel > __CJSON_LOG_LEVEL_START && logLevel < __CJSON_LOG_LEVEL_END) {
        s_g_logLevel = logLevel;
    }
    pthread_mutex_unlock(&s_g_cLoggerMutex);
}