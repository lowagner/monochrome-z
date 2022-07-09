#pragma once

#include "../core/playdate.h"

#include <stdint.h>

// 25x14 tiles in a room (25*16 = 400, 14*16 = 224, which leaves a row at the bottom for a HUD).
#define ROOM_WIDTH_IN_TILES 25
#define ROOM_HEIGHT_IN_TILES 14

typedef struct room {
    // you can use any arbitrary row-by-row layout, but let us know the row stride and data offset.
    // to draw, we'll use tile.data[tile.offset + tile.row_stride * delta_y + delta_x]
    // from delta_x = 0 to ROOM_WIDTH_IN_TILES - 1 and delta_y = 0 to ROOM_HEIGHT_IN_TILES - 1.
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
