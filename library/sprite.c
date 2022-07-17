#include "sprite.h"

#include "../core/data.h"
#include "../core/error.h"

#include <stdlib.h> // qsort

static int sprite_compare(const void *void_index1, const void *void_index2);

static int internal_sprite_count = 0;
static sprite_t sprites[MAX_SPRITE_COUNT];
static uint8_t ordered_sprites[MAX_SPRITE_COUNT];
// the "head" of this list is the last element (sprite_free_list[MAX_SPRITE_COUNT])
static uint8_t sprite_free_list[MAX_SPRITE_COUNT + 1];
static uint8_t sprite_redraw_areas[(LCD_ROWS / 16) * (LCD_COLUMNS / 16) / 8 + 1]; 

void sprite_reset() {
    internal_sprite_count = 0;
    for (int i = 0; i < MAX_SPRITE_COUNT; ++i) {
        sprite_free_list[i] = i + 1;
    }
    // end of the list is the head of the free list:
    sprite_free_list[MAX_SPRITE_COUNT] = 0;
}

int sprite_count() {
    return internal_sprite_count;
}

sprite_t *sprite_add() {
    ASSERT(internal_sprite_count < MAX_SPRITE_COUNT);
    int to_be_used_index = sprite_free_list[MAX_SPRITE_COUNT];
    ASSERT(to_be_used_index < MAX_SPRITE_COUNT);
    sprite_free_list[MAX_SPRITE_COUNT] = sprite_free_list[to_be_used_index];
    sprite_free_list[to_be_used_index] = MAX_SPRITE_COUNT;
    // will actually need to order this sprite, but that's done in the "draw" method:
    ordered_sprites[internal_sprite_count++] = to_be_used_index;
    playdate->system->logToConsole("adding sprite %d", to_be_used_index);
    return sprites + to_be_used_index;
}

void sprite_remove(sprite_t *sprite) {
    ASSERT(internal_sprite_count > 0);
    int now_unused_sprite_index = (int)(sprite - &sprites[0]);
    playdate->system->logToConsole("removing sprite %d", now_unused_sprite_index);
    ASSERT(now_unused_sprite_index >= 0 && now_unused_sprite_index < MAX_SPRITE_COUNT);
    int previous_first_free = sprite_free_list[MAX_SPRITE_COUNT];
    sprite_free_list[MAX_SPRITE_COUNT] = now_unused_sprite_index;
    sprite_free_list[now_unused_sprite_index] = previous_first_free;
    if (--internal_sprite_count <= 0) {
        return;
    }
    // need to shift all elements down from ordered_sprites[internal_sprite_count]
    // until we erase the now_unused_sprite_index from ordered_sprites.
    int ordered_check_index = internal_sprite_count;
    int last_ordered_sprite_index = ordered_sprites[ordered_check_index];
    while (last_ordered_sprite_index != now_unused_sprite_index) {
        ASSERT(ordered_check_index > 0);
        int next_ordered_sprite_index = ordered_sprites[ordered_check_index - 1];
        ordered_sprites[ordered_check_index - 1] = last_ordered_sprite_index;
        --ordered_check_index;
        last_ordered_sprite_index = next_ordered_sprite_index;
    }
}

static inline int sprite_area_clamp(
    int *min_over_16,   // start with this bit
    int *max_over_16,   // go up to, but not including this bit
    int start,
    int length,
    int absolute_max_over_16
) {
    // need the floor of start / 16:
    *min_over_16 = start / 16;
    if (*min_over_16 >= absolute_max_over_16) {
        return 0;
    }
    // need the ceiling of (start + length) / 16:
    *max_over_16 = (start + length + 15) / 16;
    if (*max_over_16 <= 0) {
        return 0;
    }
    if (*min_over_16 < 0) {
        *min_over_16 = 0;
    }
    if (*max_over_16 > absolute_max_over_16) {
        *max_over_16 = absolute_max_over_16;
    }
    return 1;
}

static inline void sprite_area_check(const display_sprite_t *display_sprite) {
    int min_x_over_16, max_x_over_16;
    if (!sprite_area_clamp(
        &min_x_over_16,
        &max_x_over_16,
        display_sprite->x,
        display_sprite->width,
        LCD_COLUMNS / 16
    )) {
        return;
    }
    int min_y_over_16, max_y_over_16;
    if (!sprite_area_clamp(
        &min_y_over_16,
        &max_y_over_16,
        display_sprite->y,
        display_sprite->height,
        LCD_ROWS / 16
    )) {
        return;
    }
    for (int y_over_16 = min_y_over_16; y_over_16 < max_y_over_16; ++y_over_16) {
        data_u1s_t u1s;
        data_u1s_initialize(&u1s, y_over_16 * (LCD_COLUMNS / 16) + min_x_over_16);
        data_u1s_fill(&u1s, sprite_redraw_areas, max_x_over_16 - min_x_over_16);
    }
}

static inline void sprite_area_check_all() {
    for (int i = 0; i < internal_sprite_count; ++i) {
        sprite_area_check(&sprites[ordered_sprites[i]].display);
    }
}

void sprite_pre_move_area_check() {
    memset(sprite_redraw_areas, 0, sizeof(sprite_redraw_areas));    
    sprite_area_check_all();
}

const uint8_t *sprite_post_move_area_check() {
    sprite_area_check_all();
    return sprite_redraw_areas;
}

static inline void sprite_sort() {
    qsort(ordered_sprites, internal_sprite_count, sizeof(uint8_t), sprite_compare);
}

void sprite_draw() {
    if (internal_sprite_count == 0) {
        return;
    }
    sprite_sort();
    for (int i = 0; i < internal_sprite_count; ++i) {
        const sprite_t *sprite = sprites + ordered_sprites[i];
        display_sprite_draw(sprite->display);
    }
}

static int sprite_compare(const void *void_index1, const void *void_index2) {
    const uint8_t *index1 = void_index1;
    const uint8_t *index2 = void_index2;
    const sprite_t *sprite_1 = sprites + *index1;
    const sprite_t *sprite_2 = sprites + *index2;
    // display.z is int16_t, so casting to int (32 or 64 bit) will not overflow.
    // Note, we also want to move higher values of z to the front, since they are further away,
    // so they should be drawn first (i.e., nearer sprites can obscure them, not the other way around).
    return (int)(sprite_2->display.z) - (int)(sprite_1->display.z);
}

#ifndef NDEBUG
static void test_setup_sprites_40213() {
    sprite_reset();
    sprite_t *sprite;
    sprite = sprite_add();
    EXPECT_INT_EQUAL((int)(sprite - sprites), 0);
    sprite->display.z = 100;
    sprite = sprite_add();
    EXPECT_INT_EQUAL((int)(sprite - sprites), 1);
    sprite->display.z = -1;
    sprite = sprite_add();
    EXPECT_INT_EQUAL((int)(sprite - sprites), 2);
    sprite->display.z = 30;
    sprite = sprite_add();
    EXPECT_INT_EQUAL((int)(sprite - sprites), 3);
    sprite->display.z = -3;
    sprite = sprite_add();
    EXPECT_INT_EQUAL((int)(sprite - sprites), 4);
    sprite->display.z = 120;

    sprite_sort();

    EXPECT_INT_EQUAL(sprite_count(), 5);
    EXPECT_INT_EQUAL(ordered_sprites[0], 4);
    EXPECT_INT_EQUAL(ordered_sprites[1], 0);
    EXPECT_INT_EQUAL(ordered_sprites[2], 2);
    EXPECT_INT_EQUAL(ordered_sprites[3], 1);
    EXPECT_INT_EQUAL(ordered_sprites[4], 3);
}

void test__library__sprite() {
    TEST(
        sprite_reset();
        sprite_t *sprite;
        sprite = sprite_add();
        EXPECT_INT_EQUAL((int)(sprite - sprites), 0);
        sprite->display.z = 100;
        sprite = sprite_add();
        EXPECT_INT_EQUAL((int)(sprite - sprites), 1);
        sprite->display.z = -1;
        sprite = sprite_add();
        EXPECT_INT_EQUAL((int)(sprite - sprites), 2);
        sprite->display.z = 30;
        EXPECT_INT_EQUAL(sprite_count(), 3);

        sprite_sort();

        // higher-z sprites should be drawn first, since they are further away:
        EXPECT_INT_EQUAL(ordered_sprites[0], 0);
        EXPECT_INT_EQUAL(ordered_sprites[1], 2);
        EXPECT_INT_EQUAL(ordered_sprites[2], 1);

        sprite = sprite_add();
        EXPECT_INT_EQUAL((int)(sprite - sprites), 3);
        sprite->display.z = 200;
        EXPECT_INT_EQUAL(sprite_count(), 4);

        sprite_sort();

        EXPECT_INT_EQUAL(ordered_sprites[0], 3);
        EXPECT_INT_EQUAL(ordered_sprites[1], 0);
        EXPECT_INT_EQUAL(ordered_sprites[2], 2);
        EXPECT_INT_EQUAL(ordered_sprites[3], 1);
    ,   "%s: sprite sorting works",
        AT
    );

    TEST(
        TEST(
            test_setup_sprites_40213();

            int removed_sprite = 3; // sprite 3 is last in sort order
            sprite_remove(sprites + removed_sprite);

            EXPECT_INT_EQUAL(sprite_count(), 4);
            EXPECT_INT_EQUAL(ordered_sprites[0], 4);
            EXPECT_INT_EQUAL(ordered_sprites[1], 0);
            EXPECT_INT_EQUAL(ordered_sprites[2], 2);
            EXPECT_INT_EQUAL(ordered_sprites[3], 1);
            // check that removed sprite gets added back to free list:
            EXPECT_INT_EQUAL(sprite_free_list[MAX_SPRITE_COUNT], removed_sprite);
            EXPECT_INT_EQUAL(sprite_free_list[removed_sprite], 5);
        ,   "%s: removal from last sort position works",
            AT
        );

        TEST(
            test_setup_sprites_40213();
            
            int removed_sprite = 4; // sprite 4 is first in sort order
            sprite_remove(sprites + removed_sprite);

            EXPECT_INT_EQUAL(sprite_count(), 4);
            EXPECT_INT_EQUAL(ordered_sprites[0], 0);
            EXPECT_INT_EQUAL(ordered_sprites[1], 2);
            EXPECT_INT_EQUAL(ordered_sprites[2], 1);
            EXPECT_INT_EQUAL(ordered_sprites[3], 3);
            // check that removed sprite gets added back to free list:
            EXPECT_INT_EQUAL(sprite_free_list[MAX_SPRITE_COUNT], removed_sprite);
            EXPECT_INT_EQUAL(sprite_free_list[removed_sprite], 5);
        ,   "%s: removal from first sort position works",
            AT
        );

        TEST(
            test_setup_sprites_40213();
            
            int removed_sprite = 2; // sprite 2 is in the middle
            sprite_remove(sprites + removed_sprite);

            EXPECT_INT_EQUAL(sprite_count(), 4);
            EXPECT_INT_EQUAL(ordered_sprites[0], 4);
            EXPECT_INT_EQUAL(ordered_sprites[1], 0);
            EXPECT_INT_EQUAL(ordered_sprites[2], 1);
            EXPECT_INT_EQUAL(ordered_sprites[3], 3);
            // check that removed sprite gets added back to free list:
            EXPECT_INT_EQUAL(sprite_free_list[MAX_SPRITE_COUNT], removed_sprite);
            EXPECT_INT_EQUAL(sprite_free_list[removed_sprite], 5);
        ,   "%s: removal from middle sort position works",
            AT
        );

    ,   "%s: sprite removal", AT
    );
}
#endif
