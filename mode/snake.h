#ifndef MODE_SNAKE
#define MODE_SNAKE

#include "../core/display.h"
#include "../core/lfsr.h"

#include <stdint.h>

enum snake_direction {
    kSnakeDirectionRight,
    kSnakeDirectionUp,
    kSnakeDirectionLeft,
    kSnakeDirectionDown,
};

typedef struct snake_piece {
    int x;          // 0 to 399 makes sense
    int y;          // 0 to 239 makes sense
    uint32_t lfsr;  // for use with dizziness
    uint8_t direction;  // use snake_direction enum
}
    snake_piece;

typedef struct snake_state {
    snake_piece head;
    snake_piece tail;
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

struct snake {
    int half_size;
    int dizziness;
    int counter;
    int inverse_speed;
    snake_state state;
};

extern struct snake snake;

void snake_initialize();
void snake_reset();
void snake_update(display_slice slice);

#endif
