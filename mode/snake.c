#include "snake.h"

#include "../core/buttons.h"
#include "../core/runtime.h"

#include <string.h> // memcpy

enum snake_direction_t {
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
    uint8_t dizzy;  // read from the trail.
}
    snake_piece_t;

typedef struct snake_state {
    int counter;
    int current_length;
    int dizziness;
    // don't modify the direction on head.direction until
    // we actually advance the snake, so we can see if we
    // want to move diagonally:
    int desired_direction;
    snake_piece_t head;
    snake_piece_t tail;
    int size_delta;
    struct {
        display_box_t box;
        int present;
    }
        apple;
    int game_over;
    uint64_t score;
}
    snake_state_t;

static void snake_advance();
static void snake_advance_head();
static void snake_advance_tail();
static void snake_advance_piece_no_draw(snake_piece_t *piece);
static int snake_random_walk(snake_piece_t *piece);
static void snake_clear(int left_x, int top_y);
static void snake_draw_no_trail(const snake_piece_t *piece);
static void snake_draw(const snake_piece_t *piece);
static void snake_draw_tail(const snake_piece_t *piece);
static void snake_read_direction_and_dizziness_from_trail(snake_piece_t *piece);
static int snake_check_collisions(const snake_piece_t *piece);
static void snake_maybe_add_apple();
static int snake_add_apple();

snake_info_t next_snake = {
    .starting_length = 16,
    .size = 10,
    // TODO: add support for dizziness
    .dizziness = 0,
    .inverse_speed = 3,
};

static struct snake {
    snake_info_t info;
    snake_state_t state;
}
    snake;

static int snake_needs_init = 1;
static int snake_needs_reset = 1;

#define GAME_OVER 130

enum snake_collision {
    kSnakeCollisionNone = 0,
    kSnakeCollisionDeath,
    kSnakeCollisionApple,
};

void snake_speed_set_value(int index) {
    next_snake.inverse_speed = 11 - index;
}

int snake_speed_get_index() {
    return 11 - next_snake.inverse_speed;
}

const char *snake_speed_options[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
};

runtime_menu_t snake_speed_menu = {
    .pd_menu = NULL,
    .title = "speed",
    .options = snake_speed_options,
    .option_count = 11,
    .set_value_from_index = snake_speed_set_value,
    .get_index_from_value = snake_speed_get_index,
};

void snake_length_set_value(int index) {
    next_snake.starting_length = 2 << index;
}

int snake_length_get_index() {
    // lazy `log(length/2) / log(2)`
    int length = next_snake.starting_length / 2;
    int index = 0;
    while (length / 2) {
        length /= 2;
        if (++index > 10) {
            break;
        }
    }
    return index;
}

const char *snake_length_options[] = {
    "2",
    "4",
    "8",
    "16",
    "32",
    "64",
    "128",
    "256",
    "512",
    "1024",
    "2048",
};

runtime_menu_t snake_length_menu = {
    .pd_menu = NULL,
    .title = "length",
    .options = snake_length_options,
    .option_count = 11,
    .set_value_from_index = snake_length_set_value,
    .get_index_from_value = snake_length_get_index,
};

const int snake_size_values[] = {
    2,
    4,
    5,
    8,
    10,
    16,
    20,
    40,
};

void snake_size_set_value(int index) {
    if (index < 0) index = 0;
    if (index > 7) index = 7;
    next_snake.size = snake_size_values[index];
}

int snake_size_get_index() {
    int index = -1;
    while (++index < 8) {
        if (next_snake.size <= snake_size_values[index]) {
            break;
        }
    }
    return index;
}

const char *snake_size_options[] = {
    "2",
    "4",
    "5",
    "8",
    "10",
    "16",
    "20",
    "40",
};

runtime_menu_t snake_size_menu = {
    .pd_menu = NULL,
    .title = "size",
    .options = snake_size_options,
    .option_count = 8,
    .set_value_from_index = snake_size_set_value,
    .get_index_from_value = snake_size_get_index,
};

static void snake_initialize() {
    playdate->system->logToConsole("snake init");

    runtime_add_menu(&snake_speed_menu);
    runtime_add_menu(&snake_length_menu);
    runtime_add_menu(&snake_size_menu);

    snake_needs_init = 0;
}

void snake_reset(display_slice_t slice) {
    playdate->system->logToConsole("snake reset");
    snake.info = next_snake;
    if (snake.info.size < 2) {
        snake.info.size = 2;
    } else if (snake.info.size > 40) {
        snake.info.size = 40;
    }
    // require being at least 2 units long:
    next_snake.starting_length = next_snake.starting_length > 2 ? next_snake.starting_length : 2;
    uint32_t time = playdate->system->getCurrentTimeMilliseconds();
    snake.state = $(snake_state){
        .counter = 0,
        .current_length = next_snake.starting_length,
        .desired_direction = kSnakeDirectionRight,
        .head = $(snake_piece){
            .x = 2 * snake.info.size,
            .y = 3 * snake.info.size,
            .lfsr = time ? time : 1,
            .direction = kSnakeDirectionRight,
            .dizzy = next_snake.dizziness,
        },
        // size will be at least 2, and we'll increment to size 2 in this reset
        // method (see snake_advance_piece_no_draw), so only count delta above that:
        .size_delta = next_snake.starting_length - 2,
        .game_over = 0,
        .score = 0,
    };
    snake.state.apple.present = 0;
    memcpy(&snake.state.tail, &snake.state.head, sizeof $(snake_piece));
    snake_advance_piece_no_draw(&snake.state.head);

    display_slice_fill(0, slice);
    snake_draw_no_trail(&snake.state.head);
    snake_draw_tail(&snake.state.tail);

    snake_needs_reset = 0;
}

static void snake_game_loop();

void snake_update(display_slice_t slice) {
    if (runtime.transition.counter) {
        if (runtime.transition.next_mode != kRuntimeModeSnake) {
            // we'll need to add back menus next time, since
            // runtime will clear any PD menus.
            snake_needs_init = 1;
            snake_speed_menu.pd_menu = NULL;
        }
        // we rely on the display pixels to contain the game state, so
        // unless we have the full display to work with, don't do anything else:
        snake_needs_reset = 1;
        display_slice_fill_alternating(85, 170, slice);
    } else if (snake_needs_init) {
        snake_initialize();
    } else if (snake_needs_reset) {
        snake_reset(slice);
    } else {
        snake_game_loop();
    }
}

static void snake_game_loop() {
    if (buttons.pushed & kButtonA) {
        display_invert();
    }
    if (buttons.pushed & kButtonB) {
        // let pushing this button affect the next run as well.
        // note that snake.info.dizziness is ignored since we need to have dizziness
        // on a per-segment basis (since we can switch dizziness on and off).
        if (next_snake.dizziness) {
            next_snake.dizziness = 0;
        } else {
            next_snake.dizziness = 1;
        }
        snake.state.head.dizzy = next_snake.dizziness;
    }
    if (snake.state.game_over) {
        if (snake.state.game_over == GAME_OVER) {
            display_invert();
        }
        playdate->system->logToConsole("snake game over %d", snake.state.game_over);
        // wait before resetting:
        if (--snake.state.game_over <= 0) {
            snake_needs_reset = 1;
            display_invert();
        }
        return;
    }
    // actual game playing:
    int x_axis, y_axis;
    buttons_axis_current(&x_axis, &y_axis);
    if (x_axis && (snake.state.head.direction & 1) == 1) {
        // snake was traveling in the y direction, so we can switch to the x direction:
        snake.state.desired_direction = -x_axis + 1;
    } else if (y_axis && (snake.state.head.direction & 1) == 0) {
        // snake was traveling in the x direction, so we can switch to the y direction:
        snake.state.desired_direction = y_axis + 2;
    }
    
    if (++snake.state.counter < snake.info.inverse_speed) {
        // other logic while we wait for snake to move.
        snake_maybe_add_apple();
    } else {
        snake.state.counter = 0;
        snake_advance();
        snake_maybe_add_apple();
    }
}

void snake_advance() {
    playdate->system->logToConsole("advancing snake from (%d, %d)", snake.state.head.x, snake.state.head.y);
    
    int head_dizziness = snake.state.head.dizzy;
    if (snake.state.desired_direction != snake.state.head.direction) {
        snake.state.head.direction = snake.state.desired_direction;
        // don't be dizzy when turning, or you might hit yourself:
        snake.state.head.dizzy = 0;
    }
    // let the tail move first so that the head can do a narrow miss.
    // we might need to draw it back later, though, if the snake grew by eating an apple.
    snake_clear(snake.state.tail.x, snake.state.tail.y);
    snake_advance_head();
    snake.state.head.dizzy = head_dizziness;
    if (snake.state.size_delta == 0) {
        snake_advance_tail();
        if (snake.state.game_over) {
            // ensure that the head is visible, in case the head crashed into the tail.
            snake_draw_no_trail(&snake.state.head);
        }
    } else if (snake.state.size_delta > 0) {
        // draw tail back since we cleared it for narrow misses (see comment above).
        snake_draw_tail(&snake.state.tail);
        --snake.state.size_delta;
        ++snake.state.current_length;
    } else {
        // TODO: support this, maybe, but require having at least length == 2
        playdate->system->logToConsole("invalid snake size delta: %d", snake.state.size_delta);
        snake.state.game_over = GAME_OVER;
    }
}

static void snake_advance_head() {
    playdate->system->logToConsole("adv snk head from %d, %d", snake.state.head.x, snake.state.head.y);
    snake_piece_t old_snake_head;
    memcpy(&old_snake_head, &snake.state.head, sizeof $(snake_piece));
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
    snake_draw_no_trail(&snake.state.head);
}

static void snake_advance_tail() {
    playdate->system->logToConsole("adv snk tail from %d, %d", snake.state.tail.x, snake.state.tail.y);
    // don't need to clear the tail since that is done in snake_advance to avoid narrow misses
    // with the head.
    snake_advance_piece_no_draw(&snake.state.tail);
    snake_read_direction_and_dizziness_from_trail(&snake.state.tail);
    // clearing might have gotten rid of part of the new tail (dizziness),
    // make sure to put it back, only AFTER we've read the direction from the trail.
    snake_draw_tail(&snake.state.tail);
}

static void snake_advance_piece_no_draw(snake_piece_t *piece) {
    // does not actually draw, in case we're the tail (where we don't want to overwrite the trail)
    // or the head (where we want to check collisions first).
    int delta_orthogonal = snake_random_walk(piece);
    switch (piece->direction) {
        case kSnakeDirectionRight:
            piece->x += snake.info.size;
            piece->y += delta_orthogonal;
            break;
        case kSnakeDirectionUp:
            piece->x -= delta_orthogonal;
            piece->y -= snake.info.size;
            break;
        case kSnakeDirectionLeft:
            piece->x -= snake.info.size;
            piece->y -= delta_orthogonal;
            break;
        case kSnakeDirectionDown:
            piece->x += delta_orthogonal;
            piece->y += snake.info.size;
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

static int snake_random_walk(snake_piece_t *piece) {
    if (piece->dizzy == 0) {
        return 0;
    }
    lfsr32_next(&piece->lfsr);
    // delta_orthogonal should be in +-(snake.info.size - 1)
    // we won't go all the way up to these limits when size is large, however,
    // just to keep this loop down to <10 cycles.
    int max_abs_delta = snake.info.size / 10 + 1;
    uint32_t random_walk = piece->lfsr / 7;
    int delta_orthogonal = 0;
    for (int walk = 0; walk < max_abs_delta; ++walk) {
        delta_orthogonal += (random_walk % 3) - 1; // between -1 and +1
        random_walk /= 3;
    }
    return delta_orthogonal;
}

static void snake_clear(int left_x, int top_y) {
    playdate->system->logToConsole("clear snk piece %d, %d", left_x, top_y);
    display_box_fill(0, $(display_box){
        .start_x = left_x,
        .start_y = top_y,
        .end_x = left_x + snake.info.size,
        .end_y = top_y + snake.info.size,
    });
}

static void snake_draw_no_trail(const snake_piece_t *piece) {
    playdate->system->logToConsole("drw snk piece %d, %d", piece->x, piece->y);
    // TODO: for large enough snake size, don't draw the left/right sides of the
    // snake, so that it's easier to see the snake's lines when going along-side itself.
    // make sure that corners work out ok, though.  maybe have an "incoming_direction"
    // on the snake_piece_t.
    display_box_fill(255, $(display_box){
        .start_x = piece->x,
        .start_y = piece->y,
        .end_x = piece->x + snake.info.size,
        .end_y = piece->y + snake.info.size,
    });
}

static void snake_draw(const snake_piece_t *piece) {
    snake_draw_no_trail(piece);
    int half_size = snake.info.size / 2;
    if (snake.info.size % 2) {
        if (!piece->dizzy) {
            // center pixel, if present, indicates dizziness for odd sizes:
            display_pixel_clear(piece->x + half_size, piece->y + half_size);
        }
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
                if (!piece->dizzy) {
                    display_pixel_clear(piece->x + half_size, piece->y + half_size);
                }
                break;
            case kSnakeDirectionUp:
                if (!piece->dizzy) {
                    display_pixel_clear(piece->x + half_size, piece->y + half_size - 1);
                }
                display_pixel_clear(piece->x + half_size - 1, piece->y + half_size - 1);
                break;
            case kSnakeDirectionLeft:
                if (!piece->dizzy) {
                    display_pixel_clear(piece->x + half_size - 1, piece->y + half_size - 1);
                }
                display_pixel_clear(piece->x + half_size - 1, piece->y + half_size);
                break;
            case kSnakeDirectionDown:
                display_pixel_clear(piece->x + half_size, piece->y + half_size);
                if (!piece->dizzy) {
                    display_pixel_clear(piece->x + half_size - 1, piece->y + half_size);
                }
                break;
        }
    }
}

static void snake_draw_tail(const snake_piece_t *piece) {
    snake_draw(piece);
    if (snake.info.size <= 2) {
        return;
    }
    int delta = snake.info.size - 1;
    switch (piece->direction) {
        case kSnakeDirectionRight:
            display_pixel_clear(piece->x, piece->y);
            display_pixel_clear(piece->x, piece->y + delta);
            break;
        case kSnakeDirectionUp:
            display_pixel_clear(piece->x, piece->y + delta);
            display_pixel_clear(piece->x + delta, piece->y + delta);
            break;
        case kSnakeDirectionLeft:
            display_pixel_clear(piece->x + delta, piece->y);
            display_pixel_clear(piece->x + delta, piece->y + delta);
            break;
        case kSnakeDirectionDown:
            display_pixel_clear(piece->x, piece->y);
            display_pixel_clear(piece->x + delta, piece->y);
            break;
    }
    if (snake.info.size <= 5) {
        return;
    }
    switch (piece->direction) {
        case kSnakeDirectionRight:
            display_pixel_clear(piece->x + 1, piece->y);
            display_pixel_clear(piece->x, piece->y + 1);
            display_pixel_clear(piece->x, piece->y + delta - 1);
            display_pixel_clear(piece->x + 1, piece->y + delta);
            break;
        case kSnakeDirectionUp:
            display_pixel_clear(piece->x, piece->y + delta - 1);
            display_pixel_clear(piece->x + delta, piece->y + delta - 1);
            display_pixel_clear(piece->x + 1, piece->y + delta);
            display_pixel_clear(piece->x + delta - 1, piece->y + delta);
            break;
        case kSnakeDirectionLeft:
            display_pixel_clear(piece->x + delta - 1, piece->y);
            display_pixel_clear(piece->x + delta - 1, piece->y + delta);
            display_pixel_clear(piece->x + delta, piece->y + 1);
            display_pixel_clear(piece->x + delta, piece->y + delta - 1);
            break;
        case kSnakeDirectionDown:
            display_pixel_clear(piece->x + 1, piece->y);
            display_pixel_clear(piece->x + delta - 1, piece->y);
            display_pixel_clear(piece->x, piece->y + 1);
            display_pixel_clear(piece->x + delta, piece->y + 1);
            break;
    }
}

static void snake_read_direction_and_dizziness_from_trail(snake_piece_t *piece) {
    // could optimize for current heading (piece->direction),
    // i.e., since you can't go backwards, but that makes the code a bit messy.
    int half_size = snake.info.size / 2;
    if (snake.info.size % 2) {
        // center pixel, if present, indicates dizziness for odd sizes:
        piece->dizzy = display_pixel_collision(piece->x + half_size, piece->y + half_size);
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
        int missing_pieces = (
            // top left == 1
                (!display_pixel_collision(
                    piece->x + half_size - 1,
                    piece->y + half_size - 1
                ) << 0)
            // top right == 2
            |   (!display_pixel_collision(
                    piece->x + half_size,
                    piece->y + half_size - 1
                ) << 1)
            // bottom left == 4
            |   (!display_pixel_collision(
                    piece->x + half_size - 1,
                    piece->y + half_size
                ) << 2)
            // bottom right == 8
            |   (!display_pixel_collision(
                    piece->x + half_size,
                    piece->y + half_size
                ) << 3)
        );
        switch (missing_pieces) {
            case 1: // top-left missing only
                piece->direction = kSnakeDirectionUp;
                piece->dizzy = 1;
                break;
            case 2: // top-right missing only
                piece->direction = kSnakeDirectionRight;
                piece->dizzy = 1;
                break;
            case 4: // bottom-left missing only
                piece->direction = kSnakeDirectionLeft;
                piece->dizzy = 1;
                break;
            case 8: // bottom-right missing only
                piece->direction = kSnakeDirectionDown;
                piece->dizzy = 1;
                break;
            case 3: // top missing: 2 + 1
                piece->direction = kSnakeDirectionUp;
                piece->dizzy = 0;
                break;
            case 5: // left missing: 4 + 1
                piece->direction = kSnakeDirectionLeft;
                piece->dizzy = 0;
                break;
            case 10: // right missing: 8 + 2
                piece->direction = kSnakeDirectionRight;
                piece->dizzy = 0;
                break;
            case 12: // bottom missing: 4 + 8
                piece->direction = kSnakeDirectionDown;
                piece->dizzy = 0;
                break;
            default:
                playdate->system->logToConsole(
                    "weird missing pieces %d @ %d, %d",
                    missing_pieces, piece->x, piece->y
                );
                snake.state.game_over = GAME_OVER;
        }
    }
}

static int snake_check_collisions(const snake_piece_t *piece) {
    playdate->system->logToConsole("checking collisions @ %d, %d", piece->x, piece->y);
    display_box_t snake_box = {
        .start_x = piece->x,
        .start_y = piece->y,
        .end_x = piece->x + snake.info.size,
        .end_y = piece->y + snake.info.size,
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
    int x = rand() % (LCD_COLUMNS - snake.info.size);
    int y = rand() % (LCD_ROWS - snake.info.size);
    snake.state.apple.box.start_x = x;
    snake.state.apple.box.end_x = x + snake.info.size;
    snake.state.apple.box.start_y = y;
    snake.state.apple.box.end_y = y + snake.info.size;
    if (display_box_collision(snake.state.apple.box)) {
        return 0;
    }
    display_box_fill_alternating(85, 170, snake.state.apple.box);
    snake.state.apple.present = 1;
    return 1;
}

#ifndef NDEBUG
void test__mode__snake() {
    snake_piece_t test_piece;
    TEST(
        display_slice_fill(0, $(display_slice){.start_row = 0, .end_row = LCD_ROWS});
        snake.info.size = 9;
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
                    &&  y < test_piece.y + snake.info.size
                    &&  x >= test_piece.x
                    &&  x < test_piece.x + snake.info.size
                    &&  (x != test_piece.x + snake.info.size/2 + 1 || y != test_piece.y + snake.info.size/2)
                )),
                "%s: at (x, y) = (%d, %d)", AT, x, y
            );
        },
        "%s: can draw a snake piece going right within a byte boundary", AT
    );
}
#endif
