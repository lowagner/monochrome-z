#ifndef MODE_SNAKE
#define MODE_SNAKE

#include "../core/display.h"
#include "../core/lfsr.h"

#include <stdint.h>

typedef struct snake_info {
    // starting length needs to be at least 2, since head
    // and tail need to be able to keep track of their state
    // by what's drawn on the screen.
    int starting_length;
    int size;
    int dizziness;
    int inverse_speed;
}
    snake_info_t;

extern snake_info_t next_snake;

void snake_update(display_slice_t slice);

#endif
