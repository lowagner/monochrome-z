#include "snake.h"

#include "../core/runtime.h"

#include <string.h> // memcpy

static int snake_advance_head();
static void snake_advance_tail();
static void snake_advance_piece(snake_piece *piece);
static void snake_clear(int center_x, int center_y);
static void snake_draw(const snake_piece *piece);
static void snake_read_direction_from_trail(snake_piece *piece);
static int snake_check_collisions(const snake_piece *piece);
static int snake_add_apple();

struct snake snake;

static int snake_needs_init = 1;
static int snake_needs_apple = 1;

enum snake_collision {
    kSnakeCollisionNone = 0,
    kSnakeCollisionDeath,
    kSnakeCollisionApple,
};

void snake_initialize() {
    snake.counter = 0;
    snake.inverse_speed = 10;
    snake.half_size = 8;
    // TODO: add support for dizziness
    snake.dizziness = 0;
    snake_reset();
}

void snake_reset() {
    snake = (struct snake){
        .head = (snake_piece){
            .x = 10 + snake.half_size,
            .y = 10 + snake.half_size,
            // TODO: set from getMilliseconds
            .lfsr = 1,
            .direction = kSnakeDirectionRight,
        },
    };
    snake_needs_apple = 1;
    memcpy(&snake.tail, &snake.head, sizeof(snake_piece));
    snake_advance_head();
}

void snake_update(display_slice slice) {
    if (runtime.transition.counter) {
        // we rely on the display pixels to contain the game state, so
        // unless we have the full display to work with, don't do anything else:
        snake_needs_init = 1;
        display_clear_alternating(85, 170, slice);
    } else if (snake_needs_init) {
        display_clear(0, slice);
        snake_initialize();
        snake_draw(&snake.head);
        snake_draw(&snake.tail);
        snake_needs_init = 0;
    } else if (snake.game_over) {
        snake_needs_init = 1;
        // TODO
    } else if (++snake.counter < snake.inverse_speed) {
        // other logic while we wait for snake to move??
    } else {
        snake.counter = 0;
        if (snake_advance_head() != kSnakeCollisionApple) {
            // advancing was either death or into nothing,
            // either way, we should advance the tail:
            snake_advance_tail();

            if (snake_needs_apple) {
                for (int tries = 0; tries < 10; ++tries) {
                    if (snake_add_apple()) {
                        snake_needs_apple = 0;
                        break;
                    }
                }
            }
        }
    }
}

static int snake_advance_head() {
    snake_piece old_snake_head;
    memcpy(&old_snake_head, &snake.head, sizeof(snake_piece));
    // we actually want to clear out the old head so the
    // new head doesn't collide with it accidentally due to dizziness.
    snake_clear(old_snake_head.x, old_snake_head.y);
    snake_advance_piece(&snake.head);
    int collision_result = snake_check_collisions(&snake.head);
    switch (collision_result) {
        case kSnakeCollisionDeath:
            snake.game_over = 1;
            break;
        case kSnakeCollisionApple:
            snake_clear(snake.apple.x, snake.apple.y);
            snake_needs_apple = 1;
            break;
    }
    snake_draw(&old_snake_head);
    snake_draw(&snake.head);
    return collision_result;
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
    display_box_draw(0, (display_box){
        .start_x = center_x - snake.half_size,
        .start_y = center_y - snake.half_size,
        .end_x = center_x + snake.half_size + 1,
        .end_y = center_y + snake.half_size + 1,
    });
}

static void snake_draw(const snake_piece *piece) {
    display_box_draw(255, (display_box){
        .start_x = piece->x - snake.half_size,
        .start_y = piece->y - snake.half_size,
        .end_x = piece->x + snake.half_size + 1,
        .end_y = piece->y + snake.half_size + 1,
    });
    switch (piece->direction) {
        case kSnakeDirectionRight:
            display_pixel_clear(piece->x + 1, piece->y);
            break;
        case kSnakeDirectionUp:
            display_pixel_clear(piece->x, piece->y - 1);
            break;
        case kSnakeDirectionLeft:
            display_pixel_clear(piece->x - 1, piece->y);
            break;
        case kSnakeDirectionDown:
            display_pixel_clear(piece->x, piece->y + 1);
            break;
    }
}

static void snake_read_direction_from_trail(snake_piece *piece) {
    // could optimize for current heading (piece->direction),
    // i.e., since you can't go backwards, but that makes the code a bit messy.
    // we expect display_pixel_collision(piece->x, piece->y) to be 1.
    if (!display_pixel_collision(piece->x + 1, piece->y)) {
        piece->direction = kSnakeDirectionRight;
    } else if (!display_pixel_collision(piece->x, piece->y - 1)) { 
        piece->direction = kSnakeDirectionUp;
    } else if (!display_pixel_collision(piece->x - 1, piece->y)) {
        piece->direction = kSnakeDirectionLeft;
    } else if (!display_pixel_collision(piece->x, piece->y + 1)) { 
        piece->direction = kSnakeDirectionDown;
    } else {
        snake.game_over = 1;
        // TODO: error message here
    }
}

static int snake_check_collisions(const snake_piece *piece) {
    display_box snake_box = {
        .start_x = piece->x - snake.half_size,
        .start_y = piece->y - snake.half_size,
        .end_x = piece->x + snake.half_size + 1,
        .end_y = piece->y + snake.half_size + 1,
    };
    int display_collision = display_box_collision(snake_box);
    if (!display_collision) {
        return kSnakeCollisionNone;
    }
    if (display_box_box_collision(snake_box, snake.apple.box)) {
        return kSnakeCollisionApple;
    }
    return kSnakeCollisionDeath;
}

static int snake_add_apple() {
    // returns 0 if unsuccessful, otherwise 1.
    // TODO
    return 0;
}

#ifndef NDEBUG
void test__mode__snake() {
    TEST(
        display_clear(0, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
        // TODO
        {},
        "%s: can draw a snake piece right", AT
    );
}
#endif
