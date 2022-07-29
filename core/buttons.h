#pragma once

#include "playdate.h"

struct buttons {
    PDButtons current;
    PDButtons pushed;
    PDButtons released;
    struct {
        // will be kButtonA if released without any dpad pushes while pressed,
        // otherwise will be dpad pushes if A is being held down.
        PDButtons A;
        // will be kButtonB if released without any dpad pushes while pressed,
        // otherwise will be dpad pushes if B is being held down.
        PDButtons B;
    }
        special;
};

extern struct buttons buttons;

// runtime.c calls this for you in the generic update method.
void buttons_update();

// you should call this yourself if you want to handle buttons being held when dpad is pushed.
void buttons_special_update();

void buttons_axis_current(int *x_axis, int *y_axis);
void buttons_axis_pushed(int *x_axis, int *y_axis);
