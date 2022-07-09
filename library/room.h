#pragma once

#include "../core/playdate.h"

#include <stdint.h>

typedef struct room {
    // we have a 25x14 map (25*16 = 400, 14*16 = 224, which leaves a row at the bottom for a HUD).
    // use any arbitrary row-by-row layout, but let us know the row stride and data offset.
    // for this room, we'll consume tile.data[tile.offset + tile.row_stride * delta_y + delta_x]
    // from delta_x = 0 to 24 and delta_y = 0 to 13.
    struct {
        uint8_t *data;
        int row_stride;
        int offset;
    }
        tile;
    uint8_t needs_full_redraw;
    // TODO: maybe someday allow x_offset_over_8, but don't want to think about it now.
    int x_offset_over_16;
    // TODO: maybe someday allow any y_offset, don't want to think about it now.
    int y_offset_over_16;
    // TODO: add sprites
}
    room_t;

void room_draw(const room_t *room);
