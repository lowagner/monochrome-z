#include "runtime.h"

static int update(void *);

struct runtime runtime = {
    .update = update,
    .mode = 0,
};

static int update(void *) {
    // TODO: update buttons here
    switch (runtime.mode) {
        // TODO: define these in the .h files, so they're automatically
        // pulled in when adding game/runtime modes.
        #ifdef SOME_RUNTIME_MODE
        case some_runtime_mode:
            return some_runtime_update();
        #endif
        default:
            return default_update();
    }
}
