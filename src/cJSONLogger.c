#include "cJSONLogger.h"

#include <cJSON.h>

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TIME_STR_LEN 128
#define MAX_LOG_MSG_LEN 256
#define MAX_LOG_COUNT 500
#define MAX_LOG_ROTATION_FILES 5

#ifdef CJSONLOGGER_DEBUG
#define DEBUG_ASSERT(expr) assert(expr)
#endif

#ifdef CJSONLOGGER_RELEASE
#define DEBUG_ASSERT(expr) ((void)0)
#endif

#ifdef CJSONLOGGER_DIST
#define DEBUG_ASSERT(expr) ((void)0)
#endif

static cJSON* s_g_rootNode = NULL;
static pthread_mutex_t s_g_rootNodeMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t s_g_cLoggerMutex = PTHREAD_MUTEX_INITIALIZER;

static CJSON_LOG_LEVEL_E s_g_logLevel = __CJSON_LOG_LEVEL_START;

static char* s_g_filePath = NULL;

static unsigned int s_g_logCount = 0;

typedef struct Queue {
    char* rotatedFiles[MAX_LOG_ROTATION_FILES];
    int head;
    int tail;
    int currentSize;
} Queue_s;

static Queue_s* s_g_rotatedFilesQueue = NULL;

static inline char* cJSONLoggerGetLogLevelStr(CJSON_LOG_LEVEL_E logLevel)
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

void cJSONLoggerInit(CJSON_LOG_LEVEL_E logLevel, const char* filePath)
{
    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (s_g_rootNode == NULL) {
        s_g_rootNode = cJSON_CreateObject();
    }

    DEBUG_ASSERT(s_g_rootNode != NULL && "Failed to create JSON root");

    pthread_mutex_unlock(&s_g_rootNodeMutex);

    cJSONLoggerSetLogLevel(logLevel);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    s_g_filePath = strdup(filePath);

    DEBUG_ASSERT(s_g_filePath != NULL && "Failed to duplicate log file path");

    pthread_mutex_unlock(&s_g_cLoggerMutex);

    int ret = atexit(cJSONLoggerDestroy);

    DEBUG_ASSERT(ret == 0 && "Failed to register exit handler");
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
        free(s_g_rotatedFilesQueue);
    }
    s_g_rotatedFilesQueue = NULL;
    pthread_mutex_unlock(&s_g_cLoggerMutex);
}

static void cJSONLoggerRotateLogs()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm* tmInfo = localtime(&ts.tv_sec);

    char timeStr[MAX_TIME_STR_LEN];
    snprintf(timeStr, sizeof(timeStr), "%d_%d_%d_%ld",
        tmInfo->tm_hour,
        tmInfo->tm_min,
        tmInfo->tm_sec,
        ts.tv_nsec);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    s_g_logCount = 0;

    if (s_g_rotatedFilesQueue == NULL) {
        s_g_rotatedFilesQueue = (Queue_s*)malloc(sizeof(Queue_s));

        DEBUG_ASSERT(s_g_rotatedFilesQueue != NULL && "Failed to allocate memory for rotated files queue");

        memset(s_g_rotatedFilesQueue, 0, sizeof(*s_g_rotatedFilesQueue));
    }

    char* filePath = s_g_filePath;
    unsigned int rotatedFileLen = strlen(filePath) + strlen(timeStr) + 2;

    s_g_filePath = (char*)malloc(rotatedFileLen);

    DEBUG_ASSERT(s_g_filePath != NULL && "Failed to allocate memory for log file path");

    snprintf(s_g_filePath, rotatedFileLen, "%s_%s", timeStr, filePath);

    if (s_g_rotatedFilesQueue->currentSize < MAX_LOG_ROTATION_FILES) {
        s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->tail] = strdup(s_g_filePath);
        s_g_rotatedFilesQueue->tail = (s_g_rotatedFilesQueue->tail + 1) % MAX_LOG_ROTATION_FILES;
        s_g_rotatedFilesQueue->currentSize++;
    }

    else {
        remove(s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->head]);
        free(s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->head]);
        s_g_rotatedFilesQueue->rotatedFiles[s_g_rotatedFilesQueue->head] = NULL;
        s_g_rotatedFilesQueue->head = (s_g_rotatedFilesQueue->head + 1) % MAX_LOG_ROTATION_FILES;
        s_g_rotatedFilesQueue->currentSize--;
    }
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    cJSONLoggerDump();

    pthread_mutex_lock(&s_g_cLoggerMutex);
    free(s_g_filePath);
    s_g_filePath = filePath;
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (s_g_rootNode != NULL) {
        cJSON_Delete(s_g_rootNode);
    }

    s_g_rootNode = cJSON_CreateObject();
    pthread_mutex_unlock(&s_g_rootNodeMutex);
}

static void cJSONLoggerPushLog(cJSON* node, CJSON_LOG_LEVEL_E logLevel, const char* logMsg)
{

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm* tmInfo = localtime(&ts.tv_sec);

    char timeStr[MAX_TIME_STR_LEN];
    snprintf(timeStr, sizeof(timeStr), "%d-%d-%d %d:%d:%d.%ld",
        tmInfo->tm_year + 1900,
        tmInfo->tm_mon + 1,
        tmInfo->tm_mday,
        tmInfo->tm_hour,
        tmInfo->tm_min,
        tmInfo->tm_sec,
        ts.tv_nsec);

    char* logMsgDup = strdup(logMsg);
    char* fileName = strtok(logMsgDup, "$$");
    char* fileLine = strtok(NULL, "$$");
    char* userLog = strtok(NULL, "$$");

    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (cJSON_HasObjectItem(node, "logs") == 0) {
        cJSON_AddItemToObject(node, "logs", cJSON_CreateArray());
    }

    cJSON* logs = cJSON_GetObjectItem(node, "logs");
    cJSON* log = cJSON_CreateObject();

    cJSON_AddItemToArray(logs, log);
    cJSON_AddItemToObject(log, "Time", cJSON_CreateString(timeStr));
    cJSON_AddItemToObject(log, "LogLevel", cJSON_CreateString(cJSONLoggerGetLogLevelStr(logLevel)));

    if (fileLine != NULL) {
        cJSON_AddItemToObject(log, "FileName", cJSON_CreateString(fileName));
    }

    if (fileLine != NULL) {
        cJSON_AddItemToObject(log, "FileLine", cJSON_CreateString(fileLine));
    }

    if (userLog != NULL) {
        cJSON_AddItemToObject(log, "Log", cJSON_CreateString(userLog));
    }

    else {
        cJSON_AddItemToObject(log, "Log", cJSON_CreateString(logMsg));
    }

    pthread_mutex_unlock(&s_g_rootNodeMutex);

    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (++s_g_logCount > MAX_LOG_COUNT) {
        pthread_mutex_unlock(&s_g_cLoggerMutex);
        cJSONLoggerRotateLogs();
    }

    else {
        pthread_mutex_unlock(&s_g_cLoggerMutex);
    }

    free(logMsgDup);
}

static void cJSONLoggerLogRecur(char* jsonPath[], unsigned int size, cJSON* node, CJSON_LOG_LEVEL_E logLevel, const char* logMsg)
{
    const char* nodeName = jsonPath[0];

    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (cJSON_HasObjectItem(node, nodeName) == 0) {
        cJSON_AddItemToObject(node, nodeName, cJSON_CreateObject());
    }

    cJSON* subNode = cJSON_GetObjectItem(node, nodeName);
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    if (size == 1) {
        cJSONLoggerPushLog(subNode, logLevel, logMsg);
    }

    else {
        cJSONLoggerLogRecur(&jsonPath[1], size - 1, subNode, logLevel, logMsg);
    }
}

void cJSONLoggerLog(char* jsonPath[], unsigned int size, CJSON_LOG_LEVEL_E logLevel, const char* fmt, ...)
{
    if (size <= 0) {
        return;
    }

    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (logLevel > __CJSON_LOG_LEVEL_START && logLevel > s_g_logLevel && logLevel < __CJSON_LOG_LEVEL_END) {
        pthread_mutex_unlock(&s_g_cLoggerMutex);
        return;
    }
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    DEBUG_ASSERT(s_g_rootNode != NULL && "cJSONlogger was not initialized");

    const char* nodeName = jsonPath[0];

    pthread_mutex_lock(&s_g_rootNodeMutex);
    if (cJSON_HasObjectItem(s_g_rootNode, nodeName) == 0) {
        cJSON_AddItemToObject(s_g_rootNode, nodeName, cJSON_CreateObject());
    }

    cJSON* subNode = cJSON_GetObjectItem(s_g_rootNode, nodeName);
    pthread_mutex_unlock(&s_g_rootNodeMutex);

    va_list args;
    va_start(args, fmt);
    char logMsg[MAX_LOG_MSG_LEN];
    vsnprintf(logMsg, sizeof(logMsg), fmt, args);
    va_end(args);

    if (size == 1) {
        cJSONLoggerPushLog(subNode, logLevel, logMsg);
    }

    else {
        cJSONLoggerLogRecur(&jsonPath[1], size - 1, subNode, logLevel, logMsg);
    }
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
    pthread_mutex_unlock(&s_g_cLoggerMutex);

    DEBUG_ASSERT(file != NULL);

    fprintf(file, "%s", string);

    fclose(file);
    free(string);
}

void cJSONLoggerSetLogLevel(CJSON_LOG_LEVEL_E logLevel)
{
    pthread_mutex_lock(&s_g_cLoggerMutex);
    if (logLevel > __CJSON_LOG_LEVEL_START && logLevel < __CJSON_LOG_LEVEL_END) {
        s_g_logLevel = logLevel;
    }
    pthread_mutex_unlock(&s_g_cLoggerMutex);
}