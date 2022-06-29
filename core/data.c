#include "data.h"

#ifndef NDEBUG
#include "error.h"
#include <string.h> // memset
#endif

void data_booleans_initialize(data_booleans *booleans, uint32_t offset) {
    booleans->byte_offset = offset / 8;
    booleans->bit_offset = 7 - offset % 8; // we iterate over most-significant-bits first for playdate
}

void data_booleans_increment(data_booleans *booleans) {
    if (--booleans->bit_offset > 7) {
        // bit_offset went from 0 to 255
        booleans->bit_offset = 7;
        ++booleans->byte_offset;
    }
}

void data_booleans_set(const data_booleans *booleans, uint8_t *data, int value) {
    if (value) {
        data[booleans->byte_offset] |= 1 << booleans->bit_offset;
    } else {
        data[booleans->byte_offset] &= ~(1 << booleans->bit_offset);
    }
}

int data_booleans_get(const data_booleans *booleans, const uint8_t *data) {
    return (data[booleans->byte_offset] >> booleans->bit_offset) & 1;
}

int data_booleans_flip(const data_booleans *booleans, uint8_t *data) {
    data[booleans->byte_offset] ^= 1 << booleans->bit_offset;
}

void data_u2s_initialize(data_u2s *u2s, uint32_t offset) {
    u2s->byte_offset = offset / 4;
    u2s->bit_offset = 6 - 2 * (offset % 4); // we iterate over most-significant-bits first
}

void data_u2s_increment(data_u2s *u2s) {
    u2s->bit_offset -= 2;
    if (u2s->bit_offset > 6) {
        // bit_offset went from 0 to 254
        u2s->bit_offset = 6;
        ++u2s->byte_offset;
    }
}

void data_u2s_set(const data_u2s *u2s, uint8_t *data, int value) {
    data[u2s->byte_offset] = (
            ( data[u2s->byte_offset] & ~(3 << u2s->bit_offset) )
        |   ( (value & 3) << u2s->bit_offset )
    );
}

int data_u2s_get(const data_u2s *u2s, const uint8_t *data) {
    return (data[u2s->byte_offset] >> u2s->bit_offset) & 3;
}

#ifndef NDEBUG
void test__core__data() {
    data_booleans test_booleans;
    data_u2s test_u2s;
    uint8_t test_data[256];

    TEST(
        TEST(
            data_booleans_initialize(&test_booleans, 0);
            EXPECT_INT_EQUAL_LOGGED(test_booleans.byte_offset, 0);
            EXPECT_INT_EQUAL_LOGGED(test_booleans.bit_offset, 7);

            data_booleans_initialize(&test_booleans, 23);
            EXPECT_INT_EQUAL_LOGGED(test_booleans.byte_offset, 2);
            EXPECT_INT_EQUAL_LOGGED(test_booleans.bit_offset, 0);

            data_booleans_initialize(&test_booleans, 58);
            EXPECT_INT_EQUAL_LOGGED(test_booleans.byte_offset, 7);
            EXPECT_INT_EQUAL_LOGGED(test_booleans.bit_offset, 5),
            "%s: booleans initializes correct with offsets",
            AT
        );

        TEST(
            data_booleans_initialize(&test_booleans, 0);
            for (int i = 0; i < 256 * 8; ++i) {
                data_booleans_set(&test_booleans, test_data, 0);
                data_booleans_increment(&test_booleans);
            }
            for (int i = 0; i < 256; ++i) {
                TEST(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], 0),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: booleans can set a bunch of values to 0",
            AT
        );

        TEST(
            data_booleans_initialize(&test_booleans, 0);
            for (int i = 0; i < 256 * 8; ++i) {
                data_booleans_set(&test_booleans, test_data, 1);
                data_booleans_increment(&test_booleans);
            }
            for (int i = 0; i < 256; ++i) {
                TEST(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], 255),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: booleans can set a bunch of values to 1",
            AT
        );

        TEST_LOGGED(
            memset(test_data, 127, 256);
            data_booleans_initialize(&test_booleans, 0);
            for (int i = 0; i < 256 * 8; ++i) {
                if (i % 8 == 3) {
                    data_booleans_flip(&test_booleans, test_data);
                }
                data_booleans_increment(&test_booleans);
            }
            for (int i = 0; i < 256; ++i) {
                TEST_LOGGED(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], 111),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: booleans can flip bits",
            AT
        ),
        "%s: booleans", AT
    );

    TEST_LOGGED(
        TEST_LOGGED(
            data_u2s_initialize(&test_u2s, 0);
            EXPECT_INT_EQUAL_LOGGED(test_u2s.byte_offset, 0);
            EXPECT_INT_EQUAL_LOGGED(test_u2s.bit_offset, 6);

            data_u2s_initialize(&test_u2s, 23);
            EXPECT_INT_EQUAL_LOGGED(test_u2s.byte_offset, 5);
            EXPECT_INT_EQUAL_LOGGED(test_u2s.bit_offset, 0);

            data_u2s_initialize(&test_u2s, 58);
            EXPECT_INT_EQUAL_LOGGED(test_u2s.byte_offset, 14);
            EXPECT_INT_EQUAL_LOGGED(test_u2s.bit_offset, 2),
            "%s: u2s initializes correct with offsets",
            AT
        );

        TEST_LOGGED(
            data_u2s_initialize(&test_u2s, 0);
            for (int i = 0; i < 256; ++i) {
                data_u2s_set(&test_u2s, test_data, 2);
                data_u2s_increment(&test_u2s);
                data_u2s_set(&test_u2s, test_data, 1);
                data_u2s_increment(&test_u2s);
                data_u2s_set(&test_u2s, test_data, 3);
                data_u2s_increment(&test_u2s);
                data_u2s_set(&test_u2s, test_data, 0);
                data_u2s_increment(&test_u2s);
            }
            for (int i = 0; i < 256; ++i) {
                TEST_LOGGED(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], (2 << 6) | (1 << 4) | (3 << 2)),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: u2s can set a bunch of values",
            AT
        ),
        "%s: u2s", AT
    );
}
#endif
