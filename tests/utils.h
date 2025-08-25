#ifndef UTILS_H
#define UTILS_H

#include <cJSON.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FUNC_NAME(fn) #fn
#define MAX_TEST_NAME_LEN 128 + 1
#define VECTOR_INIT_CAPACITY 64

typedef enum TestStatus {
    PASSED = 0,
    FAILED
} TestStatus_e;

typedef struct TestInfo {
    TestStatus_e status;
    char name[MAX_TEST_NAME_LEN];
} TestInfo_s;

static cJSON* s_g_testStatistics = NULL;

static void destroyTestSuite(void)
{
    char* string = cJSON_Print(s_g_testStatistics);

    FILE* file = fopen("test_report.json", "w");
    fprintf(file, "%s", string);

    fclose(file);
    free(string);
}

static void initTestSuite(void)
{
    s_g_testStatistics = cJSON_CreateObject();

#ifdef CJSONLOGGER_TEST_DEBUG
    cJSON_AddItemToObject(s_g_testStatistics, "TestMode", cJSON_CreateString("Debug"));
#endif

#ifdef CJSONLOGGER_TEST_RELEASE
    cJSON_AddItemToObject(s_g_testStatistics, "TestMode", cJSON_CreateString("Release"));
#endif

#ifdef CJSONLOGGER_TEST_DIST
    cJSON_AddItemToObject(s_g_testStatistics, "TestMode", cJSON_CreateString("Dist"));
#endif

    cJSON_AddItemToObject(s_g_testStatistics, "Passed", cJSON_CreateArray());
    cJSON_AddItemToObject(s_g_testStatistics, "Failed", cJSON_CreateArray());

    atexit(destroyTestSuite);
}

static void pushStats(TestInfo_s* testInfo, int expectedResult, int childExitStatus)
{
    cJSON* testStatusArray = NULL;
    if (testInfo->status == PASSED) {
        testStatusArray = cJSON_GetObjectItem(s_g_testStatistics, "Passed");
    } else {
        testStatusArray = cJSON_GetObjectItem(s_g_testStatistics, "Failed");
    }

    cJSON* report = cJSON_CreateObject();
    cJSON_AddItemToObject(report, "TestName", cJSON_CreateString(testInfo->name));
    cJSON_AddItemToObject(report, "expected", cJSON_CreateNumber(expectedResult));
    cJSON_AddItemToObject(report, "actual", cJSON_CreateNumber(childExitStatus));

    cJSON_AddItemToArray(testStatusArray, report);
}

#define RUN_TEST(expectedResult, func, ...)                                                                               \
    do {                                                                                                                  \
        TestInfo_s testInfo;                                                                                              \
        int childExitStatus;                                                                                              \
        snprintf(testInfo.name, MAX_TEST_NAME_LEN, "%s", FUNC_NAME(func));                                                \
        printf("Running test [%s] ...\n", testInfo.name);                                                                 \
        if (fork() == 0) {                                                                                                \
            return func(__VA_ARGS__);                                                                                     \
        } else {                                                                                                          \
            wait(&childExitStatus);                                                                                       \
        }                                                                                                                 \
        if (childExitStatus == expectedResult) {                                                                          \
            testInfo.status = PASSED;                                                                                     \
            printf("Test [%s] passed\n", testInfo.name);                                                                  \
        } else {                                                                                                          \
            testInfo.status = FAILED;                                                                                     \
            printf("Test [%s] failed, expected status [%d], got [%d]\n", testInfo.name, expectedResult, childExitStatus); \
        }                                                                                                                 \
        pushStats(&testInfo, expectedResult, childExitStatus);                                                            \
    } while (0)

#endif // UTILS_H