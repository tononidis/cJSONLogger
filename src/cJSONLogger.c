#include "cJSONLogger.h"
#include "cJSON.h"

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TIME_STR_LEN 128
#define MAX_LOG_MSG_LEN 256

static cJSON* s_g_rootNode = NULL;
static pthread_mutex_t s_g_jsonLoggerMutex = PTHREAD_MUTEX_INITIALIZER;

void cJsonLoggerInit()
{
    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    s_g_rootNode = cJSON_CreateObject();
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);

    assert(s_g_rootNode != NULL && "Failed to create JSON root");
}

static void cJsonLoggerPushLog(cJSON* node, const char* logMsg)
{
    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    if (cJSON_HasObjectItem(node, "logs") == 0) {
        cJSON_AddItemToObject(node, "logs", cJSON_CreateArray());
    }

    cJSON* logs = cJSON_GetObjectItem(node, "logs");
    cJSON* log = cJSON_CreateObject();

    cJSON_AddItemToArray(logs, log);
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);

    char* logMsgDup = strdup(logMsg);
    char* fileName = strtok(logMsgDup, ":");
    char* fileLine = strtok(NULL, ":");
    char* userLog = strtok(NULL, ":");

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm* tm_info = localtime(&ts.tv_sec);

    char timeStr[MAX_TIME_STR_LEN];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d.%09ld",
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday,
        tm_info->tm_hour,
        tm_info->tm_min,
        tm_info->tm_sec,
        ts.tv_nsec);

    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    cJSON_AddItemToObject(log, "TimeStamp", cJSON_CreateString(timeStr));
    cJSON_AddItemToObject(log, "FileName", cJSON_CreateString(fileName));
    cJSON_AddItemToObject(log, "FileLine", cJSON_CreateString(fileLine));
    cJSON_AddItemToObject(log, "Log", cJSON_CreateString(userLog));
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);

    free(logMsgDup);
}

static void cJsonLoggerLogRecur(char* jsonPath[], int size, cJSON* node, const char* logMsg)
{
    const char* nodeName = jsonPath[0];

    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    if (cJSON_HasObjectItem(node, nodeName) == 0) {
        cJSON_AddItemToObject(node, nodeName, cJSON_CreateObject());
    }

    cJSON* subNode = cJSON_GetObjectItem(node, nodeName);
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);

    if (size == 1) {
        cJsonLoggerPushLog(subNode, logMsg);
    }

    else {
        cJsonLoggerLogRecur(&jsonPath[1], size - 1, subNode, logMsg);
    }
}

void cJsonLoggerLog(char* jsonPath[], int size, const char* fmt, ...)
{
    if (size <= 0) {
        return;
    }

    assert(s_g_rootNode != NULL && "cJSONlogger was not initialized");
    const char* nodeName = jsonPath[0];

    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    if (cJSON_HasObjectItem(s_g_rootNode, nodeName) == 0) {
        cJSON_AddItemToObject(s_g_rootNode, nodeName, cJSON_CreateObject());
    }

    cJSON* subNode = cJSON_GetObjectItem(s_g_rootNode, nodeName);
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);

    va_list args;
    va_start(args, fmt);
    char logMsg[MAX_LOG_MSG_LEN];
    vsnprintf(logMsg, sizeof(logMsg), fmt, args);
    va_end(args);

    if (size == 1) {
        cJsonLoggerPushLog(subNode, logMsg);
    }

    else {
        cJsonLoggerLogRecur(&jsonPath[1], size - 1, subNode, logMsg);
    }
}

void cJsonLoggerDump()
{
    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    char* string = cJSON_Print(s_g_rootNode);
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);

    assert(string != NULL);

    FILE* file = fopen("log.json", "w");
    assert(file != NULL);

    fprintf(file, "%s", string);

    fclose(file);
    free(string);
}

void cJsonLoggerDelete()
{
    pthread_mutex_lock(&s_g_jsonLoggerMutex);
    cJSON_Delete(s_g_rootNode);
    pthread_mutex_unlock(&s_g_jsonLoggerMutex);
}