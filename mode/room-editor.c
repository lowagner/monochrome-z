#include "room-editor.h"

#include "../library/room.h"

room_editor_t room_editor = {
    .initialization = 15,
    .drawing = {
        .tile_A = 0,
        .tile_B = 1,
        .cursor_x = 0,
        .cursor_y = 0,
    },
};

void room_editor_update(display_slice_t slice) {

}
