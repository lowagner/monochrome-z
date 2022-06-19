#include "snake.h"

#include <string.h> // memcpy

static void snake_advance_head();
static void snake_advance_tail();
static void snake_advance_piece(snake_piece *piece);
static void snake_clear(int center_x, int center_y);
static void snake_draw(const snake_piece *piece);
static void snake_read_direction_from_trail(snake_piece *piece);
static int snake_check_collisions(const snake_piece *piece);

struct snake snake;

enum snake_collision {
    kSnakeCollisionNone = 0,
    kSnakeCollisionDeath,
    kSnakeCollisionApple,
};

void snake_initialize() {
    snake.half_size = 4;
    // TODO: add support for dizziness
    snake.dizziness = 0;
    snake_reset();
}

void snake_reset() {
    snake = (struct snake){
        .head = (snake_piece){
            .x = 10,
            .y = 10,
            // TODO: set from getMilliseconds
            .lfsr = 1,
            .direction = kSnakeDirectionRight,
        },
    };
    memcpy(&snake.tail, &snake.head, sizeof(snake_piece));
    snake_advance_head();
}

void snake_update(display_slice slice) {
    display_clear(0, slice);
}

static void snake_advance_head() {
    snake_piece old_snake_head;
    memcpy(&old_snake_head, &snake.head, sizeof(snake_piece));
    // we actually want to clear out the old head so the
    // new head doesn't collide with it accidentally due to dizziness.
    snake_clear(old_snake_head.x, old_snake_head.y);
    snake_advance_piece(&snake.head);
    switch (snake_check_collisions(&snake.head)) {
        case kSnakeCollisionDeath:
            snake.game_over = 1;
            break;
        case kSnakeCollisionApple:
            snake_clear(snake.apple_x, snake.apple_y);
            break;
    }
    snake_draw(&old_snake_head);
    snake_draw(&snake.head);
}

static void snake_advance_tail() {
    snake_clear(snake.tail.x, snake.tail.y);
    snake_advance_piece(&snake.tail);
    snake_read_direction_from_trail(&snake.tail);
    // clearing might have gotten rid of part of the new tail (dizziness),
    // make sure to put it back, only AFTER we've read the direction from the trail.
    snake_draw(&snake.tail);
}

static void snake_advance_piece(snake_piece *piece) {
    // does not actually draw, in case we're the tail (where we don't want to overwrite the trail)
    // or the head (where we want to check collisions first).
    // TODO
}

static void snake_clear(int center_x, int center_y) {
    // TODO:
}

static void snake_draw(const snake_piece *piece) {
    // TODO:
}

static void snake_read_direction_from_trail(snake_piece *piece) {
    // could optimize for current heading (piece->direction),
    // i.e., since you can't go backwards, but that makes the code a bit messy.
    // we expect display_frame_pixel(piece->x, piece->y) to be 1.
    if (!display_frame_pixel(piece->x + 1, piece->y)) {
        piece->direction = kSnakeDirectionRight;
    } else if (!display_frame_pixel(piece->x, piece->y - 1)) { 
        piece->direction = kSnakeDirectionUp;
    } else if (!display_frame_pixel(piece->x - 1, piece->y)) {
        piece->direction = kSnakeDirectionLeft;
    } else if (!display_frame_pixel(piece->x, piece->y + 1)) { 
        piece->direction = kSnakeDirectionDown;
    } else {
        snake.game_over = 1;
        // TODO: error message here
    }
}

static int snake_check_collisions(const snake_piece *piece) {
    // TODO:
    return kSnakeCollisionNone;
}
