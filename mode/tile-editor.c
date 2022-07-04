#include "tile-editor.h"

#include "../core/buttons.h"
#include "../core/display.h"
#include "../core/runtime.h"

#include <string.h> // memcpy

tile_editor_t tile_editor = {
    .tile = {
        .index = 0,
        .type = 0,
    },
    .drawing = {
        .color = 1,
        .last_pixel_y = LCD_ROWS / 2,
        .last_pixel_x = LCD_COLUMNS / 2,
    },
};

void tile_editor_update(display_slice slice) {
    // TODO:
}
