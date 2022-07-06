#pragma once

#include "../core/playdate.h"

#include <stdint.h>

typedef struct room {
    // leave a row at the bottom for HUD:
    uint8_t tiles[LCD_ROWS / 16 - 1][LCD_COLUMNS / 16];
    uint8_t needs_full_redraw;
    // TODO: maybe someday allow x_offset_over_8, but don't want to think about it now.
    int x_offset_over_16;
    // TODO: maybe someday allow any y_offset, don't want to think about it now.
    int y_offset_over_16;
    // TODO: add sprites
}
    room_t;

void room_draw(const room_t *room);
