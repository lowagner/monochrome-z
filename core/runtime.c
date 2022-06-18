#include "runtime.h"

#include "buttons.h"
#include "playdate.h"   // for LCD_ROWS

static int update(void *);
static int update_mode(int mode, display_slice slice);

struct runtime runtime = {
    .update = update,
    .next_mode = kRuntimeModeNone,
    .transition = 0,
};

static int runtime_mode = kRuntimeModeWipe;

static int update(void *) {
    buttons_update();
    if (runtime.next_mode != runtime_mode) {
        runtime.transition += 6;
        if (runtime.transition >= LCD_ROWS) {
            runtime.transition = 0;
            runtime_mode = runtime.next_mode;
            playdate->system->logToConsole("finished transition");
            goto no_transition_update;
        }
        // in case someone modifies runtime.transition in an update,
        // make sure to use a consistent one for both display slices:
        int runtime_transition = runtime.transition;
        update_mode(runtime.next_mode, (display_slice){
            .start_row = 0,
            .end_row = runtime_transition - 2,
        });
        update_mode(runtime_mode, (display_slice){
            .start_row = runtime_transition,
            .end_row = LCD_ROWS,
        });
        // draw a line between the modes:
        display_clear(255, (display_slice){
            .start_row = runtime_transition - 2,
            .end_row = runtime_transition,
        });
    } else {
        no_transition_update:
        update_mode(runtime_mode, (display_slice){
            .start_row = 0,
            .end_row = LCD_ROWS,
        });
    }
}

static int update_mode(int mode, display_slice slice) {
    switch (mode) {
        // TODO: make this a "wipe" mode, which switches back to a "runtime.return_mode"
        case kRuntimeModeWipe:
            display_clear(85, slice);
            return 1;
        // TODO: define these in the .h files, so they're automatically
        // pulled in when adding game/runtime modes.
        #ifdef SOME_RUNTIME_MODE
        case some_runtime_mode:
            some_runtime_update(slice);
            return 1;
        #endif
        default:
            default_update(slice);
            return 1;
    }
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t argument) {
    switch (event) {
        case kEventInit:
            playdate_init(pd);
            initialize();
            break;
        case kEventKeyPressed:
            pd->system->logToConsole("key press %d", argument);
            break;
        case kEventKeyReleased:
            pd->system->logToConsole("key release %d", argument);
            break;
    }
    return 0;
}
