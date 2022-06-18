#pragma once

#include "display.h"

struct runtime {
    // readonly `int update(void *)` function pointer:
    int (*const update)(void *);
    // set this if your mode wants to transition to a different mode.
    int next_mode;
    // you probably shouldn't adjust this yourself unless you know what
    // you're doing, but you can read it:
    int transition;
    // TODO: mode_transition for how we should transition
};


extern struct runtime runtime;

// TODO: add enum for different runtime modes

// You should define this in your main.c (or elsewhere).
void default_update(display_slice slice);
