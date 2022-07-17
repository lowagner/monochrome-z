#include "map-editor.h"

#include "../core/data.h"
#include "../core/runtime.h"
#include "../library/room.h"
#include "../library/sprite.h"

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
        .x_offset_over_16 = 0,
        .y_offset_over_16 = 0,
    },
    .initialization = 16,
    .drawing = {
        .tile = 0,
        .cursor = {
            .x = 0,
            .y = 0,
            .data2 = {0},
        },
    },
};

// do reverse order so we can count down to 0:
enum map_editor_init_t {
    kMapEditorInitNone = 0,
    kMapEditorInitAddCursorSprite,
    kMapEditorInitDrawRoom,
    kMapEditorInitLoadRoom,
};

void map_editor_update(display_slice_t slice) {
    if (runtime.transition.counter || runtime.transition.next_mode != kRuntimeModeMapEditor) {
        display_slice_fill_alternating(180, 63, slice);
        map_editor.initialization = kMapEditorInitLoadRoom;
        return;
    }
    if (map_editor.initialization) {
        switch (map_editor.initialization) {
            case kMapEditorInitLoadRoom:
                // TODO
                break;
            case kMapEditorInitDrawRoom:
                map_set_room_tile(&map_editor.room, &map_editor.map);
                room_draw(&map_editor.room);
                break;
            case kMapEditorInitAddCursorSprite: {
                data_u2s_t u2s;
                data_u2s_initialize(&u2s, 0);
                for (int row = 0; row < 4; ++row) {
                    if (row == 0 || row == 3) {
                        // skip, on, on, skip
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelSkip
                        );
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelOn
                        );
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelOn
                        );
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelSkip
                        );
                    } else {
                        // rows 1 and 2: on, on, on, on
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelOn
                        );
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelOn
                        );
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelOn
                        );
                        data_u2s_set_and_increment(
                            &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelOn
                        );
                    }
                }
                break;
            }
        }
        --map_editor.initialization;
    } else {
        sprite_pre_move_area_check();
        const uint8_t *redraw_areas = sprite_post_move_area_check();
        room_draw_partial(&map_editor.room, redraw_areas);
        sprite_draw();
    }
}
