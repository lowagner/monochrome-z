#include "map-editor.h"

#include "../core/data.h"
#include "../core/runtime.h"
#include "../library/room.h"
#include "../library/sprite.h"

static void map_editor_initialize_next();
static void map_editor_move_sprites();

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
    kMapEditorInitResetSprites,
    kMapEditorInitDrawRoom,
    kMapEditorInitLoadRoom,
    // don't use this for anything:
    kMapEditorInitSentinel,
};

void map_editor_update(display_slice_t slice) {
    if (runtime.transition.counter || runtime.transition.next_mode != kRuntimeModeMapEditor) {
        display_slice_fill_alternating(180, 63, slice);
        map_editor.initialization = kMapEditorInitSentinel - 1;
        return;
    }
    if (map_editor.initialization) {
        map_editor_initialize_next();
        --map_editor.initialization;
    } else {
        const uint8_t *redraw_areas = sprite_pre_move_redraw_areas();
        map_editor_move_sprites();
        room_draw_partial(&map_editor.room, redraw_areas);
        sprite_draw();
    }
}

static void map_editor_move_sprites() {
    int x_axis, y_axis;
    buttons_axis_pushed(&x_axis, &y_axis);
    map_editor.drawing.cursor.x += x_axis;
    if (map_editor.drawing.cursor.x < 0) {
        // TODO: switch rooms
        map_editor.drawing.cursor.x = 0;
    } else if (map_editor.drawing.cursor.x >= ROOM_WIDTH_IN_TILES) {
        // TODO: switch rooms
        map_editor.drawing.cursor.x = ROOM_WIDTH_IN_TILES - 1;
    }
    map_editor.drawing.cursor.y += y_axis;
    if (map_editor.drawing.cursor.y < 0) {
        // TODO: switch rooms
        map_editor.drawing.cursor.y = 0;
    } else if (map_editor.drawing.cursor.y >= ROOM_HEIGHT_IN_TILES) {
        // TODO: switch rooms
        map_editor.drawing.cursor.y = ROOM_HEIGHT_IN_TILES - 1;
    }
    if (x_axis || y_axis) {
        playdate->system->logToConsole(
            "map cursor position (%d, %d)",
            map_editor.drawing.cursor.x,
            map_editor.drawing.cursor.y
        );
    }
    map_editor.drawing.cursor.sprite->display.x = 6 + 16 * map_editor.drawing.cursor.x;
    map_editor.drawing.cursor.sprite->display.y = 6 + 16 * map_editor.drawing.cursor.y;
}

static void map_editor_initialize_next() {
    switch (map_editor.initialization) {
        case kMapEditorInitLoadRoom:
            // TODO
            break;
        case kMapEditorInitDrawRoom:
            map_set_room_tile(&map_editor.room, &map_editor.map);
            room_draw(&map_editor.room);
            break;
        case kMapEditorInitResetSprites:
            sprite_reset();
            break;
        case kMapEditorInitAddCursorSprite: {
            data_u2s_t u2s;
            data_u2s_initialize(&u2s, 0);
            for (int row = 0; row < 4; ++row) {
                if (row == 0 || row == 3) {
                    // skip, flip, flip, skip
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelSkip
                    );
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelFlip
                    );
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelFlip
                    );
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelSkip
                    );
                } else {
                    // rows 1 and 2: flip, flip, flip, flip
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelFlip
                    );
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelFlip
                    );
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelFlip
                    );
                    data_u2s_set_and_increment(
                        &u2s, map_editor.drawing.cursor.data2, kDisplaySpritePixelFlip
                    );
                }
            }
            map_editor.drawing.cursor.sprite = sprite_add($(display_sprite){
                .x = 6,
                .y = 6,
                .z = SPRITE_Z_FRONT,
                .data2 = map_editor.drawing.cursor.data2,
                .width = 4,
                .height = 4,
            });
            break;
        }
    }
}
