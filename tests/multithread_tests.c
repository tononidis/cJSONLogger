/**
 * @file multithread_tests.c
 *
 * @brief Multithreaded tests for cJSONLogger (designed to be used with the helgrind tool).
 *
 * @author Stefanos Tononidis
 *
 * @date 2025-09-02
 */

#include <cJSONLogger.h>

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

/**
 * @brief A simple dynamic array (vector) implementation for storing pointers to data.
 */
typedef struct Vector {
    void** data;
    size_t size;
    size_t capacity;
} Vector_s;

/**
 * @brief Initializes a vector with the specified capacity.
 *
 * @param capacity The initial capacity of the vector.
 *
 * @return A pointer to the initialized vector.
 */
Vector_s* vectorInit(size_t capacity)
{
    Vector_s* vector = malloc(sizeof(Vector_s));
    assert(vector != NULL);
    vector->data = malloc(capacity * sizeof(void*));
    assert(vector->data != NULL);
    vector->size = 0;
    vector->capacity = capacity;
    return vector;
}

/**
 * @brief Destroys the vector and frees its resources.
 *
 * @param vector The vector to destroy.
 */
void vectorDestroy(Vector_s* vector)
{
    if (vector != NULL) {
        if (vector->data != NULL) {
            free(vector->data);
            vector->data = NULL;
        }
        free(vector);
        vector = NULL;
    }
}

/**
 * @brief Push an item (void pointer to address) to the end of the vector, resizing if necessary.
 *
 * @param vector The vector to push the item to.
 *
 * @param item The item to push.
 */
void vectorPushBack(Vector_s* vector, void* item)
{
    assert(vector != NULL);
    if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        vector->data = realloc(vector->data, vector->capacity * sizeof(void*));
        assert(vector->data != NULL);
    }

    vector->data[vector->size] = item;
    vector->size++;
}

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

    Vector_s* createdFiles_v = vectorInit(8);

    while (atomic_load(&s_g_threadFlag) != 0) {
        char buffer[1024];
        int length = read(fd, buffer, sizeof(buffer));
        assert(length >= 0);
        struct inotify_event* event = (struct inotify_event*)&buffer;

        assert(event->len > 0);
        assert(event->mask & IN_CREATE);

        vectorPushBack(createdFiles_v, strdup(event->name));
    }

    int res = -1;
    res = inotify_rm_watch(fd, wd);
    assert(res == 0);

    res = close(fd);
    assert(res == 0);

    return createdFiles_v;
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
    CJSON_LOG_CRITICAL("foo");
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

    Vector_s* createdFiles_v = NULL;
    pthread_join(fileWatcherThread, (void**)&createdFiles_v);

    for (size_t i = 0; i < createdFiles_v->size; i++) {
        int res = remove((char*)createdFiles_v->data[i]);
        assert(res == 0);
        free(createdFiles_v->data[i]);
    }

    vectorDestroy(createdFiles_v);
    createdFiles_v = NULL;

    return 0;
}