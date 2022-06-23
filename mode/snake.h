#ifndef MODE_SNAKE
#define MODE_SNAKE

#include "../core/display.h"
#include "../core/lfsr.h"

#include <stdint.h>

enum snake_direction {
    kSnakeDirectionRight = 0,
    kSnakeDirectionUp = 1,
    kSnakeDirectionLeft = 2,
    kSnakeDirectionDown = 3,
};

typedef struct snake_piece {
    // left side of snake piece:
    int x;          // roughly 0 to 399 makes sense
    // top side of snake piece:
    int y;          // roughly 0 to 239 makes sense
    uint32_t lfsr;  // for use with dizziness
    uint8_t direction;  // use snake_direction enum
}
    snake_piece;

typedef struct snake_state {
    int counter;
    // don't modify the direction on head.direction until
    // we actually advance the snake, so we can see if we
    // want to move diagonally:
    int desired_direction;
    snake_piece head;
    snake_piece tail;
    int size_delta;
    struct {
        int x, y;
        display_box box;
        int present;
    }
        apple;
    int game_over;
    uint64_t score;
}
    snake_state;

// TODO: make this into an `extern struct snake next_snake` (for any menu modifications)
// and make the current `struct snake snake` a static internal value to snake.c
struct snake {
    // starting length needs to be at least 2, since head
    // and tail need to be able to keep track of their state
    // by what's drawn on the screen.
    int starting_length;
    int size;
    int dizziness;
    int inverse_speed;
    snake_state state;
};

extern struct snake snake;

void snake_initialize();
void snake_reset();
void snake_update(display_slice slice);

#endif
