#include "modes.h"
#include "../../core/core.h"

#include <stdlib.h> // rand

DATA4(
    const char *, value, "interwoven",
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
}

void default_update(display_slice slice) {
    display_clear(0, slice);
    // TODO: push this logic into a display_text() method
    int title_length = strlen(title.value);
    if (title.y >= slice.start_row && title.y + font.height <= slice.end_row) {
        playdate->graphics->drawText(title.value, title_length, kASCIIEncoding, title.x, title.y);
    }
    int arbitrary_time = playdate->system->getCurrentTimeMilliseconds() / 700;
    if (arbitrary_time != title.last_jump) {
        title.last_jump = arbitrary_time;
        unsigned int random = rand();
        title.x += 4 * (-3 + 2 * (random & 3)); // -3 + 2 * (0, 1, 2, 3) -> (-3, -1, 1, 3)
        title.y += 2 * (-3 + 2 * ((random >> 2) & 3));
    }
    if (buttons.current & kButtonLeft) {
        title.x -= 5;
    }
    if (buttons.current & kButtonRight) {
        title.x += 5;
    }
    if (buttons.current & kButtonUp) {
        title.y -= 5;
    }
    if (buttons.current & kButtonDown) {
        title.y += 5;
    }
    if (title.x < 0 || title.x >= LCD_COLUMNS - title_length * font.width) {
        title.x = (LCD_COLUMNS - title_length * font.width) / 2;
    }
    if (title.y < 0 || title.y >= LCD_ROWS - font.height) {
        title.y = (LCD_ROWS - font.height) / 2;
    }
    if (runtime.transition.counter == 0) {
        runtime.transition.next_mode = kRuntimeModeSnake;
    }
}
