#ifndef MODE_ROOM_EDITOR
#define MODE_ROOM_EDITOR

#include "../core/display.h"

#include <stdint.h>

typedef struct room_editor {
    uint8_t initialization;
    struct {
        uint8_t tile_A;
        uint8_t tile_B;
        uint8_t cursor_x;
        uint8_t cursor_y;
    }
        drawing;
}
    room_editor_t;

extern room_editor_t room_editor;

void room_editor_update(display_slice_t slice);

#endif
