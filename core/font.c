#include "font.h"

#include "playdate.h"

struct font font = {
    .path = "",
    .height = 0,
    .width = 0,
};

void font_load(const char *path) {
    const char* error;
    LCDFont *lcd_font = playdate->graphics->loadFont(path, &error);
    if (lcd_font == NULL) {
        playdate->system->error("%s:%i couldn't load font %s: %s", __FILE__, __LINE__, path, error);
    }
    playdate->graphics->setFont(lcd_font);
    font.path = path;
    // TODO: better, or loading our own font bitmaps...
    font.height = 16;
    font.width = 7;
}

// TODO: font-saving
