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
// draws all sprites that have been added, in z-order (based on sprite.display.z):
void sprite_draw();
