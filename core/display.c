#include "display.h"

#include "playdate.h"

#include <string.h> // memset

#define ROW_STRIDE 52

void display_clear(uint8_t bg_color, display_slice slice) {
    if (slice.start_row >= slice.end_row) {
        return;
    }
    uint8_t *display_buffer = playdate->graphics->getFrame();
    memset(
        display_buffer + ROW_STRIDE * slice.start_row,
        ~bg_color,
        ROW_STRIDE * (slice.end_row - slice.start_row)
    );
    playdate->graphics->markUpdatedRows(slice.start_row, slice.end_row - 1);
}
