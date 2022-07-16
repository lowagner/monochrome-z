#pragma once

#include "../core/display.h"

#include <stdint.h>

// should be less than 256.  smaller is better, at least if you need to sort.
#define MAX_SPRITE_COUNT 64

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

    display_sprite_t display;
}
    sprite_t;

void sprite_reset();
int sprite_count();
sprite_t *sprite_add();
void sprite_remove(sprite_t *sprite);
// calculates where sprites currently are in order to calculate tiles that they might leave
// (and those tiles will need to be redrawn).
void sprite_pre_move_tile_check();
// calculates where sprites currently are in order to calculate tiles to redraw before drawing sprites.
// NOTE: use after calling sprite_pre_move_tile_check() first!
// returns a pointer which you should use with `data_u1s` to iterate over row by row,
// 16x16 tile chunks at a time.  you should redraw any scenery/tiles in those grids,
// then call sprite_draw
const uint8_t *sprite_post_move_tile_check();
// draws all sprites that have been added, in z-order (based on sprite.display.z):
void sprite_draw();
