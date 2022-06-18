#pragma once

#include "display.h"

enum runtime_mode {
    kRuntimeModeWipe = -1,
    kRuntimeModeNone = 0,
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

// You should define this in your main.c (or elsewhere).
void default_update(display_slice slice);
// You should also define this in your main.c (or elsewhere).
void initialize();
