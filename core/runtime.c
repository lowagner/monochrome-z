#include "runtime.h"

#include "buttons.h"
#include "playdate.h"   // for LCD_ROWS

static int update(void *unused);
static void update_mode(int mode, display_slice_t slice);
static void update_transition_modes(unsigned int transition_counter, int top_mode, int bottom_mode);
static inline void runtime_menu_reset_value(runtime_menu_t *menu);

struct runtime runtime = {
    .update = update,
    .transition = {
        .next_mode = kRuntimeModeNone,
        .counter = 0,
        .speed = 10,
        .up = 1,
    },
};

static int runtime_mode = kRuntimeModeWipe;
static int runtime_menu_count = 0;

static runtime_menu_t *runtime_menus[3] = {NULL, NULL, NULL};

static int update(void *unused) {
    buttons_update();
    if (runtime.transition.next_mode != runtime_mode) {
        if (runtime.transition.speed > 200) {
            // make sure the old mode has a chance to react/clean up any resources
            runtime.transition.speed = 200;
        }
        runtime.transition.counter += runtime.transition.speed + 1;
        if (runtime.transition.counter >= LCD_ROWS) {
            runtime.transition.counter = 0;
            runtime_mode = runtime.transition.next_mode;
            playdate->system->logToConsole("finished transition to mode %d", runtime_mode);
            playdate->system->removeAllMenuItems();
            while (runtime_menu_count > 0) {
                runtime_menu_t *menu = runtime_menus[--runtime_menu_count];
                if (menu == NULL) {
                    playdate->system->logToConsole("expected a menu at %d", runtime_menu_count);
                    continue;
                }
                menu->pd_menu = NULL;
                runtime_menus[runtime_menu_count] = NULL;
            }
            goto no_transition_update;
        }
        if (runtime.transition.up) {
            // new mode transitions in from below.
            update_transition_modes(
                LCD_ROWS - runtime.transition.counter,
                // top mode:
                runtime_mode,
                // bottom mode:
                runtime.transition.next_mode
            );
        } else {
            // transition down.  new mode comes in from above:
            update_transition_modes(
                runtime.transition.counter,
                // top mode:
                runtime.transition.next_mode,
                // bottom mode:
                runtime_mode
            );
        }
    } else {
        no_transition_update:
        update_mode(runtime_mode, $(display_slice){
            .start_row = 0,
            .end_row = LCD_ROWS,
        });
    }
    return 1;
}

static void update_transition_modes(unsigned int transition_counter, int top_mode, int bottom_mode) {
    // in case someone modifies runtime.transition.counter in an update,
    // make sure to use a consistent one for both display slices, via passed-in transition_counter.
    // we also flip transition_counter based on different transition types (up vs. down).
    if (transition_counter >= 2) {
        update_mode(top_mode, $(display_slice){
            .start_row = 0,
            .end_row = transition_counter - 2,
        });
        // draw a line between the modes:
        display_slice_fill(255, $(display_slice){
            .start_row = transition_counter - 2,
            .end_row = transition_counter,
        });
    } else {
        // transition_counter should still be >= 0.
        display_slice_fill(255, $(display_slice){
            .start_row = 0,
            .end_row = transition_counter,
        });
    }
    update_mode(bottom_mode, $(display_slice){
        .start_row = transition_counter,
        .end_row = LCD_ROWS,
    });
}

static void update_mode(int mode, display_slice_t slice) {
    switch (mode) {
        // TODO: make this a "wipe" mode, which switches back to a "runtime.return_mode"
        case kRuntimeModeWipe:
            display_slice_fill(85, slice);
            return;
        #ifdef MODE_SNAKE
        case kRuntimeModeSnake:
            snake_update(slice);
            return;
        #endif
        #ifdef MODE_TILE_EDITOR
        case kRuntimeModeTileEditor:
            tile_editor_update(slice);
            return;
        #endif
        #ifdef MODE_MAP_EDITOR
        case kRuntimeModeMapEditor:
            map_editor_update(slice);
            return;
        #endif
        default:
            default_update(slice);
            return;
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

void runtime_menu_callback(void *data) {
    runtime_menu_t *menu = data;
    if (menu->pd_menu == NULL) {
        return;
    }
    menu->set_value_from_index(playdate->system->getMenuItemValue(menu->pd_menu));
    runtime_menu_reset_value(menu);
}

int runtime_add_menu(runtime_menu_t *menu) {
    if (runtime_menu_count >= 3 || menu->option_count == 0) {
        return 0;
    }
    runtime_menus[runtime_menu_count++] = menu;
    menu->pd_menu = playdate->system->addOptionsMenuItem(
        menu->title,
        menu->options,
        menu->option_count,
        runtime_menu_callback,
        menu
    );
    runtime_menu_reset_value(menu);
    return 1;
}

static inline void runtime_menu_reset_value(runtime_menu_t *menu) {
    int option_index = menu->get_index_from_value();
    if (option_index < 0 || option_index >= menu->option_count) {
        return;
    }
    playdate->system->setMenuItemValue(menu->pd_menu, option_index);
}
