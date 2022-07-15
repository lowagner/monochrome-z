#pragma once

#include <stdint.h>

void display_invert();

void display_pixel_draw(int x, int y);
void display_pixel_clear(int x, int y);
// returns 0 for blank (white) and 1 for drawn (black), depending on inversion.
// returns 1 if off screen, since we assume there is a wall around the screen.
int display_pixel_collision(int x, int y);

typedef struct display_slice {
    // TODO: allow int16_t, check bounds when drawing, e.g., NORMALIZE_SLICE
    // start drawing here:
    uint8_t start_row;
    // draw up until, but not including, this:
    uint8_t end_row;
}
    display_slice_t;

void display_slice_fill(uint8_t bg_color, display_slice_t slice);
void display_slice_fill_alternating(uint8_t bg_color0, uint8_t bg_color1, display_slice_t slice);
// TODO: void display_slice_fill_multicolor(int color_count, uint8_t *row_colors, display_slice_t slice);

typedef struct display_box {
    // start drawing here:
    int16_t start_x;
    // draw up until, but not including, this:
    int16_t end_x;
    // start drawing here:
    int16_t start_y;
    // draw up until, but not including, this:
    int16_t end_y;
}
    display_box_t;

void display_box_fill(uint8_t color, display_box_t box);
void display_box_fill_alternating(uint8_t color0, uint8_t color1, display_box_t box);
void display_box_fill_multicolor(int color_count, const uint8_t *row_colors, display_box_t box);
// will return "1" if your box collides with currently drawn pixels (based on inversion).
// going off-screen will also count as a collision, since we pretend that there is a wall there.
int display_box_collision(display_box_t box);
// checks if two display boxes collide with each other (no frame pixels are checked).
int display_box_box_collision(display_box_t box1, display_box_t box2);

// a 16x16 tile which is meant to be drawn as the background;
// it doesn't have any transparency options, since it is the background.
typedef struct display_tile {
    // data should be 32 bytes long and laid out as follows:
    // * row 0:     data1[0], data1[1]
    // * row 1:     data1[2], data1[3]
    //   ...
    // * row 15:    data1[30], data1[31]
    // each byte of data1 corresponds to 8 pixels onscreen, 1 bit per pixel.
    // 0 = clear the pixel, 1 = fill the pixel.
    const uint8_t *data1;

    uint8_t x_over_8;   // the left side of the tile, divided by 8.
                        // (real tile x coordinates will normally be at multiples of 16,
                        //  but the drawing algorithm only cares that they are multiples of 8.)
    uint8_t y;          // the top of the tile (no restrictions).
}
    display_tile_t;

// draws the tile to the display.  note that the tile should be completely on-screen,
// otherwise there will be some undefined behavior.
// i.e., x_over_8 should be between 0 and 48 (x_max = 400 - 16), inclusive,
//   and y should be between 0 and 224 = 240 - 16, inclusive.
void display_tile_draw(display_tile_t tile);

// a sprite of variable size, with transparency options.
// meant to fit into 96 bits = 12 bytes (on 32 bit architecture).
typedef struct display_sprite {
    // data should be laid out as follows:
    // * row 0:   data2[0] ... data2[width / 4 - 1]
    // * row 1:   data2[width / 4] ... data2[2 * width / 4 - 1]
    // etc.
    // each byte of data2 corresponds to 4 pixels onscreen, with 2 bits per pixel.
    // 0 = clear the pixel, 1 = fill the pixel, 2 = skip this pixel, 3 = invert the pixel.
    // if e.g., one_byte = data2[some_location];
    // then, sticking with least significant bits are to the "right":
    // * ((one_byte >> 6) & 2) is the first pixel (from the left)
    // * ((one_byte >> 4) & 2) is the second pixel
    // * ((one_byte >> 2) & 2) is the third pixel
    // * ((one_byte >> 0) & 2) is the fourth, right-most pixel
    const uint8_t *data2;

    int16_t x;
    int16_t y;

    int16_t z; // for ordering front to back.  you can use this for whatever, display doesn't use it.
    uint8_t width; // actual width in pixels (columns), expected to be divisible by 4
    uint8_t height; // actual height in pixels (rows)
}
    display_sprite_t;

// draws the sprite to the display. there will be checks in case the sprite is off-screen,
// so no need to be as careful as with display_tile_draw here for drawing sprites.
void display_sprite_draw(display_sprite_t sprite);
