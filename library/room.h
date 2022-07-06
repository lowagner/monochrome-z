#pragma once

#include "../core/playdate.h"

#include <stdint.h>

typedef struct room {
    // leave a row at the bottom for HUD:
    uint8_t tiles[LCD_ROWS / 16 - 1][LCD_COLUMNS / 16];
}
    room_t;

void room_draw(const room_t *room);
