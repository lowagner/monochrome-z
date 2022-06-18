#pragma once

#include <stdint.h>

typedef struct display_slice {
    // start drawing here:
    uint8_t start_row;
    // draw up until, but not including, this:
    uint8_t end_row;
}
    display_slice;

void display_clear(uint8_t bg_color, display_slice slice);

