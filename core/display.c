#include "display.h"

#define ROW_STRIDE 52

#include "data.h"
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

// use 0 for inverted (black bg with white fg), 255 for not inverted (white bg with black fg).
static uint8_t display_inversion = 255;
// TODO: double check these, i'm not 100% sure that i got this right...
#define PIXEL_ON (!display_inversion)
#define PIXEL_OFF (!!display_inversion)

void display_invert() {
    uint8_t *display_buffer = display();
    for (int byte = 0; byte < LCD_ROWS * ROW_STRIDE; ++byte) {
        *display_buffer = ~*display_buffer;
        ++display_buffer;
    }
    display_inversion = 255 - display_inversion;
    playdate->graphics->markUpdatedRows(0, LCD_ROWS - 1);
}

void display_pixel_draw(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return;
    }
    data_u1s_t u1s;
    data_u1s_initialize(&u1s, 8 * ROW_STRIDE * y + x);
    data_u1s_set(&u1s, display(), !display_inversion);
    playdate->graphics->markUpdatedRows(y, y);
}

void display_pixel_clear(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return;
    }
    data_u1s_t u1s;
    data_u1s_initialize(&u1s, 8 * ROW_STRIDE * y + x);
    data_u1s_set(&u1s, display(), display_inversion);
    playdate->graphics->markUpdatedRows(y, y);
}

int display_pixel_collision(int x, int y) {
    if (x < 0 || x >= LCD_COLUMNS || y < 0 || y >= LCD_ROWS) {
        return 1;
    }
    // TODO: use data_u1s for this
    const uint8_t *row_buffer = display() + ROW_STRIDE * y;
    int byte = x / 8;
    int bit = 7 - (x % 8); // most-significant-bits are left-most.
    return ((row_buffer[byte] ^ display_inversion) >> bit) & 1;
}

static inline void display_byte_draw_with_mask(
    uint8_t *buffer_at_byte,
    uint8_t mask,
    uint8_t color
) {
    *buffer_at_byte = (*buffer_at_byte & ~mask) | ((color ^ display_inversion) & mask);
}

static inline uint8_t display_byte_collision_with_mask(
    uint8_t byte,
    uint8_t mask
) {
    return (byte ^ display_inversion) & mask;
}

void display_slice_fill(uint8_t bg_color, display_slice_t slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    // TODO: check for end_row past LCD_ROWS
    uint8_t *display_buffer = display();
    memset(
        display_buffer + ROW_STRIDE * slice.start_row,
        bg_color ^ display_inversion,
        ROW_STRIDE * (slice.end_row - slice.start_row)
    );
    playdate->graphics->markUpdatedRows(slice.start_row, slice.end_row - 1);
}

void display_slice_fill_alternating(uint8_t bg_color0, uint8_t bg_color1, display_slice_t slice) {
    if (slice.start_row >= slice.end_row || slice.start_row >= LCD_ROWS) {
        return;
    }
    bg_color0 = bg_color0 ^ display_inversion;
    bg_color1 = bg_color1 ^ display_inversion;
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

#define BOX_FULLY_OFF_SCREEN(b) \
    (b.end_x <= 0 || b.start_x >= LCD_COLUMNS || b.end_y <= 0 || b.start_y >= LCD_ROWS)

#define BOX_ANY_OFF_SCREEN(b) \
    (b.start_x < 0 || b.end_x > LCD_COLUMNS || b.start_y < 0 || b.end_y > LCD_ROWS)

#define NORMALIZE_BOX(b) { \
    if (b.start_x < 0) b.start_x = 0; \
    if (b.end_x > LCD_COLUMNS) b.end_x = LCD_COLUMNS; \
    if (b.start_y < 0) b.start_y = 0; \
    if (b.start_y >= LCD_ROWS) b.end_y = LCD_ROWS; \
}

#define U8_BITMASK_LEFT_DISPLAY_BITS(x) (-(1 << (8 - (x))))
#define U8_BITMASK_RIGHT_DISPLAY_BITS(x) ((1 << (8 - (x))) - 1)

void display_box_fill(uint8_t color, display_box_t box) {
    display_box_fill_multicolor(1, &color, box);
}

void display_box_fill_alternating(uint8_t color0, uint8_t color1, display_box_t box) {
    uint8_t row_colors[2] = {color0, color1};
    display_box_fill_multicolor(2, row_colors, box);
}

void display_box_fill_multicolor(int color_count, uint8_t *row_colors, display_box_t box) {
    if (BOX_FULLY_OFF_SCREEN(box) || BOX_EMPTY(box)) {
        return;
    }
    NORMALIZE_BOX(box);

    int first_full_byte = box.start_x / 8; // will become ceiling of box.start_x / 8
    int start_bits = box.start_x % 8;
    if (start_bits) ++first_full_byte;  // now first_full_byte is the ceiling.

    int last_full_byte = box.end_x / 8;
    int last_bits = box.end_x % 8;

    uint8_t *const display_buffer = display();

    if (last_full_byte >= first_full_byte) {
        if (start_bits) {
            uint8_t mask = U8_BITMASK_RIGHT_DISPLAY_BITS(start_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                display_byte_draw_with_mask(
                    display_buffer + ROW_STRIDE * row + first_full_byte - 1,
                    mask,
                    row_colors[row % color_count]
                );
            }
        }
        if (last_full_byte > first_full_byte)
        for (int16_t row = box.start_y; row < box.end_y; ++row) {
            memset(
                display_buffer + ROW_STRIDE * row + first_full_byte,
                row_colors[row % color_count] ^ display_inversion,
                last_full_byte - first_full_byte
            );
        }
        if (last_bits) {
            uint8_t mask = U8_BITMASK_LEFT_DISPLAY_BITS(last_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                display_byte_draw_with_mask(
                    display_buffer + ROW_STRIDE * row + last_full_byte,
                    mask,
                    row_colors[row % color_count]
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
                row_colors[row % color_count]
            );
        }
    }
    playdate->graphics->markUpdatedRows(box.start_y, box.end_y - 1);
}

int display_box_collision(display_box_t box) {
    if (BOX_EMPTY(box)) {
        return 0;
    }
    if (BOX_ANY_OFF_SCREEN(box)) {
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

    if (last_full_byte >= first_full_byte) {
        if (start_bits) {
            uint8_t mask = U8_BITMASK_RIGHT_DISPLAY_BITS(start_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                if (display_byte_collision_with_mask(
                    display_buffer[ROW_STRIDE * row + first_full_byte - 1],
                    mask
                )) {
                    return 1;
                }
            }
        }
        if (last_full_byte > first_full_byte)
        for (int16_t row = box.start_y; row < box.end_y; ++row)
        for (int byte = first_full_byte; byte < last_full_byte; ++byte) {
            if (display_byte_collision_with_mask(
                display_buffer[ROW_STRIDE * row + byte],
                255
            )) {
                return 1;
            }
        }
        if (last_bits) {
            uint8_t mask = U8_BITMASK_LEFT_DISPLAY_BITS(last_bits);
            for (int16_t row = box.start_y; row < box.end_y; ++row) {
                if (display_byte_collision_with_mask(
                    display_buffer[ROW_STRIDE * row + last_full_byte],
                    mask
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
                display_buffer[ROW_STRIDE * row + last_full_byte],
                mask
            )) {
                return 1;
            }
        }
    }
    return 0;
}

int display_box_box_collision(display_box_t box1, display_box_t box2) {
    // TODO: might give weird results if box1 or box2 are empty.
    // maybe just push that onto callers.
    return (
            box1.start_x < box2.end_x
        &&  box2.start_x < box1.end_x
        &&  box1.start_y < box2.end_y
        &&  box2.start_y < box1.end_y
    );
}

void display_tile_draw(display_tile tile) {
    uint8_t *display_buffer = display();
    ASSERT(tile.x_over_8 <= (400 - 16) / 8);
    ASSERT(tile.y <= 240 - 16);
    const int max_row = 16 + tile.y;
    const uint8_t *tile_data = tile.data1;
    for (int row = tile.y; row < max_row; ++row) {
        // unrolling the inner loop over x, since it's just two bytes:
        display_buffer[row * ROW_STRIDE + tile.x_over_8 + 0] = (*tile_data++) ^ display_inversion;
        display_buffer[row * ROW_STRIDE + tile.x_over_8 + 1] = (*tile_data++) ^ display_inversion;
    }
}

void display_sprite_draw(display_sprite sprite) {
    uint8_t *const display_buffer = display();
    if (sprite.x >= LCD_COLUMNS || sprite.y >= LCD_ROWS) {
        return;
    }
    int16_t start_pixel_x = sprite.x > 0 ? sprite.x : 0;
    int16_t end_pixel_x = sprite.x + sprite.width;
    if (end_pixel_x < 0) {
        return;
    } else if (end_pixel_x >= LCD_COLUMNS) {
        end_pixel_x = LCD_COLUMNS;
    }
    int16_t start_pixel_y = sprite.y > 0 ? sprite.y : 0;
    int16_t end_pixel_y = sprite.y + sprite.height;
    if (end_pixel_y < 0) {
        return;
    } else if (end_pixel_y >= LCD_ROWS) {
        end_pixel_y = LCD_ROWS;
    }
    data_u1s_t display_iterator;
    data_u2s_t sprite_iterator;
    for (int16_t pixel_y = start_pixel_y; pixel_y < end_pixel_y; ++pixel_y) {
        data_u1s_initialize(&display_iterator, pixel_y * ROW_STRIDE + start_pixel_x);
        data_u2s_initialize(&sprite_iterator, 
                (pixel_y - sprite.y) * sprite.width / 4
            +   (start_pixel_x - sprite.x)
        );
        for (int16_t pixel_x = start_pixel_x; pixel_x < end_pixel_x; ++pixel_x) {
            int u2 = data_u2s_get(&sprite_iterator, sprite.data2);
            data_u2s_increment(&sprite_iterator);

            switch (u2) {
                case 0:
                    data_u1s_set(&display_iterator, display_buffer, PIXEL_OFF);
                    break;
                case 1:
                    data_u1s_set(&display_iterator, display_buffer, PIXEL_ON);
                    break;
                case 3:
                    data_u1s_flip(&display_iterator, display_buffer);
                    break;
                default:
                    // skip doing anything to display, including case 2.
                    break;
            }
            data_u1s_increment(&display_iterator);
        }
    }
    playdate->graphics->markUpdatedRows(start_pixel_y, end_pixel_y - 1);
}

#ifndef NDEBUG
void test__core__display() {
    uint8_t test_tile_data[16 * 2] = {0};
    TEST(
        uint8_t clear_color = 123;
        uint8_t display_color = ~clear_color; // inverted
        display_slice_fill(clear_color, $(display_slice){.start_row = 0, .end_row = LCD_ROWS});
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
        display_slice_fill(0, $(display_slice){.start_row = 0, .end_row = LCD_ROWS});
        display_box_fill(255, $(display_box){.start_x = 8, .end_x = 16, .start_y = 35, .end_y = 40});
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
        for (int offset = 0; offset < 10; offset += 1) {
            display_slice_fill(0, $(display_slice){.start_row = 0, .end_row = LCD_ROWS});
            display_box_fill(255, $(display_box){
                .start_x = 8 + offset,
                .end_x = 18 + offset,
                .start_y = 35 + offset,
                .end_y = 45 + offset,
            });
            for (int y = 30; y < 60; ++y) 
            for (int x = 0; x < LCD_COLUMNS; ++x) {
                TEST(
                    EXPECT_INT_EQUAL_LOGGED(display_pixel_collision(x, y), (
                            y >= 35 + offset 
                        &&  y < 45 + offset
                        &&  x >= 8 + offset
                        &&  x < 18 + offset
                    )),
                    "%s: at offset %d (x, y) = (%d, %d), display_byte = %d",
                    AT, offset, x, y, test_display_buffer[y * ROW_STRIDE + x / 8] 
                );
            }
        },
        "%s: can draw a 10x10 black box across a boundary", AT
    );

    TEST(
        const uint8_t *display_buffer = display();
        for (int clear_color_int = 0; clear_color_int <= 255; clear_color_int += 85) {
            uint8_t bg_color = clear_color_int;
            uint8_t fg_color = ~bg_color; 
            display_slice_fill(bg_color, $(display_slice){.start_row = 0, .end_row = LCD_ROWS});
            for (int start_x = -1; start_x < LCD_COLUMNS; start_x += 131)
            for (int end_x = start_x + 1; end_x <= start_x + 55; end_x += 27)
            for (int start_y = -1; start_y < LCD_ROWS; start_y += 130)
            for (int end_y = start_y + 1; end_y <= start_y + 18; end_y += 19) {
                /*printf("at %d box{x=[%d, %d), y=[%d, %d)}\n",
                        clear_color_int, start_x, end_x, start_y, end_y); */
                display_box_fill(fg_color, $(display_box){
                    .start_x = start_x,
                    .end_x = end_x,
                    .start_y = start_y,
                    .end_y = end_y,
                });
                display_box_fill(bg_color, $(display_box){
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

    TEST(
        const uint8_t *display_buffer = display();
        display_slice_fill(0, $(display_slice){.start_row = 0, .end_row = LCD_ROWS});
        for (int i = 0; i < 32; ++i) {
            test_tile_data[i] = 8 * (i - 3);
        }
        display_tile_draw((display_tile){
            .data1 = test_tile_data,
            .x_over_8 = 5,
            .y = 7,
        });
        const uint8_t *data = test_tile_data;
        for (int y = 7 - 1; y < 7 + 16 + 1; ++y)
        for (int byte = 5 - 1; byte < 5 + 2 + 1; ++byte) {
            TEST(
                EXPECT_INT_EQUAL(
                    display_buffer[y * ROW_STRIDE + byte],
                    (uint8_t)~(
                            (y >= 7 && y < 7+16 && byte >= 5 && byte < 7)
                        ?   *data++ // tile data here
                        :   0       // bg color here
                    )
                ),
                "at display(y = %d, byte = %d)",
                y, byte
            );
        },
        "%s: can draw a tile", AT
    );
}
#endif
