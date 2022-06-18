#pragma once

#include "playdate.h"

struct buttons {
    PDButtons current;
    PDButtons pushed;
    PDButtons released;
};

extern struct buttons buttons;

// runtime.c calls this for you in the generic update method.
void buttons_update();
