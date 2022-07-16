#include "data.h"

#ifndef NDEBUG
#include "error.h"
#include <string.h> // memset
#endif

char data_nibble(uint8_t nibble) {
    static const char hex[16] = {
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        'a',
        'b',
        'c',
        'd',
        'e',
        'f',
    };
    return hex[nibble & 15];
}

void data_u1s_initialize(data_u1s_t *u1s, uint32_t offset) {
    u1s->byte_offset = offset / 8;
    u1s->bit_offset = 7 - offset % 8; // we iterate over most-significant-bits first for playdate
}

void data_u1s_increment(data_u1s_t *u1s) {
    if (--u1s->bit_offset > 7) {
        // bit_offset went from 0 to 255
        u1s->bit_offset = 7;
        ++u1s->byte_offset;
    }
}

void data_u1s_set(const data_u1s_t *u1s, uint8_t *data, int value) {
    if (value) {
        data[u1s->byte_offset] |= 1 << u1s->bit_offset;
    } else {
        data[u1s->byte_offset] &= ~(1 << u1s->bit_offset);
    }
}

void data_u1s_fill(data_u1s_t *u1s, uint8_t *data, int length_bits) {
    if (length_bits <= 0) {
        return;
    }
    // if bit_offset == 7, then we're on a byte boundary (delta = 0)
    // if bit_offset == 6, then we're 7 bits away from the byte boundary
    // if bit_offset == 5, then we're 6 bits away from the byte boundary
    // etc., so delta bits is normally 1 more than bit_offset:
    int delta_to_byte_boundary = (u1s->bit_offset + 1) & 7;
    if (delta_to_byte_boundary == 0) {
        continue_filling_bits_from_full_byte:
        // u1s->bit_offset == 7, so we're at a byte boundary and can start filling here:
        int length_bytes = length_bits / 8;
        length_bits = length_bits % 8;
        memset(data + u1s->byte_offset, 255, length_bytes);
        u1s->byte_offset += length_bytes;
        if (length_bits) {
            data[u1s->byte_offset] |= U8_BITMASK_LEFT_BITS(length_bits);
            u1s->bit_offset -= length_bits;
        }
    } else if (length_bits >= delta_to_byte_boundary) {
        data[u1s->byte_offset] |= U8_BITMASK_RIGHT_BITS(delta_to_byte_boundary);
        ++u1s->byte_offset;
        u1s->bit_offset = 7;
        length_bits -= delta_to_byte_boundary;
        if (length_bits) {
            goto continue_filling_bits_from_full_byte;
        }
    } else {
        // if bit_offset == 6, we want to start at bit 1 until 1 + length_bits
        // if bit_offset == 5, we want to start at bit 2 until 2 + length_bits
        // etc. notice that the starting bit is exactly (7 - bit_offset)
        // and that delta_to_byte_boundary + length_bits < 7 since
        // [delta = (bit_offset + 1) % 8] + 
        // suppose length_bits = 6 and delta_to_byte_boundary = 7
        int left_bits = (7 - u1s->bit_offset) + length_bits;
        ASSERT(left_bits <= 8);
        data[u1s->byte_offset] |= (
                U8_BITMASK_LEFT_BITS(left_bits)
            &   U8_BITMASK_RIGHT_BITS(delta_to_byte_boundary)
        );
        u1s->bit_offset -= length_bits;
    }
}

int data_u1s_get(const data_u1s_t *u1s, const uint8_t *data) {
    return (data[u1s->byte_offset] >> u1s->bit_offset) & 1;
}

void data_u1s_flip(const data_u1s_t *u1s, uint8_t *data) {
    data[u1s->byte_offset] ^= 1 << u1s->bit_offset;
}

void data_u2s_initialize(data_u2s_t *u2s, uint32_t offset) {
    u2s->byte_offset = offset / 4;
    u2s->bit_offset = 6 - 2 * (offset % 4); // we iterate over most-significant-bits first
}

void data_u2s_increment(data_u2s_t *u2s) {
    u2s->bit_offset -= 2;
    if (u2s->bit_offset > 6) {
        // bit_offset went from 0 to 254
        u2s->bit_offset = 6;
        ++u2s->byte_offset;
    }
}

void data_u2s_set(const data_u2s_t *u2s, uint8_t *data, int value) {
    data[u2s->byte_offset] = (
            ( data[u2s->byte_offset] & ~(3 << u2s->bit_offset) )
        |   ( (value & 3) << u2s->bit_offset )
    );
}

int data_u2s_get(const data_u2s_t *u2s, const uint8_t *data) {
    return (data[u2s->byte_offset] >> u2s->bit_offset) & 3;
}

#ifndef NDEBUG
void test__core__data() {
    data_u1s_t test_u1s;
    data_u2s_t test_u2s;
    uint8_t test_data[256];

    TEST(
        TEST(
            data_u1s_initialize(&test_u1s, 0);
            EXPECT_INT_EQUAL_LOGGED(test_u1s.byte_offset, 0);
            EXPECT_INT_EQUAL_LOGGED(test_u1s.bit_offset, 7);

            data_u1s_initialize(&test_u1s, 23);
            EXPECT_INT_EQUAL_LOGGED(test_u1s.byte_offset, 2);
            EXPECT_INT_EQUAL_LOGGED(test_u1s.bit_offset, 0);

            data_u1s_initialize(&test_u1s, 58);
            EXPECT_INT_EQUAL_LOGGED(test_u1s.byte_offset, 7);
            EXPECT_INT_EQUAL_LOGGED(test_u1s.bit_offset, 5),
            "%s: u1s initializes correct with offsets",
            AT
        );

        TEST(
            data_u1s_initialize(&test_u1s, 0);
            for (int i = 0; i < 256 * 8; ++i) {
                data_u1s_set(&test_u1s, test_data, 0);
                data_u1s_increment(&test_u1s);
            }
            for (int i = 0; i < 256; ++i) {
                TEST(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], 0),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: u1s can set a bunch of values to 0",
            AT
        );

        TEST(
            data_u1s_initialize(&test_u1s, 0);
            for (int i = 0; i < 256 * 8; ++i) {
                data_u1s_set(&test_u1s, test_data, 1);
                data_u1s_increment(&test_u1s);
            }
            for (int i = 0; i < 256; ++i) {
                TEST(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], 255),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: u1s can set a bunch of values to 1",
            AT
        );

        TEST_LOGGED(
            memset(test_data, 127, 256);
            data_u1s_initialize(&test_u1s, 0);
            for (int i = 0; i < 256 * 8; ++i) {
                if (i % 8 == 3) {
                    data_u1s_flip(&test_u1s, test_data);
                }
                data_u1s_increment(&test_u1s);
            }
            for (int i = 0; i < 256; ++i) {
                TEST_LOGGED(
                    EXPECT_INT_EQUAL_LOGGED(test_data[i], 111),
                    "%s: at byte offset %d",
                    AT, i
                );
            },
            "%s: u1s can flip bits",
            AT
        );

        TEST_LOGGED(
            memset(test_data, 0, 256);
            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 5);
                data_u1s_fill(&test_u1s, test_data, 3);
                EXPECT_INT_EQUAL(test_data[0], 7);
                EXPECT_INT_EQUAL(test_data[1], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 1);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 7);
            ,   "%s: u1s can fill bits to a boundary",
                AT
            );

            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 2*8 + 1);
                data_u1s_fill(&test_u1s, test_data, 4);
                EXPECT_INT_EQUAL(test_data[1], 0);
                EXPECT_INT_EQUAL(test_data[2], 64 | 32 | 16 | 8);
                EXPECT_INT_EQUAL(test_data[3], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 2);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 2);
            ,   "%s: u1s can fill bits within a byte",
                AT
            );

            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 4*8 + 7);
                // single bit at end of byte boundary:
                data_u1s_fill(&test_u1s, test_data, 1);
                EXPECT_INT_EQUAL(test_data[3], 0);
                EXPECT_INT_EQUAL(test_data[4], 1);
                EXPECT_INT_EQUAL(test_data[5], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 5);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 7);

                // single bit at start of (next) byte boundary:
                data_u1s_fill(&test_u1s, test_data, 1);
                EXPECT_INT_EQUAL(test_data[5], 128);
                EXPECT_INT_EQUAL(test_data[6], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 5);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 6);
            ,   "%s: u1s can fill single bit within a byte",
                AT
            );

            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 9*8);
                data_u1s_fill(&test_u1s, test_data, 24);
                EXPECT_INT_EQUAL(test_data[8], 0);
                EXPECT_INT_EQUAL(test_data[9], 255);
                EXPECT_INT_EQUAL(test_data[10], 255);
                EXPECT_INT_EQUAL(test_data[11], 255);
                EXPECT_INT_EQUAL(test_data[12], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 12);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 7);
            ,   "%s: u1s can fill bytes worth of data, starting on byte boundary",
                AT
            );

            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 13*8);
                data_u1s_fill(&test_u1s, test_data, 2*8 + 2);
                EXPECT_INT_EQUAL(test_data[12], 0);
                EXPECT_INT_EQUAL(test_data[13], 255);
                EXPECT_INT_EQUAL(test_data[14], 255);
                EXPECT_INT_EQUAL(test_data[15], 128 | 64);
                EXPECT_INT_EQUAL(test_data[16], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 15);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 5);
            ,   "%s: u1s can fill bytes worth of data + bits, starting on byte boundary",
                AT
            );

            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 17*8 + 3);
                data_u1s_fill(&test_u1s, test_data, 5*8 + 5 + 6);
                EXPECT_INT_EQUAL(test_data[16], 0);
                EXPECT_INT_EQUAL(test_data[17], 1 | 2 | 4 | 8 | 16); // 5 bits flipped here
                EXPECT_INT_EQUAL(test_data[18], 255);
                EXPECT_INT_EQUAL(test_data[19], 255);
                EXPECT_INT_EQUAL(test_data[20], 255);
                EXPECT_INT_EQUAL(test_data[21], 255);
                EXPECT_INT_EQUAL(test_data[22], 255);
                EXPECT_INT_EQUAL(test_data[23], 128 | 64 | 32 | 16 | 8 | 4); // 6 bits flipped
                EXPECT_INT_EQUAL(test_data[24], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 23);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 1);
            ,   "%s: u1s can fill bytes worth of data + bits, crossing byte boundaries",
                AT
            );

            TEST_LOGGED(
                data_u1s_initialize(&test_u1s, 25*8 + 4);
                data_u1s_fill(&test_u1s, test_data, 2*8 + 4);
                EXPECT_INT_EQUAL(test_data[24], 0);
                EXPECT_INT_EQUAL(test_data[25], 1 | 2 | 4 | 8);
                EXPECT_INT_EQUAL(test_data[26], 255);
                EXPECT_INT_EQUAL(test_data[27], 255);
                EXPECT_INT_EQUAL(test_data[28], 0);
                EXPECT_INT_EQUAL(test_u1s.byte_offset, 28);
                EXPECT_INT_EQUAL(test_u1s.bit_offset, 7);
            ,   "%s: u1s can fill bytes worth of data + bits, ending on byte boundary",
                AT
            );
        ,   "%s: u1s can fill bits",
            AT
        );
    ,   "%s: u1s", AT
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
