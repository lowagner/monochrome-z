#ifndef MODE_SNAKE
#define MODE_SNAKE

#include "../core/display.h"

struct snake {
    int head_x, head_y;
    int direction;
    int dizziness;
};

extern struct snake snake;

void snake_update(display_slice slice);

#endif
