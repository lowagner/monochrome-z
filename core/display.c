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

// pass in a pre-computed (pre-inverted) color here.
// i.e., real_color = display_maybe_invert_color(color).
static inline void display_byte_draw_with_mask(
    uint8_t *buffer_at_byte,
    uint8_t mask,
    uint8_t real_color
) {
    *buffer_at_byte = (*buffer_at_byte & ~mask) | (real_color & mask);
}

static inline uint8_t display_maybe_invert_color(uint8_t color) {
    // TODO: only bit-flip if not color inverted
    return ~color;
}

// pass in a pre-computed inversion here.
// i.e., inversion = display_inversion().
static inline uint8_t display_byte_collision_with_mask(
    uint8_t byte,
    uint8_t mask,
    uint8_t inversion
) {
    return (byte ^ inversion) & mask;
}

static inline uint8_t display_inversion() {
    // TODO: send 0 if inverted
    return 255;
}

void display_clear(uint8_t bg_color, display_slice slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    // TODO: check for end_row past LCD_ROWS
    uint8_t *display_buffer = display();
    memset(
        display_buffer + ROW_STRIDE * slice.start_row,
        display_maybe_invert_color(bg_color),
        ROW_STRIDE * (slice.end_row - slice.start_row)
    );
    playdate->graphics->markUpdatedRows(slice.start_row, slice.end_row - 1);
}

void display_clear_alternating(uint8_t bg_color0, uint8_t bg_color1, display_slice slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    bg_color0 = display_maybe_invert_color(bg_color0);
    bg_color1 = display_maybe_invert_color(bg_color1);
    // TODO: check for end_row past LCD_ROWS
    uint8_t *display_row = display() + ROW_STRIDE * slice.start_row;
    for (int row = slice.start_row; row < slice.end_row; ++row) {
        memset(
            display_row,
            row % 2 ? bg_color1 : bg_color0,
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

#define U8_BITMASK_LEFT_DISPLAY_BITS(x) (-(1 << (8 - (x))))
#define U8_BITMASK_RIGHT_DISPLAY_BITS(x) ((1 << (x)) - 1)

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
    color = display_maybe_invert_color(color);

    if (last_full_byte - first_full_byte > 0) {
        if (start_bits) {
            uint8_t mask = U8_BITMASK_RIGHT_DISPLAY_BITS(start_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                display_byte_draw_with_mask(
                    display_buffer + ROW_STRIDE * row + first_full_byte - 1,
                    mask,
                    color
                );
            }
        }
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            memset(
                display_buffer + ROW_STRIDE * row + first_full_byte,
                color,
                last_full_byte - first_full_byte
            );
        }
        if (last_bits) {
            uint8_t mask = U8_BITMASK_LEFT_DISPLAY_BITS(last_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                display_byte_draw_with_mask(
                    display_buffer + ROW_STRIDE * row + last_full_byte,
                    mask,
                    color
                );
            }
        }
    } else {
        uint8_t mask = (
                U8_BITMASK_RIGHT_DISPLAY_BITS(start_bits)
            &   U8_BITMASK_LEFT_DISPLAY_BITS(last_bits)
        );
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            display_byte_draw_with_mask(
                display_buffer + ROW_STRIDE * row + last_full_byte,
                mask,
                color
            );
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
    const uint8_t inversion = display_inversion();

    if (last_full_byte - first_full_byte > 0) {
        if (start_bits) {
            uint8_t mask = U8_BITMASK_RIGHT_DISPLAY_BITS(start_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                if (display_byte_collision_with_mask(
                    display_buffer[ROW_STRIDE * row + first_full_byte - 1],
                    mask,
                    inversion
                )) {
                    return 1;
                }
            }
        }
        for (int16_t row = box.start_y; row < box.end_y; ++row)
        for (int byte = first_full_byte; byte < last_full_byte; ++byte) {
            if (display_byte_collision_with_mask(
                display_buffer[ROW_STRIDE * row + byte],
                255,
                inversion
            )) {
                return 1;
            }
        }
        if (last_bits) {
            uint8_t mask = U8_BITMASK_LEFT_DISPLAY_BITS(last_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                if (display_byte_collision_with_mask(
                    display_buffer[ROW_STRIDE * row + last_full_byte],
                    mask,
                    inversion
                )) {
                    return 1;
                }
            }
        }
    } else {
        uint8_t mask = (
                U8_BITMASK_RIGHT_DISPLAY_BITS(start_bits)
            &   U8_BITMASK_LEFT_DISPLAY_BITS(last_bits)
        );
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            if (display_byte_collision_with_mask(
                display_buffer + ROW_STRIDE * row + last_full_byte,
                mask,
                inversion
            )) {
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
    //playdate->graphics->markUpdatedRows(y, y);
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

#ifndef NDEBUG
void test__core__display() {
    TEST(
        uint8_t clear_color = 123;
        uint8_t display_color = ~clear_color; // inverted
        display_clear(clear_color, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
        const uint8_t *display_buffer = display();
        for (int y = 0; y < LCD_ROWS; ++y)
        for (int byte = 0; byte < LCD_COLUMNS / 8; ++byte) {
            TEST(
                EXPECT_INT_EQUAL(
                    display_buffer[y * ROW_STRIDE + byte],
                    display_color 
                ),
                "at (y = %d, byte = %d)", y, byte
            );
        },
        "%s: can clear the whole display", AT
    );

    TEST(
        display_clear(0, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
        display_box_draw(255, (display_box){.start_x = 8, .end_x = 16, .start_y = 35, .end_y = 40});
        const uint8_t *display_buffer = display();
        for (int y = 0; y < LCD_ROWS; ++y)
        for (int byte = 0; byte < LCD_COLUMNS / 8; ++byte) {
            TEST(
                EXPECT_INT_EQUAL(
                    display_buffer[y * ROW_STRIDE + byte],
                    255 * (1 - (y >= 35 && y < 40 && byte == 1))
                ),
                "at (y = %d, byte = %d)", y, byte
            );
        },
        "%s: can draw a black box on a white screen", AT
    );

    TEST(
        const uint8_t *display_buffer = display();
        for (int clear_color_int = 0; clear_color_int <= 255; clear_color_int += 85) {
            uint8_t bg_color = clear_color_int;
            uint8_t fg_color = ~bg_color; 
            display_clear(bg_color, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
            for (int start_x = -1; start_x < LCD_COLUMNS; start_x += 131)
            for (int end_x = start_x + 1; end_x <= start_x + 55; end_x += 27)
            for (int start_y = -1; start_y < LCD_ROWS; start_y += 130)
            for (int end_y = start_y + 1; end_y <= start_y + 18; end_y += 19) {
                /*printf("at %d box{x=[%d, %d), y=[%d, %d)}\n",
                        clear_color_int, start_x, end_x, start_y, end_y); */
                display_box_draw(fg_color, (display_box){
                    .start_x = start_x,
                    .end_x = end_x,
                    .start_y = start_y,
                    .end_y = end_y,
                });
                display_box_draw(bg_color, (display_box){
                    .start_x = start_x,
                    .end_x = end_x,
                    .start_y = start_y,
                    .end_y = end_y,
                });
                for (int y = 0; y < LCD_ROWS; ++y)
                for (int byte = 0; byte < LCD_COLUMNS / 8; ++byte) {
                    TEST(
                        EXPECT_INT_EQUAL(
                            display_buffer[y * ROW_STRIDE + byte],
                            // bg color, inverted:
                            (uint8_t)~bg_color
                        ),
                        "at box{x=[%d, %d), y=[%d, %d)} display(y = %d, byte = %d)",
                        start_x, end_x, start_y, end_y, y, byte
                    );
                }
            }
        },
        "%s: can erase a drawn box with a clear box", AT
    );
}
#endif
