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
        .cursor_x = 0,
        .cursor_y = 0,
    },
};

void tile_editor_update(display_slice_t slice) {
    // TODO:
}
