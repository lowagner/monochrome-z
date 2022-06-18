#include "buttons.h"

struct buttons buttons = {
    .current = 0,
    .pushed = 0,
    .released = 0,
};

// runtime.c calls this for you in the generic update method.
inline void buttons_update() {
    playdate->system->getButtonState(&buttons.current, &buttons.pushed, &buttons.released);
}
