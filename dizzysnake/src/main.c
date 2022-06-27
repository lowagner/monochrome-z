#include "modes.h"
#include "../../core/core.h"

DATA4(
    const char *, value, "dizzysnake",
    int, x, LCD_COLUMNS / 2,
    int, y, LCD_ROWS / 2,
    int, last_jump, 0,
    title
);

void initialize() {
    // TODO: better way of adding these tests programatically
    #ifndef NDEBUG
    test__core__display();
    test__core__error();
    test__mode__snake();
    #endif
    font_load("/System/Fonts/Asheville-Sans-14-Bold.pft");
    runtime.transition.next_mode = kRuntimeModeSnake;
    runtime.transition.speed = 10;
}

void default_update(display_slice slice) {
    display_clear(17, slice);
}
