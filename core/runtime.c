#include "runtime.h"

#include "buttons.h"
#include "playdate.h"   // for LCD_ROWS

static int update(void *);
static int update_mode(int mode, display_slice slice);

struct runtime runtime = {
    .update = update,
    .next_mode = 0,
};

static int runtime_mode = -1;
static int runtime_transition = 0;

static int update(void *) {
    buttons_update();
    if (runtime.next_mode != runtime_mode) {
        runtime_transition += 6;
        if (runtime_transition >= LCD_ROWS) {
            runtime_transition = 0;
            runtime_mode = runtime.next_mode;
            playdate->system->logToConsole("finished transition");
        } else {
            update_mode(runtime.next_mode, (display_slice){
                .start_row = 0,
                .end_row = runtime_transition,
            });
        }
    }
    update_mode(runtime_mode, (display_slice){
        .start_row = runtime_transition,
        .end_row = LCD_ROWS,
    });
}

static int update_mode(int mode, display_slice slice) {
    // TODO: update buttons here
    switch (mode) {
        // TODO: make this a "wipe" mode, which switches back to a "runtime.return_mode"
        case -1:
            display_clear(slice, 85);
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
