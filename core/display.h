#pragma once

#include <stdint.h>

typedef struct display_slice {
    // start drawing here:
    uint8_t start_row;
    // draw up until, but not including, this:
    uint8_t end_row;
}
    display_slice;

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
    display_box;

void display_invert();
void display_clear(uint8_t bg_color, display_slice slice);
void display_clear_alternating(uint8_t bg_color0, uint8_t bg_color1, display_slice slice);

void display_box_draw(uint8_t color, display_box box);
void display_box_draw_alternating(uint8_t color0, uint8_t color1, display_box box);
void display_box_draw_multicolor(int color_count, uint8_t *row_colors, display_box box);
// TODO: display_box_draw_alternating
// will return "1" if your box collides with currently drawn pixels (based on inversion).
// going off-screen will also count as a collision, since we pretend that there is a wall there.
int display_box_collision(display_box box);
// checks if two display boxes collide with each other (no frame pixels are checked).
int display_box_box_collision(display_box box1, display_box box2);

void display_pixel_draw(int x, int y);
void display_pixel_clear(int x, int y);
// returns 0 for blank (white) and 1 for drawn (black), depending on inversion.
// returns 1 if off screen, since we assume there is a wall around the screen.
int display_pixel_collision(int x, int y);
