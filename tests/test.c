#include "utils.h"

#include <cJSONLogger.h>

#include <sys/wait.h>
#include <unistd.h>

#define SIGNAL_BASE 128

typedef enum ReturnStatus {
    OK = 0,
    ERROR,
    FORK_EXITED // Will never be returned because the child process was terminated abnormally.
} ReturnStatus_e;

int test_cJSONLogger_log_without_init_with_disabled_severity(void)
{
    CJSON_LOG_INFO(NULL, "");
    return OK;
}

int test_cJSONLogger_log_without_init_with_enabled_severity(void)
{
    cJSONLoggerSetLogLevel(CJSON_LOG_LEVEL_INFO);
    CJSON_LOG_INFO(NULL, "");
    return FORK_EXITED;
}

int main(void)
{
    initTestSuite();

#ifdef CJSONLOGGER_TEST_DEBUG

    RUN_TEST(OK, test_cJSONLogger_log_without_init_with_disabled_severity);
    RUN_TEST(SIGNAL_BASE + SIGABRT, test_cJSONLogger_log_without_init_with_enabled_severity);

#else

    RUN_TEST(OK, test_cJSONLogger_log_without_init_with_disabled_severity);
    RUN_TEST(SIGNAL_BASE + SIGSEGV, test_cJSONLogger_log_without_init_with_enabled_severity);

#endif

    return 0;
}