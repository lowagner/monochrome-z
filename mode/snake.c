#include "snake.h"

#include "../core/runtime.h"

#include <string.h> // memcpy

static void snake_advance();
static int snake_advance_head();
static void snake_advance_tail();
static void snake_advance_piece(snake_piece *piece);
static void snake_clear(int center_x, int center_y);
static void snake_draw(const snake_piece *piece);
static void snake_read_direction_from_trail(snake_piece *piece);
static int snake_check_collisions(const snake_piece *piece);
static void snake_maybe_add_apple();
static int snake_add_apple();

struct snake snake;

static int snake_needs_init = 1;
static int snake_needs_reset = 1;

#define GAME_OVER 130

enum snake_collision {
    kSnakeCollisionNone = 0,
    kSnakeCollisionDeath,
    kSnakeCollisionApple,
};

void snake_initialize() {
    snake.counter = 0;
    snake.inverse_speed = 3;
    snake.half_size = 8;
    // TODO: add support for dizziness
    snake.dizziness = 0;

    snake_needs_init = 0;
}

void snake_reset() {
    snake.state = (snake_state){
        .head = (snake_piece){
            .x = 20 + snake.half_size,
            .y = 20 + snake.half_size,
            // TODO: set from getMilliseconds
            .lfsr = 1,
            .direction = kSnakeDirectionRight,
        },
        .game_over = 0,
        .score = 0,
    };
    snake.state.apple.present = 0;
    memcpy(&snake.state.tail, &snake.state.head, sizeof(snake_piece));
    snake_advance_head();

    snake_needs_reset = 0;
}

void snake_update(display_slice slice) {
    if (runtime.transition.counter) {
        playdate->system->logToConsole("snake counter %d", runtime.transition.counter);
        // we rely on the display pixels to contain the game state, so
        // unless we have the full display to work with, don't do anything else:
        snake_needs_reset = 1;
        display_clear_alternating(85, 170, slice);
    } else if (snake_needs_init) {
        playdate->system->logToConsole("snake init");
        snake_initialize();
    } else if (snake_needs_reset) {
        playdate->system->logToConsole("snake reset");
        snake_reset();

        display_clear(0, slice);
        snake_draw(&snake.state.head);
        snake_draw(&snake.state.tail);
    } else if (snake.state.game_over) {
        playdate->system->logToConsole("snake game over %d", snake.state.game_over);
        // wait before resetting:
        if (--snake.state.game_over <= 0) {
            snake_needs_reset = 1;
        }
    } else if (++snake.counter < snake.inverse_speed) {
        playdate->system->logToConsole("snake waiting");
        // other logic while we wait for snake to move.
        snake_maybe_add_apple();
    } else {
        playdate->system->logToConsole("snake advancing");
        snake.counter = 0;
        snake_advance();
        snake_maybe_add_apple();
    }
}

void snake_advance() {
    playdate->system->logToConsole("updating snake %d, %d", snake.state.head.x, snake.state.head.y);
    if (snake_advance_head() == kSnakeCollisionApple) {
        return;
    }
    // advancing was either death or into nothing,
    // either way, we should advance the tail:
    snake_advance_tail();
}

static int snake_advance_head() {
    snake_piece old_snake_head;
    memcpy(&old_snake_head, &snake.state.head, sizeof(snake_piece));
    // we actually want to clear out the old head so the
    // new head doesn't collide with it accidentally due to dizziness.
    snake_clear(old_snake_head.x, old_snake_head.y);
    snake_advance_piece(&snake.state.head);
    int collision_result = snake_check_collisions(&snake.state.head);
    switch (collision_result) {
        case kSnakeCollisionDeath:
            playdate->system->logToConsole("oh no, ded");
            snake.state.game_over = GAME_OVER;
            break;
        case kSnakeCollisionApple:
            snake_clear(snake.state.apple.x, snake.state.apple.y);
            snake.state.apple.present = 0;
            break;
    }
    snake_draw(&old_snake_head);
    switch (snake.state.head.direction) {
        case kSnakeDirectionRight:
            snake.state.head.x += snake.half_size;
            break;
        case kSnakeDirectionUp:
            snake.state.head.y -= snake.half_size;
            break;
        case kSnakeDirectionLeft:
            snake.state.head.x -= snake.half_size;
            break;
        case kSnakeDirectionDown:
            snake.state.head.y += snake.half_size;
            break;
        default:
            snake.state.game_over = GAME_OVER;
    }
    snake_draw(&snake.state.head);
    return collision_result;
}

static void snake_advance_tail() {
    snake_clear(snake.state.tail.x, snake.state.tail.y);
    snake_advance_piece(&snake.state.tail);
    snake_read_direction_from_trail(&snake.state.tail);
    // clearing might have gotten rid of part of the new tail (dizziness),
    // make sure to put it back, only AFTER we've read the direction from the trail.
    snake_draw(&snake.state.tail);
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
        snake.state.game_over = GAME_OVER;
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
    if (snake.state.apple.present && display_box_box_collision(
        snake_box,
        snake.state.apple.box
    )) {
        return kSnakeCollisionApple;
    }
    return kSnakeCollisionDeath;
}

static void snake_maybe_add_apple() {
    if (!snake.state.apple.present)
    for (int tries = 0; tries < 10; ++tries) {
        if (snake_add_apple()) {
            snake.state.apple.present = 1;
            return;
        }
    }
}

static int snake_add_apple() {
    // returns 0 if unsuccessful, otherwise 1.
    // TODO
    return 0;
}

#ifndef NDEBUG
void test__mode__snake() {
    snake_piece test_piece = {.x = 12, .y = 13, .lfsr = 1, .direction = kSnakeDirectionRight};
    TEST(
        display_clear(0, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
        snake.half_size = 4;
        test_piece.x = 12;
        test_piece.y = 13;
        test_piece.lfsr = 1;
        test_piece.direction = kSnakeDirectionRight;
        snake_draw(&test_piece);
        for (int y = 0; y < LCD_ROWS; ++y) 
        for (int x = 0; x < LCD_COLUMNS; ++x) {
            TEST(
                EXPECT_INT_EQUAL(display_pixel_collision(x, y), (
                        y >= test_piece.y - snake.half_size
                    &&  y <= test_piece.y + snake.half_size
                    &&  x >= test_piece.x - snake.half_size
                    &&  x <= test_piece.x + snake.half_size
                    &&  (x != test_piece.x + 1 || y != test_piece.y)
                )),
                "%s: at (x, y) = (%d, %d)", AT, x, y
            );
        },
        "%s: can draw a snake piece going right within a byte boundary", AT
    );
}
#endif
