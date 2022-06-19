#include "snake.h"

struct snake snake = {
    .head_x = 10,
    .head_y = 10,
    .direction = 0,
    .dizziness = 5,
};

void snake_update(display_slice slice) {
    display_clear(0, slice);
}
