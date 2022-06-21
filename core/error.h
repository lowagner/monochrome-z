#pragma once

#include <string.h>

#define ERROR_BUFFER_SIZE 4096
// TODO: replace with __VA_ARGS__
#define AND ,
#define _AND AND
#define __AND _AND
#define AT __FILE__ ":" STRINGIFY(__LINE__)
// may or may not crash the program, depending on test status:
#define ERROR(X) error(X);
// will definitely crash the program, regardless of test status:
#define DEFINITELY_ERROR(X) playdate->system->error(X);
#define NOTHING ((void)0)
#define STRINGIFY(X) _STRINGIFY(X)
#define _STRINGIFY(X) #X

#ifdef NDEBUG
    // RELEASE mode, no debug:
    #define DEBUG_ONLY(x) NOTHING
#else
    // DEBUG mode (not NDEBUG):
    #define DEBUG_ONLY(X) X
#endif

#define _ASSERT(e, X) DEBUG_ONLY({ \
    if (!(X)) { \
        e("expected " #X); \
    } \
})
#define ASSERT(X) _ASSERT(playdate->system->error, X)
#define ASSERT_LOGGED(X) _ASSERT(error_log, X)

#define _TEST_WITH_CONTEXT(e, x, contextFormat, ...) DEBUG_ONLY({ \
    ++error_test_only; \
    x; \
    { \
        const char *error = error_pull(); \
        if (error[0] != 0) { \
            e(contextFormat ": %s", __VA_ARGS__, error); \
        } \
    } \
    --error_test_only; \
})
#define TEST_WITH_CONTEXT(x, contextFormat, ...) \
    _TEST_WITH_CONTEXT(playdate->system->error, x, contextFormat, __VA_ARGS__)
#define TEST_WITH_CONTEXT_LOGGED(x, contextFormat, ...) \
    _TEST_WITH_CONTEXT(error_log, x, contextFormat, __VA_ARGS__)

#define TEST(x) DEBUG_ONLY({ \
    ++error_test_only; \
    x; \
    { \
        const char *error = error_pull(); \
        if (error[0] != 0) { \
            DEFINITELY_ERROR("%s: %s" __AND AT __AND error); \
        } \
    } \
    --error_test_only; \
})
#define _EXPECT_EQUAL(e, t, format, x, y) DEBUG_ONLY({ \
    t test_x = (x); \
    t test_y = (y); \
    if (test_x != test_y) { \
        e(#x " = " format " != " #y " = " format, test_x, test_y); \
    } \
})
#define EXPECT_INT_EQUAL(x, y) \
    _EXPECT_EQUAL(playdate->system->error, int, "%d", x, y)
#define EXPECT_INT_EQUAL_LOGGED(x, y) \
    _EXPECT_EQUAL(error_log, int, "%d", x, y)

#define EXPECT_ERROR(expected_error_input, run) DEBUG_ONLY({ \
    run; \
    { \
        const char *error = error_pull(); \
        const char *expected_error = (expected_error_input); \
        if (strncmp(error, expected_error, ERROR_BUFFER_SIZE)) { \
            DEFINITELY_ERROR("expected `" expected_error_input "`, got error `%s`" AND error); \
        } \
    } \
})

#include "playdate.h"

#include <stdarg.h>

// stops execution (if not in a test):
void error(const char *format, ...);
// logs an error (or warning).
void error_log(const char *format, ...);
// logs an error (or warning), with variable argument list.
void error_log_variable(const char *format, va_list variable_arguments);

DEBUG_ONLY(extern int error_test_only);

DEBUG_ONLY(const char *error_pull());
