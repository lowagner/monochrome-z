#include "pd_api.h"

#include <stdlib.h> // rand

#define DATA3(type1, name1, init1, type2, name2, init2, type3, name3, init3, struct_name) \
    struct struct_name { \
        type1 name1; \
        type2 name2; \
        type3 name3; \
    } \
    struct_name = { \
        .name1 = init1, \
        .name2 = init2, \
        .name3 = init3, \
    }

#define DATA4(type1, name1, init1, type2, name2, init2, type3, name3, init3, type4, name4, init4, struct_name) \
    struct struct_name { \
        type1 name1; \
        type2 name2; \
        type3 name3; \
        type4 name4; \
    } \
    struct_name = { \
        .name1 = init1, \
        .name2 = init2, \
        .name3 = init3, \
        .name4 = init4, \
    }

DATA4(
    const char *, path, "/System/Fonts/Asheville-Sans-14-Bold.pft",
    LCDFont *, pd, NULL,
    int, height, 16,
    int, width, 7,
    font
);

DATA4(
    const char *, value, "interwoven",
    int, x, LCD_COLUMNS / 2,
    int, y, LCD_ROWS / 2,
    int, last_jump, 0,
    title
);

DATA3(
    PDButtons, current, 0,
    PDButtons, pushed, 0,
    PDButtons, released, 0,
    buttons
);

int update(void *user_data);

int initialize(PlaydateAPI *pd) {
    const char* error;
    font.pd = pd->graphics->loadFont(font.path, &error);
    if (font.pd == NULL) {
        pd->system->error("%s:%i couldn't load font %s: %s", __FILE__, __LINE__, font.path, error);
    }
    pd->graphics->setFont(font.pd);

    // Set an update callback to indicate the game is pure C (no Lua):
    pd->system->setUpdateCallback(update, pd);
}

int update(void *user_data) {
    PlaydateAPI *pd = user_data;
    pd->graphics->clear(kColorWhite);
    int title_length = strlen(title.value);
    pd->graphics->drawText(title.value, title_length, kASCIIEncoding, title.x, title.y);
    int arbitrary_time = pd->system->getCurrentTimeMilliseconds() / 700;
    if (arbitrary_time != title.last_jump) {
        title.last_jump = arbitrary_time;
        unsigned int random = rand();
        title.x += 4 * (-3 + 2 * (random & 3)); // -3 + 2 * (0, 1, 2, 3) -> (-3, -1, 1, 3)
        title.y += 2 * (-3 + 2 * ((random >> 2) & 3));
    }
    pd->system->getButtonState(&buttons.current, &buttons.pushed, &buttons.released);
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
    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t argument) {
    switch (event) {
        case kEventInit:
            initialize(pd);
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
