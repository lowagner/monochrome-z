#include "display.h"

#include "playdate.h"

#include <string.h> // memset

#define ROW_STRIDE 52

void display_clear(uint8_t bg_color, display_slice slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    // TODO: check for end_row past LCD_ROWS
    uint8_t *display_buffer = playdate->graphics->getFrame();
    memset(
        display_buffer + ROW_STRIDE * slice.start_row,
        ~bg_color,
        ROW_STRIDE * (slice.end_row - slice.start_row)
    );
    playdate->graphics->markUpdatedRows(slice.start_row, slice.end_row - 1);
}

int display_frame_pixel(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return 0;
    }
    const uint8_t *row_buffer = playdate->graphics->getFrame() + ROW_STRIDE * y;
    int byte = x / 8;
    int bit = 7 - (x % 8); // most-significant-bits are left-most.
    return ~(row_buffer[byte] >> bit) & 1;
}
