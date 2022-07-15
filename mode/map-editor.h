#ifndef MODE_MAP_EDITOR
#define MODE_MAP_EDITOR

#include "../core/display.h"
#include "../library/map.h"

#include <stdint.h>

typedef struct map_editor {
    map_t map;
    room_t room;

    uint8_t initialization;

    struct {
        uint8_t tile;
        uint8_t cursor_x;
        uint8_t cursor_y;
        uint8_t B_pushed_without_dpad;
    }
        drawing;
}
    map_editor_t;

extern map_editor_t map_editor;

void map_editor_update(display_slice_t slice);

#endif
