#ifndef MODE_TILE_EDITOR
#define MODE_TILE_EDITOR

#include "../core/display.h"
#include "../library/tile.h"

#include <stdint.h>

typedef struct tile_editor {
    tile_t tile;
    uint8_t initialization;
    struct {
        uint8_t color;
        /* TODO: add for crank circles?
        uint8_t last_pixel_x;
        uint8_t last_pixel_y;
        */

        uint8_t cursor_x;
        uint8_t cursor_y;
    }
        drawing;
}
    tile_editor_t;

extern tile_editor_t tile_editor;

void tile_editor_update(display_slice_t slice);

#endif
