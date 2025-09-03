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
#include <stdatomic.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>

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

/**
 * @brief Global thread flag for controlling the file watcher thread.
 */
static atomic_int s_g_threadFlag = 1;

/**
 * @brief File watcher thread handler that is used to sanitize any created log files.
 *
 * @param ctx The context pointer (unused).
 *
 * @return always NULL
 */
static void* fileWatcherHandler(void* ctx)
{
    int fd = inotify_init();
    assert(fd >= 0);

    int wd = inotify_add_watch(fd, "./", IN_CREATE);
    assert(wd >= 0);

    while (atomic_load(&s_g_threadFlag) != 0) {
        char buffer[1024];
        int length = read(fd, buffer, sizeof(buffer));
        assert(length >= 0);
        struct inotify_event* event = (struct inotify_event*)&buffer;

        assert(event->len > 0);
        assert(event->mask & IN_CREATE);

        int res = remove(event->name);
        assert(res == 0);
    }

    int res = -1;
    res = inotify_rm_watch(fd, wd);
    assert(res == 0);

    res = close(fd);
    assert(res == 0);

    return NULL;
}

/**
 * @brief cJSONLogger initialization thread handler.
 *
 * @param ctx The context pointer (unused).
 *
 * @return always NULL
 */
static void* initLoggerHandler(void* ctx)
{
    cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, LOG_FILE);
    return NULL;
}

/**
 * @brief cJSONLogger log thread handler.
 *
 * @param ctx The context pointer (unused).
 *
 * @return always NULL
 */
static void* logHandler(void* ctx)
{
    CJSON_LOG_CRITICAL(NULL, "foo");
    return NULL;
}

/**
 * @brief cJSONLogger dump thread handler.
 *
 * @param ctx The context pointer (unused).
 *
 * @return always NULL
 */
static void* dumpHandler(void* ctx)
{
    cJSONLoggerDump();
    return NULL;
}

/**
 * @brief cJSONLogger file rotation thread handler.
 *
 * @param ctx The context pointer (unused).
 *
 * @return always NULL
 */
static void* rotateHandler(void* ctx)
{
    cJSONLoggerRotate();
    return NULL;
}

/**
 * @brief cJSONLogger destruction thread handler.
 *
 * @param ctx The context pointer (unused).
 *
 * @return always NULL
 */
static void* destroyHandler(void* ctx)
{
    cJSONLoggerDestroy();
    return NULL;
}

/**
 * @brief Entry point for cJSONLogger multithread tests (by using the helgrind tool).
 *
 * @return int, always 0.
 */
int main(void)
{
    pthread_t fileWatcherThread;
    pthread_create(&fileWatcherThread, NULL, fileWatcherHandler, NULL);

    RUN_TEST(initLoggerHandler, initLoggerHandler);
    RUN_TEST(logHandler, logHandler);
    RUN_TEST(logHandler, dumpHandler);
    RUN_TEST(logHandler, rotateHandler);
    RUN_TEST(dumpHandler, dumpHandler);
    RUN_TEST(rotateHandler, rotateHandler);
    RUN_TEST(destroyHandler, destroyHandler);
    RUN_TEST(initLoggerHandler, destroyHandler);
    RUN_TEST(initLoggerHandler, logHandler);

    // Force a final log rotation
    atomic_store(&s_g_threadFlag, 0);
    initLoggerHandler(NULL);
    rotateHandler(NULL);
    pthread_join(fileWatcherThread, NULL);

    return 0;
}