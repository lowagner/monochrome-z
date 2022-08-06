#pragma once

#include "../core/display.h"

#include <stdint.h>

// should be less than 256.  smaller is better, at least if you need to sort.
#define MAX_SPRITE_COUNT 64

#define SPRITE_Z_BACK INT16_MAX
#define SPRITE_Z_FRONT INT16_MIN

typedef struct sprite {
    int16_t x;
    int16_t y;
    int16_t z;

    int16_t v_x;
    int16_t v_y;
    int16_t v_z;

    uint8_t health;
    uint8_t magic;
    uint8_t stamina;
    uint8_t state;

    // large positive numbers for display.z are further away and will be drawn *first*.
    // negative numbers for display.z are closer and will be drawn *last*.
    display_sprite_t display;
}
    sprite_t;

void sprite_reset();
int sprite_count();
sprite_t *sprite_add(display_sprite_t display_data);
void sprite_remove(sprite_t *sprite);
// calculates where sprites currently are in order to calculate areas that they might leave
// (and those areas will need to be redrawn).
// returns a pointer which you should use with `data_u1s` to iterate over row by row,
// 16x16 area chunks at a time.  you should redraw any scenery/areas in those grids,
// then call sprite_draw
const uint8_t *sprite_pre_move_redraw_areas();
// draws all sprites that have been added, in z-order (based on sprite.display.z):
// please call sprite_pre_move_redraw_areas first, then move sprites, then call this:
void sprite_draw();
