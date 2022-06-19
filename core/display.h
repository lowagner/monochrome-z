#pragma once

#include <stdint.h>

// TODO: add an display struct with inversion
// essentially use ~ everywhere when not inverted and nothing when inverted

typedef struct display_slice {
    // start drawing here:
    uint8_t start_row;
    // draw up until, but not including, this:
    uint8_t end_row;
}
    display_slice;

void display_clear(uint8_t bg_color, display_slice slice);
// returns 0 for blank (white) and 1 for drawn (black), depending on inversion.
int display_frame_pixel(int x, int y);
