#include "snake.h"

#include "../core/buttons.h"
#include "../core/runtime.h"

#include <string.h> // memcpy

static void snake_advance();
static void snake_advance_head();
static void snake_advance_tail();
static void snake_advance_piece_no_draw(snake_piece *piece);
static void snake_clear(int left_x, int top_y);
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
    playdate->system->logToConsole("snake init");
    snake.starting_length = 20;
    snake.size = 10;
    // TODO: add support for dizziness
    snake.dizziness = 0;
    snake.inverse_speed = 3;

    snake_needs_init = 0;
}

void snake_reset() {
    playdate->system->logToConsole("snake reset");
    if (snake.size < 2) {
        snake.size = 2;
    } else if (snake.size > 40) {
        snake.size = 40;
    }
    snake.state = (snake_state){
        .counter = 0,
        .desired_direction = kSnakeDirectionRight,
        .head = (snake_piece){
            .x = 4 * snake.size,
            .y = 4 * snake.size,
            // TODO: set from getMilliseconds
            .lfsr = 1,
            .direction = kSnakeDirectionRight,
        },
        // size will be at least 2, and we'll increment to size 2 in this reset
        // method (see snake_advance_piece_no_draw), so only count delta above that:
        .size_delta = snake.starting_length > 2 ? snake.starting_length - 2 : 0,
        .game_over = 0,
        .score = 0,
    };
    snake.state.apple.present = 0;
    memcpy(&snake.state.tail, &snake.state.head, sizeof(snake_piece));
    snake_advance_piece_no_draw(&snake.state.head);

    snake_needs_reset = 0;
}

void snake_update(display_slice slice) {
    if (runtime.transition.counter) {
        // we rely on the display pixels to contain the game state, so
        // unless we have the full display to work with, don't do anything else:
        snake_needs_reset = 1;
        display_clear_alternating(85, 170, slice);
    } else if (snake_needs_init) {
        snake_initialize();
    } else if (snake_needs_reset) {
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
    } else {
        // actual game playing:
        int x_axis = (
                ( (buttons.current & kButtonLeft) ? -1 : 0 )
            +   ( (buttons.current & kButtonRight) ? +1 : 0 )
        );
        int y_axis = (
                ( (buttons.current & kButtonUp) ? -1 : 0 )
            +   ( (buttons.current & kButtonDown) ? +1 : 0 )
        );
        if (x_axis || y_axis) {
            if (x_axis && y_axis) {
                // we have to guess where the player wants to go
                if ((snake.state.head.direction & 1) == 1) {
                    // snake was traveling in the y direction, so we can switch to the x direction:
                    snake.state.desired_direction = -x_axis + 1;
                } else {
                    // snake was traveling in the x direction, so we can switch to the y direction:
                    snake.state.desired_direction = y_axis + 2;
                }
            } else if (x_axis) {
                // just x_axis
                if ((snake.state.head.direction & 1) == 1) {
                    // snake was traveling in the y direction, so we can switch to the x direction:
                    snake.state.desired_direction = -x_axis + 1;
                }
            } else {
                // just y_axis
                if ((snake.state.head.direction & 1) == 0) {
                    // snake was traveling in the x direction, so we can switch to the y direction:
                    snake.state.desired_direction = y_axis + 2;
                }
            }
        }
        
        if (++snake.state.counter < snake.inverse_speed) {
            // other logic while we wait for snake to move.
            snake_maybe_add_apple();
        } else {
            snake.state.counter = 0;
            snake_advance();
            snake_maybe_add_apple();
        }
    }
}

void snake_advance() {
    playdate->system->logToConsole("advancing snake from (%d, %d)", snake.state.head.x, snake.state.head.y);
    snake.state.head.direction = snake.state.desired_direction;
    snake_advance_head();
    if (snake.state.size_delta == 0) {
        snake_advance_tail();
    } else if (snake.state.size_delta > 0) {
        --snake.state.size_delta;
    } else {
        // TODO: support this, maybe, but require having at least length == 2
        playdate->system->logToConsole("invalid snake size delta: %d", snake.state.size_delta);
        snake.state.game_over = GAME_OVER;
    }
}

static void snake_advance_head() {
    playdate->system->logToConsole("adv snk head from %d, %d", snake.state.head.x, snake.state.head.y);
    snake_piece old_snake_head;
    memcpy(&old_snake_head, &snake.state.head, sizeof(snake_piece));
    // we actually want to clear out the old head so the
    // new head doesn't collide with it accidentally due to dizziness.
    snake_clear(old_snake_head.x, old_snake_head.y);
    snake_advance_piece_no_draw(&snake.state.head);
    int collision_result = snake_check_collisions(&snake.state.head);
    switch (collision_result) {
        case kSnakeCollisionDeath:
            playdate->system->logToConsole("oh no, ded");
            snake.state.game_over = GAME_OVER;
            break;
        case kSnakeCollisionApple:
            ++snake.state.size_delta;
            snake_clear(snake.state.apple.box.start_x, snake.state.apple.box.start_y);
            snake.state.apple.present = 0;
            break;
    }
    snake_draw(&old_snake_head);
    snake_draw(&snake.state.head);
}

static void snake_advance_tail() {
    playdate->system->logToConsole("adv snk tail from %d, %d", snake.state.tail.x, snake.state.tail.y);
    snake_clear(snake.state.tail.x, snake.state.tail.y);
    snake_advance_piece_no_draw(&snake.state.tail);
    snake_read_direction_from_trail(&snake.state.tail);
    // clearing might have gotten rid of part of the new tail (dizziness),
    // make sure to put it back, only AFTER we've read the direction from the trail.
    snake_draw(&snake.state.tail);
}

static void snake_advance_piece_no_draw(snake_piece *piece) {
    // does not actually draw, in case we're the tail (where we don't want to overwrite the trail)
    // or the head (where we want to check collisions first).
    int delta_direction = 0;
    int delta_orthogonal = 0;
    if (snake.dizziness) {
        lfsr32_next(&piece->lfsr);
        // TODO: affect delta_direction and delta_orthogonal.
        // delta_direction should be between -max(snake.size - 3, 0) and 0,
        // and should get more negative when delta_orthogonal is larger.
        // delta_orthogonal should be in +-(snake.size - 1)
        //int max_delta = snake.size - 1;
        uint32_t random_walk = piece->lfsr;
    }
    switch (piece->direction) {
        case kSnakeDirectionRight:
            piece->x += snake.size + delta_direction;
            piece->y += delta_orthogonal;
            break;
        case kSnakeDirectionUp:
            piece->x -= delta_orthogonal;
            piece->y -= snake.size + delta_direction;
            break;
        case kSnakeDirectionLeft:
            piece->x -= snake.size + delta_direction;
            piece->y -= delta_orthogonal;
            break;
        case kSnakeDirectionDown:
            piece->x += delta_orthogonal;
            piece->y += snake.size + delta_direction;
            break;
        default:
            snake.state.game_over = GAME_OVER;
            playdate->system->logToConsole(
                "invalid direction for snake to advance in %d",
                piece->direction
            );
    }
    playdate->system->logToConsole("* advanced snk piece to %d, %d", piece->x, piece->y);
}

static void snake_clear(int left_x, int top_y) {
    playdate->system->logToConsole("clear snk piece %d, %d", left_x, top_y);
    display_box_draw(0, (display_box){
        .start_x = left_x,
        .start_y = top_y,
        .end_x = left_x + snake.size,
        .end_y = top_y + snake.size,
    });
}

static void snake_draw(const snake_piece *piece) {
    playdate->system->logToConsole("drw snk piece %d, %d", piece->x, piece->y);
    display_box_draw(255, (display_box){
        .start_x = piece->x,
        .start_y = piece->y,
        .end_x = piece->x + snake.size,
        .end_y = piece->y + snake.size,
    });
    int half_size = snake.size / 2;
    if (snake.size % 2) {
        switch (piece->direction) {
            case kSnakeDirectionRight:
                display_pixel_clear(piece->x + half_size + 1, piece->y + half_size);
                break;
            case kSnakeDirectionUp:
                display_pixel_clear(piece->x + half_size, piece->y + half_size - 1);
                break;
            case kSnakeDirectionLeft:
                display_pixel_clear(piece->x + half_size - 1, piece->y + half_size);
                break;
            case kSnakeDirectionDown:
                display_pixel_clear(piece->x + half_size, piece->y + half_size + 1);
                break;
        }
    } else {
        switch (piece->direction) {
            case kSnakeDirectionRight:
                display_pixel_clear(piece->x + half_size, piece->y + half_size - 1);
                break;
            case kSnakeDirectionUp:
                display_pixel_clear(piece->x + half_size - 1, piece->y + half_size - 1);
                break;
            case kSnakeDirectionLeft:
                display_pixel_clear(piece->x + half_size - 1, piece->y + half_size);
                break;
            case kSnakeDirectionDown:
                display_pixel_clear(piece->x + half_size, piece->y + half_size);
                break;
        }
    }
}

static void snake_read_direction_from_trail(snake_piece *piece) {
    // could optimize for current heading (piece->direction),
    // i.e., since you can't go backwards, but that makes the code a bit messy.
    int half_size = snake.size / 2;
    if (snake.size % 2) {
        if (!display_pixel_collision(piece->x + half_size + 1, piece->y + half_size)) {
            piece->direction = kSnakeDirectionRight;
        } else if (!display_pixel_collision(piece->x + half_size, piece->y + half_size - 1)) { 
            piece->direction = kSnakeDirectionUp;
        } else if (!display_pixel_collision(piece->x + half_size - 1, piece->y + half_size)) {
            piece->direction = kSnakeDirectionLeft;
        } else if (!display_pixel_collision(piece->x + half_size, piece->y + half_size + 1)) { 
            piece->direction = kSnakeDirectionDown;
        } else {
            playdate->system->logToConsole("couldn't read trail @ %d, %d", piece->x, piece->y);
            snake.state.game_over = GAME_OVER;
        }
    } else {
        if (!display_pixel_collision(piece->x + half_size, piece->y + half_size - 1)) {
            piece->direction = kSnakeDirectionRight;
        } else if (!display_pixel_collision(piece->x + half_size - 1, piece->y + half_size - 1)) { 
            piece->direction = kSnakeDirectionUp;
        } else if (!display_pixel_collision(piece->x + half_size - 1, piece->y + half_size)) {
            piece->direction = kSnakeDirectionLeft;
        } else if (!display_pixel_collision(piece->x + half_size, piece->y + half_size)) { 
            piece->direction = kSnakeDirectionDown;
        } else {
            playdate->system->logToConsole("couldn't read trail @ %d, %d", piece->x, piece->y);
            snake.state.game_over = GAME_OVER;
        }
    }
}

static int snake_check_collisions(const snake_piece *piece) {
    playdate->system->logToConsole("checking collisions @ %d, %d", piece->x, piece->y);
    display_box snake_box = {
        .start_x = piece->x,
        .start_y = piece->y,
        .end_x = piece->x + snake.size,
        .end_y = piece->y + snake.size,
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
    for (int tries = 0; tries < 5; ++tries) {
        if (snake_add_apple()) {
            return;
        }
    }
}

static int snake_add_apple() {
    // returns 0 if unsuccessful, otherwise 1.
    int x = rand() % (LCD_COLUMNS - snake.size);
    int y = rand() % (LCD_ROWS - snake.size);
    snake.state.apple.box.start_x = x;
    snake.state.apple.box.end_x = x + snake.size;
    snake.state.apple.box.start_y = y;
    snake.state.apple.box.end_y = y + snake.size;
    if (display_box_collision(snake.state.apple.box)) {
        return 0;
    }
    display_box_draw_alternating(85, 170, snake.state.apple.box);
    snake.state.apple.present = 1;
    return 1;
}

#ifndef NDEBUG
void test__mode__snake() {
    snake_piece test_piece;
    TEST(
        display_clear(0, (display_slice){.start_row = 0, .end_row = LCD_ROWS});
        snake.size = 9;
        test_piece.x = 12;
        test_piece.y = 13;
        test_piece.lfsr = 1;
        test_piece.direction = kSnakeDirectionRight;
        snake_draw(&test_piece);
        for (int y = 0; y < LCD_ROWS; ++y) 
        for (int x = 0; x < LCD_COLUMNS; ++x) {
            TEST(
                EXPECT_INT_EQUAL_LOGGED(display_pixel_collision(x, y), (
                        y >= test_piece.y
                    &&  y < test_piece.y + snake.size
                    &&  x >= test_piece.x
                    &&  x < test_piece.x + snake.size
                    &&  (x != test_piece.x + snake.size/2 + 1 || y != test_piece.y + snake.size/2)
                )),
                "%s: at (x, y) = (%d, %d)", AT, x, y
            );
        },
        "%s: can draw a snake piece going right within a byte boundary", AT
    );
}
#endif
