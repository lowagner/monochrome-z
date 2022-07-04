#pragma once

#include "display.h"
#include "error.h"

// need to ensure we grab the modes that we want, so that their MODE_X macros are defined
// for later in our update(void *) function.
#include STRINGIFY(PD_PROJECT_MODES)

enum runtime_mode {
    kRuntimeModeWipe = -1,
    kRuntimeModeNone = 0,
    // TODO: ModeMenu
    #ifdef MODE_SNAKE
    kRuntimeModeSnake,
    #endif
    #ifdef MODE_TILE_EDITOR
    kRuntimeModeTileEditor,
    #endif
};

struct runtime {
    // readonly `int update(void *)` function pointer:
    int (*const update)(void *);
    struct {
        // indicates the current mode, unless you want to change it.
        // set this if your mode wants to transition to a different mode.
        // you should probably check to see if we're not already in a transition.
        int next_mode;
        // you probably shouldn't adjust this yourself unless you know what
        // you're doing, but you can read it.  it will be nonzero if in a transition.
        unsigned int counter;
        // how fast to transition.  0 is very slow, >239 is instant.
        uint8_t speed;
        // whether to transition up (1) or down (0).
        uint8_t up;
    }
        transition;
};


extern struct runtime runtime;

typedef struct runtime_menu {
    // will be set by calling runtime_add_menu.
    // you can stop allowing callbacks to this method if you set this to NULL.
    PDMenuItem *pd_menu;
    const char *title;
    const char **options;
    int option_count;
    // callback to set a value, assuming that options[index] has been chosen.
    // called any time the menu is closed (and a different index has been chosen).
    void (*set_value_from_index)(int index);
    // returns the option index that the current value should correspond to.
    // called once when the menu is created.  will clamp to [0, option_count - 1].
    int (*get_index_from_value)(void);
}
    runtime_menu;

// returns 1 if successful, otherwise 0.
int runtime_add_menu(runtime_menu *menu);

// You should define this in your main.c (or elsewhere).
void default_update(display_slice slice);
// You should also define this in your main.c (or elsewhere).
void initialize();
