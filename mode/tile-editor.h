#ifndef MODE_TILE_EDITOR
#define MODE_TILE_EDITOR

#include "../core/display.h"
#include "../core/tile.h"

#include <stdint.h>

typedef struct tile_editor {
    tile_t tile;
    struct {
        uint8_t color;
        uint8_t last_pixel_y;
        int16_t last_pixel_x;
    }
        drawing;
}
    tile_editor_t;

extern struct tile_editor tile_editor;

void tile_editor_update(display_slice slice);

#endif
