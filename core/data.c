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
#ifndef NDEBUG
void test__core__data() {
    data_booleans test_booleans;
    uint8_t test_data[256];

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

    TEST(
        memset(test_data, 127, 256);
        data_booleans_initialize(&test_booleans, 0);
        for (int i = 0; i < 256 * 8; ++i) {
            if (i % 8 == 3) {
                data_booleans_flip(&test_booleans, test_data);
            }
            data_booleans_increment(&test_booleans);
        }
        for (int i = 0; i < 256; ++i) {
            TEST(
                EXPECT_INT_EQUAL_LOGGED(test_data[i], 111),
                "%s: at byte offset %d",
                AT, i
            );
        },
        "%s: booleans can flip bits",
        AT
    );
}
#endif
