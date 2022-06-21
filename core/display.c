#include "display.h"

#define ROW_STRIDE 52

#include "playdate.h"

#ifndef NDEBUG
#include "error.h"
uint8_t test_display_buffer[ROW_STRIDE * LCD_ROWS];
#endif

#include <string.h> // memset

static inline uint8_t *display() {
    #ifndef NDEBUG
    if (error_test_only) {
        return test_display_buffer;
    }
    #endif
    return playdate->graphics->getFrame();
}

void display_clear(uint8_t bg_color, display_slice slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    // TODO: check for end_row past LCD_ROWS
    uint8_t *display_buffer = display();
    memset(
        display_buffer + ROW_STRIDE * slice.start_row,
        ~bg_color,
        ROW_STRIDE * (slice.end_row - slice.start_row)
    );
    playdate->graphics->markUpdatedRows(slice.start_row, slice.end_row - 1);
}

void display_clear_alternating(uint8_t bg_color0, uint8_t bg_color1, display_slice slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    // TODO: check for end_row past LCD_ROWS
    uint8_t *display_row = display() + ROW_STRIDE * slice.start_row;
    for (int row = slice.start_row; row < slice.end_row; ++row) {
        memset(
            display_row,
            ~( row % 2 ? bg_color1 : bg_color0 ),
            // note that ROW_STRIDE = 52 to do 32 bit alignments,
            // but only LCD_COLUMNS / 8 = 50 bytes are visible per row:
            LCD_COLUMNS / 8
        );
        display_row += ROW_STRIDE;
    }
    playdate->graphics->markUpdatedRows(slice.start_row, slice.end_row - 1);
}

#define BOX_EMPTY(b) \
    (b.start_x >= b.end_x || b.start_y >= b.end_y)

#define BOX_OFF_SCREEN(b) \
    (b.end_x <= 0 || b.start_x >= LCD_COLUMNS || b.end_y <= 0 || b.start_y >= LCD_ROWS)

#define NORMALIZE_BOX(b) { \
    if (b.start_x < 0) b.start_x = 0; \
    if (b.end_x > LCD_COLUMNS) b.end_x = LCD_COLUMNS; \
    if (b.start_y < 0) b.start_y = 0; \
    if (b.start_y >= LCD_ROWS) b.end_y = LCD_ROWS; \
}

#define U8_BITMASK_FIRST_BITS(x) (-(1 << (7 - (x) + 1)))
#define U8_BITMASK_LAST_BITS(x) ((1 << (x)) - 1)

void display_box_draw(uint8_t color, display_box box) {
    if (BOX_OFF_SCREEN(box) || BOX_EMPTY(box)) {
        return;
    }
    NORMALIZE_BOX(box);

    int first_full_byte = box.start_x / 8; // will become ceiling of box.start_x / 8
    int start_bits = box.start_x % 8;
    if (start_bits) ++first_full_byte;  // now first_full_byte is the ceiling.

    int last_full_byte = box.end_x / 8; // floor of box.end_x / 8
    int last_bits = box.end_x % 8;

    uint8_t *const display_buffer = display();

    if (start_bits) {
        uint8_t first_bits_color = U8_BITMASK_LAST_BITS(start_bits) & color;
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            display_buffer[ROW_STRIDE * row + first_full_byte - 1] &= ~first_bits_color;
        }
    }
    if (last_full_byte - first_full_byte > 0)
    for (int16_t row = box.start_y; row < box.end_y; ++row) {
        memset(
            display_buffer + ROW_STRIDE * row + first_full_byte,
            ~color,
            last_full_byte - first_full_byte
        );
    }
    if (last_bits) {
        uint8_t last_bits_color = U8_BITMASK_FIRST_BITS(last_bits) & color;
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            display_buffer[ROW_STRIDE * row + last_full_byte] &= ~last_bits_color;
        }
    }
    playdate->graphics->markUpdatedRows(box.start_y, box.end_y - 1);
}

int display_box_collision(display_box box) {
    if (BOX_EMPTY(box)) {
        return 0;
    }
    if (BOX_OFF_SCREEN(box)) {
        // assume being off screen means you hit a wall:
        return 1;
    }
    NORMALIZE_BOX(box);

    int first_full_byte = box.start_x / 8; // will become ceiling of box.start_x / 8
    int start_bits = box.start_x % 8;
    if (start_bits) ++first_full_byte;  // now first_full_byte is the ceiling.

    int last_full_byte = box.end_x / 8; // floor of box.end_x / 8
    int last_bits = box.end_x % 8;

    uint8_t *const display_buffer = display();

    if (start_bits) {
        uint8_t bitmask = U8_BITMASK_LAST_BITS(start_bits);
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            if (~display_buffer[ROW_STRIDE * row + first_full_byte - 1] & bitmask) {
                return 1;
            }
        }
    }
    if (last_full_byte - first_full_byte > 0)
    for (int16_t row = box.start_y; row < box.end_y; ++row)
    for (int16_t byte = first_full_byte; byte < last_full_byte; ++byte) {
        if (~display_buffer[ROW_STRIDE * row + byte]) {
            return 1;
        }
    }
    if (last_bits) {
        uint8_t bitmask = U8_BITMASK_FIRST_BITS(last_bits);
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            if (~display_buffer[ROW_STRIDE * row + last_full_byte] & bitmask) {
                return 1;
            }
        }
    }
    return 0;
}

int display_box_box_collision(display_box box1, display_box box2) {
    // TODO: might give weird results if box1 or box2 are empty.
    // maybe just push that onto callers.
    return (
            box1.start_x < box2.end_x
        &&  box2.start_x < box1.end_x
        &&  box1.start_y < box2.end_y
        &&  box2.start_y < box1.end_y
    );
}

void display_pixel_draw(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return;
    }
    uint8_t *row_buffer = display() + ROW_STRIDE * y;
    int byte = x / 8;
    int bit = 7 - (x % 8); // most-significant-bits are left-most.
    // TODO: for inversion, we can't use & here.
    row_buffer[byte] &= ~(1 << bit);
    playdate->graphics->markUpdatedRows(y, y);
}

void display_pixel_clear(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return;
    }
    uint8_t *row_buffer = display() + ROW_STRIDE * y;
    int byte = x / 8;
    int bit = 7 - (x % 8); // most-significant-bits are left-most.
    // TODO: for inversion, we can't use | here.
    row_buffer[byte] |= 1 << bit;
    playdate->graphics->markUpdatedRows(y, y);
}

int display_pixel_collision(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return 1;
    }
    const uint8_t *row_buffer = display() + ROW_STRIDE * y;
    int byte = x / 8;
    int bit = 7 - (x % 8); // most-significant-bits are left-most.
    return ~(row_buffer[byte] >> bit) & 1;
}

void test__core__display() {
    TEST(
        uint8_t clear_color = 123;
        uint8_t display_color = ~clear_color; // inverted
        display_clear(clear_color, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
        const uint8_t *display_buffer = display();
        for (int y = 0; y < LCD_ROWS; ++y)
        for (int byte = 0; byte < LCD_COLUMNS / 8; ++byte) {
            TEST_WITH_CONTEXT(
                EXPECT_INT_EQUAL(
                    display_buffer[y * ROW_STRIDE + byte],
                    display_color 
                ),
                "at (y = %d, byte = %d)", y, byte
            );
        }
    );
}
