/**
 * @file multithread_tests.c
 * @brief Multithreaded tests for cJSONLogger (designed to be used with the helgrind tool).
 * 
 * @author Stefanos Tononidis
 * @date 2025-09-02
 */

#include <cJSONLogger.h>

#include <assert.h>
#include <pthread.h>

/**
 * @def RUN_TEST
 *
 * @brief Spawn two threads and run the specified functions in each thread.
 *
 * @param funcA The function to run in the first thread.
 * @param funcB The function to run in the second thread.
 */
#define RUN_TEST(funcA, funcB)                              \
    do {                                                    \
        int res = -1;                                       \
        pthread_t pThreadA = 0;                             \
        pthread_t pThreadB = 0;                             \
        res = pthread_create(&pThreadA, NULL, funcA, NULL); \
        assert(res == 0);                                   \
        res = pthread_create(&pThreadB, NULL, funcB, NULL); \
        assert(res == 0);                                   \
        res = pthread_join(pThreadA, NULL);                 \
        assert(res == 0);                                   \
        res = pthread_join(pThreadB, NULL);                 \
        assert(res == 0);                                   \
    } while (0)

/**
 * @def LOG_FILE
 *
 * @brief The log file path where the cJSONLogger test logs are stored.
 */
#define LOG_FILE "log.json"

static void* initLoggerHandler(void* ctx)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    return NULL;
}

static void* logHandler(void* ctx)
{
    CJSON_LOG_CRITICAL(NULL, "foo");
    return NULL;
}

static void* dumpHandler(void* ctx)
{
    cJSONLoggerDump();
    return NULL;
}

static void* rotateHandler(void* ctx)
{
    cJSONLoggerRotate();
    return NULL;
}

static void* destroyHandler(void* ctx)
{
    cJSONLoggerDestroy();
    return NULL;
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
    RUN_TEST(initLoggerHandler, initLoggerHandler);
    RUN_TEST(logHandler, logHandler);
    RUN_TEST(dumpHandler, dumpHandler);
    RUN_TEST(initLoggerHandler, initLoggerHandler);
    RUN_TEST(destroyHandler, destroyHandler);
    RUN_TEST(initLoggerHandler, destroyHandler);

    return 0;
}