#ifndef MODE_TILE_EDITOR
#define MODE_TILE_EDITOR

#include "../core/display.h"
#include "../core/tile.h"

#include <stdint.h>

typedef struct tile_editor {
    tile_t tile;
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

extern struct tile_editor tile_editor;

void tile_editor_update(display_slice_t slice);

#endif
