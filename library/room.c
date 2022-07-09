#include "room.h"

#include "../core/display.h"
#include "../core/error.h"
#include "../library/tile.h"

void room_draw(const room_t *room) {
    if (room->needs_full_redraw) {
        int i_min = 0;
        int i_max = LCD_COLUMNS / 16; // tiles in columns up to but not including i_max.
        uint8_t x_over_8_min = 2 * room->x_offset_over_16;
        if (room->x_offset_over_16 < 0) {
            i_min = -room->x_offset_over_16;
            i_max += room->x_offset_over_16;
            x_over_8_min = 0;
        }
        int j_min = 0;
        int j_max = LCD_ROWS / 16; // tiles in rows up to but not including j_max.
        uint8_t y_min = 16 * room->y_offset_over_16;
        if (room->y_offset_over_16 < 0) {
            j_min = -room->y_offset_over_16;
            j_max += room->y_offset_over_16;
            y_min = 0;
        }
        for (int j = j_min; j < j_max; ++j)
        for (int i = i_min; i < i_max; ++i) {
            display_tile_draw($(display_tile) {
                .data1 = tiles[room->tile.data[
                        room->tile.offset + room->tile.row_stride * j + i
                ]].data1,
                .x_over_8 = x_over_8_min + 2 * (i - i_min),
                .y = y_min + 2 *(j - j_min),
            });
        }
    }
    // TODO:
}
