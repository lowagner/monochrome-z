#pragma once

#define AND ,
#define AT __FILE__ ":" STRINGIFY(__LINE__)
#define ERROR(X) error(X);
#define NOTHING ((void)0)
#define STRINGIFY(X) _STRINGIFY(X)
#define _STRINGIFY(X) #X

#ifdef NDEBUG
    // RELEASE mode, no debug:
    #define ASSERT(X) NOTHING
    #define ASSERT_PROBABLY(X) NOTHING
    #define DEBUG_ONLY(x) NOTHING
#else
    // DEBUG mode (not NDEBUG):
    #define ASSERT(X) { \
        if (!(X)) { \
            ERROR("expected %s at %s" AND #X AND AT); \
        } \
    }
    #define ASSERT_PROBABLY(X) {if (!(X)) playdate->system->logToConsole("!!! expected " #X);}
    #define DEBUG_ONLY(X) X
#endif

#include "playdate.h"

#include <stdarg.h>

void error(const char *format, ...);

DEBUG_ONLY(extern int error_test_only);

DEBUG_ONLY(const char *error_pull());
