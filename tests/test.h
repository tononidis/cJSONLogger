/**
 * @file test.h
 * @brief This file contains utility functions for testing the cJSON logger library.
 *
 * @author Stefanos Tononidis
 * @date 2025-08-26
 */

#ifndef TEST_H
#define TEST_H

#include <cJSON.h>

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>

/**
 * @def FUNC_NAME
 *
 * @brief Get the string representation of a function name.
 *
 * @param fn The function name.
 *
 * @return The string representation of the function name.
 */
#define FUNC_NAME(fn) #fn

/**
 * @def LOG_FILE
 *
 * @brief The log file path where the cJSONLogger test logs are stored.
 */
#define LOG_FILE "log.json"

/**
 * @def MAX_STRING_LEN
 *
 * @brief The maximum length of a string to be allocated during any test.
 */
#define MAX_STRING_LEN 128 + 1

/**
 * @def MAX_TEST_NAME_LEN
 *
 * @brief The maximum length of a test name (as a function).
 */
#define MAX_TEST_NAME_LEN 128 + 1

/**
 * @def RELEASE_RESOURCE_AND_RETURN_FAIL
 *
 * @brief Release a resource and return failure.
 *
 * @param ptr The pointer to the resource to be freed.
 *
 * @param freeFunc The function to call to free the resource.
 *
 * @return int, always FAILED.
 */
#define RELEASE_RESOURCE_AND_RETURN_FAIL(ptr, freeFunc) \
    do {                                                \
        if (ptr != NULL) {                              \
            freeFunc(ptr);                              \
            ptr = NULL;                                 \
        }                                               \
        return FAILED;                                  \
    } while (0)

/**
 * @enum TestStatus
 *
 * @brief Enumeration used to evaluate a test status.
 */
typedef enum TestStatus {
    PASSED = 0,
    FAILED
} TestStatus_e;

/**
 * @struct TestInfo
 *
 * @brief Structure used to store information about a test case.
 *
 * @var status The status of the test case, either PASSED or FAILED.
 *
 * @var name The name of the test case.
 */
typedef struct TestInfo {
    TestStatus_e status;
    char name[MAX_TEST_NAME_LEN];
} TestInfo_s;

/**
 * @brief Pointer to the global test statistics JSON object.
 */
static cJSON* s_g_testStatistics = NULL;

/**
 * @brief Process id returned from forking the test process.
 */
static pid_t s_g_pid = -1;

/**
 * @brief Free forked resources by the child process.
 */
void freeChildResources(void)
{
    if (s_g_testStatistics != NULL) {
        cJSON_Delete(s_g_testStatistics);
        s_g_testStatistics = NULL;
    }
}

/**
 * @brief Read the contents of a file into a string.
 *
 * @param fileName The name of the file to read.
 *
 * @warning returned string must be freed when no longer needed.
 *
 * @return char* A pointer to the contents of the file, or NULL on failure.
 */
static char* readFile(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t logSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* logData = malloc(logSize + 1);
    if (logData != NULL) {
        if (fread(logData, sizeof(char), logSize, file) != logSize) {
            free(logData);
            logData = NULL;

            fclose(file);
            return NULL;
        }
        logData[logSize] = '\0';
    }

    else {
        fclose(file);
        return NULL;
    }

    fclose(file);
    return logData;
}

/**
 * @brief Destroys the test suite, cleans up resources and generates the test report.
 *
 * @note This function is called automatically at program exit to ensure all resources are released and the test report is generated.
 */
static void destroyTestSuite(void)
{
    if (s_g_pid == 0) {
        if (s_g_testStatistics != NULL) {
            cJSON_Delete(s_g_testStatistics);
            s_g_testStatistics = NULL;
        }
        return;
    }

    if (s_g_testStatistics != NULL) {
        cJSON* passed = cJSON_GetObjectItem(s_g_testStatistics, "Passed");
        cJSON* failed = cJSON_GetObjectItem(s_g_testStatistics, "Failed");

        assert(passed != NULL);
        assert(failed != NULL);

        cJSON_AddItemToObject(s_g_testStatistics, "Summary", cJSON_CreateObject());
        cJSON* summary = cJSON_GetObjectItem(s_g_testStatistics, "Summary");
        assert(summary != NULL);

        cJSON_AddItemToObject(summary, "Passed", cJSON_CreateNumber(cJSON_GetArraySize(passed)));
        cJSON_AddItemToObject(summary, "Failed", cJSON_CreateNumber(cJSON_GetArraySize(failed)));

        char* testsStatisticsStr = cJSON_Print(s_g_testStatistics);
        assert(testsStatisticsStr != NULL);

        cJSON_Delete(s_g_testStatistics);
        s_g_testStatistics = NULL;

        FILE* file = fopen("test_report.json", "w");
        assert(file != NULL);

        fprintf(file, "%s", testsStatisticsStr);

        fclose(file);
        file = NULL;

        free(testsStatisticsStr);
        testsStatisticsStr = NULL;
    }

    if (remove(LOG_FILE) != 0) {
        printf("Could not delete file [%s]\n", LOG_FILE);
    }
}

/**
 * @brief Initializes the test suite and sets up necessary resources.
 *
 * @note This function registers and atexit callback to destroy the test suite.
 */
static void initTestSuite(void)
{
    s_g_testStatistics = cJSON_CreateObject();
    assert(s_g_testStatistics != NULL);

/**
 * @def CJSONLOGGER_TEST_DEBUG
 *
 * @brief When testing a debug build.
 */
#ifdef CJSONLOGGER_TEST_DEBUG
    cJSON_AddItemToObject(s_g_testStatistics, "TestMode", cJSON_CreateString("Debug"));
#endif

/**
 * @def CJSONLOGGER_TEST_DEBUG
 *
 * @brief When testing a release build.
 */
#ifdef CJSONLOGGER_TEST_RELEASE
    cJSON_AddItemToObject(s_g_testStatistics, "TestMode", cJSON_CreateString("Release"));
#endif

/**
 * @def CJSONLOGGER_TEST_DEBUG
 *
 * @brief When testing a distribution build.
 */
#ifdef CJSONLOGGER_TEST_DIST
    cJSON_AddItemToObject(s_g_testStatistics, "TestMode", cJSON_CreateString("Dist"));
#endif

    cJSON_AddItemToObject(s_g_testStatistics, "Passed", cJSON_CreateArray());
    cJSON_AddItemToObject(s_g_testStatistics, "Failed", cJSON_CreateArray());

    int ret = atexit(destroyTestSuite);
    assert(ret == 0);
}

/**
 * @brief Pushes the test statistics for a single test case.
 *
 * @param testInfo Pointer to the TestInfo structure containing test information.
 * @param expectedResult The expected result (exit status) of the test.
 * @param childExitStatus The actual result (exit status) of the test.
 */
static void pushStats(TestInfo_s* testInfo, int expectedResult, int childExitStatus)
{
    cJSON* testStatusArray = NULL;
    if (testInfo->status == PASSED) {
        testStatusArray = cJSON_GetObjectItem(s_g_testStatistics, "Passed");
    } else {
        testStatusArray = cJSON_GetObjectItem(s_g_testStatistics, "Failed");
    }

    assert(testStatusArray != NULL);

    cJSON* report = cJSON_CreateObject();

    assert(report != NULL);

    cJSON_AddItemToObject(report, "TestName", cJSON_CreateString(testInfo->name));
    cJSON_AddItemToObject(report, "expected", cJSON_CreateNumber(expectedResult));
    cJSON_AddItemToObject(report, "actual", cJSON_CreateNumber(childExitStatus));

    cJSON_AddItemToArray(testStatusArray, report);
}

/**
 * @brief File watcher handler (for spawned threads) to monitor file changes in the current directory.
 *
 * @param ctx Pointer to the context (unused).
 *
 * @return void* Pointer to the name of first file that was created.
 */
static void* fileWatcherHandler(void* ctx)
{
    assert(ctx != NULL);

    int fd = inotify_init();
    assert(fd >= 0);

    int wd = inotify_add_watch(fd, "./", IN_CREATE);
    assert(wd >= 0);

    char buffer[1024];

    pthread_kill(*(pthread_t*)ctx, SIGUSR1);

    int length = read(fd, buffer, sizeof(buffer));
    assert(length >= 0);

    struct inotify_event* event = (struct inotify_event*)&buffer;

    assert(event->len > 0);
    assert(event->mask & IN_CREATE);

    int res = -1;

    res = inotify_rm_watch(fd, wd);
    assert(res == 0);

    res = close(fd);
    assert(res == 0);

    char* result = (char*)malloc(strlen(event->name) + 1);
    assert(result != NULL);

    strncpy(result, event->name, strlen(event->name));
    result[strlen(event->name)] = '\0';
    return result;
}

/**
 * @def RUN_TEST
 *
 * @brief Fork and run a test case.
 *
 * @param expectedResult The expected result (exit status) of the test.
 * @param func The test function to run.
 * @param ... Additional arguments to pass to the test function.
 */
#define RUN_TEST(expectedResult, func, ...)                                                                                   \
    do {                                                                                                                      \
        s_g_pid = fork();                                                                                                     \
        if (s_g_pid == 0) {                                                                                                   \
            freeChildResources();                                                                                             \
            return func(__VA_ARGS__);                                                                                         \
        }                                                                                                                     \
                                                                                                                              \
        else {                                                                                                                \
            TestInfo_s testInfo;                                                                                              \
            int childExitStatus;                                                                                              \
            snprintf(testInfo.name, MAX_TEST_NAME_LEN, "%s", FUNC_NAME(func));                                                \
            printf("Running test [%s] ...\n", testInfo.name);                                                                 \
            wait(&childExitStatus);                                                                                           \
                                                                                                                              \
            if (expectedResult == childExitStatus) {                                                                          \
                testInfo.status = PASSED;                                                                                     \
                printf("Test [%s] passed\n", testInfo.name);                                                                  \
            }                                                                                                                 \
                                                                                                                              \
            else {                                                                                                            \
                testInfo.status = FAILED;                                                                                     \
                printf("Test [%s] failed, expected status [%d], got [%d]\n", testInfo.name, expectedResult, childExitStatus); \
            }                                                                                                                 \
                                                                                                                              \
            pushStats(&testInfo, expectedResult, childExitStatus);                                                            \
        }                                                                                                                     \
    } while (0)

#endif // TEST_H