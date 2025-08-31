/**
 * @file test.c
 *
 * @brief This file contains the tests for the cJSON logger library.
 *
 * @author Stefanos Tononidis
 *
 * @date 2025-08-26
 */

#include <cJSONLogger.h>

#include <sys/wait.h>
#include <unistd.h>

#include "test.h"

/**
 * @brief Test the logging behavior when the severity level is not reached but the cJSONLogger is not initialized.
 * *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_log_without_init_with_disabled_severity(void)
{
    CJSON_LOG_INFO(NULL, "");
    return PASSED;
}

/**
 * @brief Test the logging behavior when the severity level is reached but the cJSONLogger is not initialized.
 *
 * @warning This test failed an assertion debug builds.
 *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_log_without_init_with_enabled_severity(void)
{
    cJSONLoggerSetLogLevel(CJSON_LOG_LEVEL_INFO);
    CJSON_LOG_INFO(NULL, "");
    return PASSED;
}

/**
 * @brief Test the logging behavior with no node object.
 * *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_log_no_node(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    CJSON_LOG_CRITICAL(NULL, "bar");
    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* jsonLogsDoc = cJSON_Parse(logData);
    free(logData);

    if (jsonLogsDoc == NULL) {
        return FAILED;
    }

    cJSON* logsArray = cJSON_GetObjectItem(jsonLogsDoc, "logs");
    if (logsArray == NULL || !cJSON_IsArray(logsArray)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logItem = cJSON_GetArrayItem(logsArray, 0);
    if (logItem == NULL || !cJSON_IsObject(logItem)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logLevelEntry = cJSON_GetObjectItem(logItem, "LogLevel");
    if (logLevelEntry == NULL || !cJSON_IsString(logLevelEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(logLevelEntry->valuestring, "CRITICAL", strlen("CRITICAL")) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logEntry = cJSON_GetObjectItem(logItem, "log");
    if (logEntry == NULL || !cJSON_IsString(logEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(logEntry->valuestring, "bar", strlen("bar")) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* timeEntry = cJSON_GetObjectItem(logItem, "Time");
    if (timeEntry == NULL || !cJSON_IsString(timeEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strlen(logEntry->valuestring) <= 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fileNameEntry = cJSON_GetObjectItem(logItem, "FileName");
    if (fileNameEntry == NULL || !cJSON_IsString(fileNameEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(fileNameEntry->valuestring, __FILENAME__, strlen(__FILENAME__)) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* funcNameEntry = cJSON_GetObjectItem(logItem, "FuncName");
    if (funcNameEntry == NULL || !cJSON_IsString(funcNameEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(funcNameEntry->valuestring, __FUNCTION__, strlen(__FUNCTION__)) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fileLineEntry = cJSON_GetObjectItem(logItem, "FileLine");
    if (fileLineEntry == NULL || !cJSON_IsNumber(fileLineEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (fileLineEntry->valueint <= 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON_Delete(jsonLogsDoc);
    jsonLogsDoc = NULL;

    return PASSED;
}
/**
 * @brief Test the logging behavior with one node object.
 * *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_log_one_node(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    char* jsonPath[] = { "foo" };
    CJSON_LOG_INFO(jsonPath, "bar");
    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* jsonLogsDoc = cJSON_Parse(logData);
    free(logData);

    if (jsonLogsDoc == NULL) {
        return FAILED;
    }

    if (cJSON_HasObjectItem(jsonLogsDoc, "foo") == 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fooNode = cJSON_GetObjectItem(jsonLogsDoc, "foo");
    if (fooNode == NULL || cJSON_IsObject(fooNode) == 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logsArray = cJSON_GetObjectItem(fooNode, "logs");
    if (logsArray == NULL || !cJSON_IsArray(logsArray)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logItem = cJSON_GetArrayItem(logsArray, 0);
    if (logItem == NULL || !cJSON_IsObject(logItem)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logLevelEntry = cJSON_GetObjectItem(logItem, "LogLevel");
    if (logLevelEntry == NULL || !cJSON_IsString(logLevelEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(logLevelEntry->valuestring, "INFO", strlen("INFO")) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logEntry = cJSON_GetObjectItem(logItem, "log");
    if (logEntry == NULL || !cJSON_IsString(logEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(logEntry->valuestring, "bar", strlen("bar")) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* timeEntry = cJSON_GetObjectItem(logItem, "Time");
    if (timeEntry == NULL || !cJSON_IsString(timeEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strlen(logEntry->valuestring) <= 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fileNameEntry = cJSON_GetObjectItem(logItem, "FileName");
    if (fileNameEntry == NULL || !cJSON_IsString(fileNameEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(fileNameEntry->valuestring, __FILENAME__, strlen(__FILENAME__)) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* funcNameEntry = cJSON_GetObjectItem(logItem, "FuncName");
    if (funcNameEntry == NULL || !cJSON_IsString(funcNameEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(funcNameEntry->valuestring, __FUNCTION__, strlen(__FUNCTION__)) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fileLineEntry = cJSON_GetObjectItem(logItem, "FileLine");
    if (fileLineEntry == NULL || !cJSON_IsNumber(fileLineEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (fileLineEntry->valueint <= 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON_Delete(jsonLogsDoc);
    jsonLogsDoc = NULL;

    return PASSED;
}

/**
 * @brief Test the logging behavior with three nested objects.
 * *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_log_three_nodes(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    char* jsonPath[] = { "foo", "bar", "baz" };
    CJSON_LOG_ERROR(jsonPath, "qux");
    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* jsonLogsDoc = cJSON_Parse(logData);
    free(logData);

    if (jsonLogsDoc == NULL) {
        return FAILED;
    }

    if (cJSON_HasObjectItem(jsonLogsDoc, "foo") == 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fooNode = cJSON_GetObjectItem(jsonLogsDoc, "foo");
    if (fooNode == NULL || cJSON_IsObject(fooNode) == 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* barNode = cJSON_GetObjectItem(fooNode, "bar");
    if (barNode == NULL || cJSON_IsObject(barNode) == 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* bazNode = cJSON_GetObjectItem(barNode, "baz");
    if (bazNode == NULL || cJSON_IsObject(bazNode) == 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logsArray = cJSON_GetObjectItem(bazNode, "logs");
    if (logsArray == NULL || !cJSON_IsArray(logsArray)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logItem = cJSON_GetArrayItem(logsArray, 0);
    if (logItem == NULL || !cJSON_IsObject(logItem)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logLevelEntry = cJSON_GetObjectItem(logItem, "LogLevel");
    if (logLevelEntry == NULL || !cJSON_IsString(logLevelEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(logLevelEntry->valuestring, "ERROR", strlen("ERROR")) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* logEntry = cJSON_GetObjectItem(logItem, "log");
    if (logEntry == NULL || !cJSON_IsString(logEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(logEntry->valuestring, "qux", strlen("qux")) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* timeEntry = cJSON_GetObjectItem(logItem, "Time");
    if (timeEntry == NULL || !cJSON_IsString(timeEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strlen(logEntry->valuestring) <= 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fileNameEntry = cJSON_GetObjectItem(logItem, "FileName");
    if (fileNameEntry == NULL || !cJSON_IsString(fileNameEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(fileNameEntry->valuestring, __FILENAME__, strlen(__FILENAME__)) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* funcNameEntry = cJSON_GetObjectItem(logItem, "FuncName");
    if (funcNameEntry == NULL || !cJSON_IsString(funcNameEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (strncmp(funcNameEntry->valuestring, __FUNCTION__, strlen(__FUNCTION__)) != 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON* fileLineEntry = cJSON_GetObjectItem(logItem, "FileLine");
    if (fileLineEntry == NULL || !cJSON_IsNumber(fileLineEntry)) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    if (fileLineEntry->valueint <= 0) {
        return failAndRelease(NULL, jsonLogsDoc);
    }

    cJSON_Delete(jsonLogsDoc);
    jsonLogsDoc = NULL;

    return PASSED;
}

/**
 * @brief Test the logging behavior when the severity level is not reached.
 * *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_severity_not_reached(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    char* jsonPath[] = { "foo" };
    CJSON_LOG_DEBUG(jsonPath, "bar");
    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* expectedLogs = cJSON_CreateObject();
    char* expectedLogsStr = cJSON_Print(expectedLogs);
    cJSON_Delete(expectedLogs);

    int ret = FAILED;

    if (strncmp(expectedLogsStr, logData, strlen(logData)) == 0) {
        ret = PASSED;
    }

    free(logData);
    free(expectedLogsStr);

    return ret;
}

/**
 * @brief Test the logging behavior when the severity level is reached.
 *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_severity_reached(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    char* jsonPath[] = { "foo" };

    cJSONLoggerSetLogLevel(CJSON_LOG_LEVEL_DEBUG);
    CJSON_LOG_DEBUG(jsonPath, "bar");
    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* expectedLogs = cJSON_CreateObject();
    char* expectedLogsStr = cJSON_Print(expectedLogs);
    cJSON_Delete(expectedLogs);

    int ret = FAILED;

    if (strncmp(expectedLogsStr, logData, strlen(logData)) != 0) {
        ret = PASSED;
    }

    free(logData);
    free(expectedLogsStr);

    return ret;
}

/**
 * @brief Test the logging behavior when the cJSON logger is destroyed and logs are attempted to be written.
 *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_destroy(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    cJSONLoggerDestroy();

    CJSON_LOG_CRITICAL(NULL, "bar");

    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* expectedLogs = cJSON_CreateObject();
    char* expectedLogsStr = cJSON_Print(expectedLogs);
    cJSON_Delete(expectedLogs);

    int ret = FAILED;
    if (strncmp(expectedLogsStr, logData, strlen(logData)) == 0) {
        ret = PASSED;
    }

    free(logData);
    free(expectedLogsStr);

    return ret;
}

/**
 * @brief Test the cJSON logger dump functionality.
 *
 * @return int, PASSED if the test passes, FAILED otherwise, values defined in enum TestStatus.
 */
static int test_cJSONLogger_dump(void)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);

    CJSON_LOG_INFO(NULL, "bar");

    cJSONLoggerDump();

    char* logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    cJSON* loggedJson = cJSON_Parse(logData);
    free(logData);
    logData = NULL;

    if (loggedJson == NULL) {
        return FAILED;
    }

    char* jsonNode[] = { "foo" };
    CJSON_LOG_INFO(jsonNode, "bar");

    if (cJSON_GetObjectItem(loggedJson, "foo") != NULL) {
        return failAndRelease(NULL, loggedJson);
    }

    cJSON_Delete(loggedJson);
    loggedJson = NULL;

    cJSONLoggerDump();

    logData = readFile(LOG_FILE);
    if (logData == NULL) {
        return FAILED;
    }

    loggedJson = cJSON_Parse(logData);
    free(logData);

    if (loggedJson == NULL) {
        return FAILED;
    }

    if (cJSON_GetObjectItem(loggedJson, "foo") == NULL) {
        return failAndRelease(NULL, loggedJson);
    }

    return PASSED;
}

/*
 * @brief Entry point for cJSONLogger tests.
 *
 * @note Some tests run differently on different build configurations.
 *
 * @warning the initTestSuite() should always be called before any tests are run.
 *
 * @return int, always 0.
 */
int main(void)
{
    initTestSuite();

    RUN_TEST(PASSED, test_cJSONLogger_log_without_init_with_disabled_severity);
    RUN_TEST(PASSED, test_cJSONLogger_log_no_node);
    RUN_TEST(PASSED, test_cJSONLogger_log_one_node);
    RUN_TEST(PASSED, test_cJSONLogger_log_three_nodes);
    RUN_TEST(PASSED, test_cJSONLogger_severity_not_reached);
    RUN_TEST(PASSED, test_cJSONLogger_severity_reached);
    RUN_TEST(PASSED, test_cJSONLogger_destroy);
    RUN_TEST(PASSED, test_cJSONLogger_dump);

#ifdef CJSONLOGGER_TEST_DEBUG

    // The bellow test expects an assert to fail only in debug builds, raising an abort() (SIGABRT)
    RUN_TEST(SIGNAL_BASE + SIGABRT, test_cJSONLogger_log_without_init_with_enabled_severity);

#else

    RUN_TEST(PASSED, test_cJSONLogger_log_without_init_with_enabled_severity);

#endif

    return 0;
}