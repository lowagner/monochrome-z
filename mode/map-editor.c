#include "map-editor.h"

#include "../core/runtime.h"
#include "../library/room.h"

map_editor_t map_editor = {
    .map = {
        .global_offset = {
            .x = 0,
            .y = 0,
            .z = 0,
        },
        .width = 16,
        .height = 16,
        .tiles = {0},
        .current_room = {
            .x = 0,
            .y = 0,
        },
    },
    .room = {
        .needs_full_redraw = 1,
        .x_offset_over_16 = 0,
        .y_offset_over_16 = 0,
    },
    .initialization = 15,
    .drawing = {
        .tile = 0,
        .cursor_x = 0,
        .cursor_y = 0,
        .B_pushed_without_dpad = 0,
    },
};

void map_editor_update(display_slice_t slice) {
    if (runtime.transition.counter || runtime.transition.next_mode != kRuntimeModeMapEditor) {
        display_slice_fill_alternating(180, 63, slice);
        return;
    }
    map_set_room_tile(&map_editor.room, &map_editor.map);
    room_draw(&map_editor.room);
    map_editor.room.needs_full_redraw = 0;
}
