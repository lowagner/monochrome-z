#ifndef MODE_MAP_EDITOR
#define MODE_MAP_EDITOR

#include "../core/display.h"
#include "../library/map.h"
#include "../library/sprite.h"

#include <stdint.h>

typedef struct map_editor {
    map_t map;
    room_t room;

    uint8_t initialization;

    struct {
        uint8_t tile;
        struct {
            int8_t x;
            int8_t y;
            uint8_t data2[4 * 4 / 4];    // 4x4 with 2-bits per pixel
            sprite_t *sprite;
        }
            cursor;
    }
        drawing;
}
    map_editor_t;

extern map_editor_t map_editor;

void map_editor_update(display_slice_t slice);

#endif
