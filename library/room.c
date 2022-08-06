#include "room.h"

#include "../core/data.h"
#include "../core/display.h"
#include "../core/error.h"
#include "../library/tile.h"

void room_draw_partial(const room_t *room, const uint8_t *redraw_areas) {
    for (int check_byte = 0; check_byte < (LCD_ROWS / 16) * (LCD_COLUMNS / 16) / 8; ++check_byte) {
        // optimize not checking every bit since we have byte info:
        if (!redraw_areas[check_byte]) {
            continue;
        }
        data_u1s_t redraw_u1s;
        data_u1s_initialize(&redraw_u1s, 8 * check_byte);
        for (int check_bit = 0; check_bit < 8; ++check_bit) {
            int redraw_area = data_u1s_get_and_increment(&redraw_u1s, redraw_areas);
            if (!redraw_area) {
                continue;
            }
            int onscreen_index = 8 * check_byte + check_bit;
            int onscreen_y_over_16 = onscreen_index / (LCD_COLUMNS / 16);
            int onscreen_x_over_16 = onscreen_index % (LCD_COLUMNS / 16);
            // if we allow more granular tile locations, we might need to redraw
            // multiple tiles here.  as we have it, though, we can get away with
            // pinpointing the exact tile we need to redraw.
            int tile_i = onscreen_x_over_16 - room->x_offset_over_16;
            int tile_j = onscreen_y_over_16 - room->y_offset_over_16;
            display_tile_draw($(display_tile) {
                .data1 = tiles[room->tile.data[
                        room->tile.offset + room->tile.row_stride * tile_j + tile_i
                ]].data1,
                .x_over_8 = onscreen_x_over_16 * 2,
                .y = onscreen_y_over_16 * 16,
            });
        }
    }
}

void room_draw(const room_t *room) {
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
            .y = y_min + 16 *(j - j_min),
        });
    }
}
