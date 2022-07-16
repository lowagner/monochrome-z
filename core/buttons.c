#include "buttons.h"

struct buttons buttons = {
    .current = 0,
    .pushed = 0,
    .released = 0,
    .special = {
        .A = 0,
        .B = 0,
    },
};

// runtime.c calls this for you in the generic update method.
void buttons_update() {
    playdate->system->getButtonState(&buttons.current, &buttons.pushed, &buttons.released);
}

static void buttons_special_update_button(
    PDButtons button, PDButtons *special, int *dpad_pushed_while_special_button_active
);

static int dpad_pushed_while_A_button_active = 0;
static int dpad_pushed_while_B_button_active = 0;

void buttons_special_update() {
    buttons_special_update_button(kButtonA, &buttons.special.A, &dpad_pushed_while_A_button_active);
    buttons_special_update_button(kButtonB, &buttons.special.B, &dpad_pushed_while_B_button_active);
}

void buttons_special_update_button(
    PDButtons button, PDButtons *special, int *dpad_pushed_while_special_button_active
) {
    *special = 0;
    if (buttons.pushed & button) {
        *dpad_pushed_while_special_button_active = 0;
    }
    if (buttons.current & button) {
        int dpad_pushed = buttons.pushed & (kButtonRight | kButtonUp | kButtonLeft | kButtonDown);
        if (dpad_pushed) {
            *special = dpad_pushed;
            *dpad_pushed_while_special_button_active = 1;
        }
    }
    if (buttons.released & button) {
        if (!*dpad_pushed_while_special_button_active) {
            *special = button;
        }
    }
}
