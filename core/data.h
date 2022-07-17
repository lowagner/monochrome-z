#pragma once

#include <stdint.h>

// here, x is the number of bits you want flipped on, "from the left" (most significant bits):
#define U8_BITMASK_LEFT_BITS(x) (-(1 << (8 - (x))))
// here, x is the number of bits you want flipped on, "from the right" (least significant bits):
#define U8_BITMASK_RIGHT_BITS(x) ((1 << (x)) - 1)

#define DATA3(type1, name1, init1, type2, name2, init2, type3, name3, init3, struct_name) \
    struct struct_name { \
        type1 name1; \
        type2 name2; \
        type3 name3; \
    } \
    struct_name = { \
        .name1 = init1, \
        .name2 = init2, \
        .name3 = init3, \
    }

#define DATA4(type1, name1, init1, type2, name2, init2, type3, name3, init3, type4, name4, init4, struct_name) \
    struct struct_name { \
        type1 name1; \
        type2 name2; \
        type3 name3; \
        type4 name4; \
    } \
    struct_name = { \
        .name1 = init1, \
        .name2 = init2, \
        .name3 = init3, \
        .name4 = init4, \
    }

char data_nibble(uint8_t nibble);

typedef struct data_u1s {
    uint32_t byte_offset;
    // should be between 0 and 7.
    uint8_t bit_offset;
}
    data_u1s_t;

void data_u1s_initialize(data_u1s_t *u1s, uint32_t offset);
void data_u1s_increment(data_u1s_t *u1s);
// sets the data at the current u1s offset to 0 or 1, depending on
// whether the value is 0 or non-zero, without increment
void data_u1s_set(const data_u1s_t *u1s, uint8_t *data, int value);
// sets the data at the current u1s offset to 0 or 1, then increments.
void data_u1s_set_and_increment(data_u1s_t *u1s, uint8_t *data, int value);
// sets data bits to 1 for an extended length starting at u1s value.
// will increment u1s max(0, length_bits) times.
void data_u1s_fill(data_u1s_t *u1s, uint8_t *data, int length_bits);
// returns a 0 or a 1, depending on whether the bit is not set or set, without increment.
int data_u1s_get(const data_u1s_t *u1s, const uint8_t *data);
// returns a 0 or a 1, then increments.
int data_u1s_get_and_increment(data_u1s_t *u1s, const uint8_t *data);
// flips the bit specified by u1s in data, i.e., from 0 to 1, or from 1 to 0
void data_u1s_flip(const data_u1s_t *u1s, uint8_t *data);
// flips and increments.
void data_u1s_flip_and_increment(data_u1s_t *u1s, uint8_t *data);

typedef struct data_u2s {
    uint32_t byte_offset;
    // should be 0, 2, 4, or 6.
    uint8_t bit_offset;
}
    data_u2s_t;

void data_u2s_initialize(data_u2s_t *u2s, uint32_t offset);
void data_u2s_increment(data_u2s_t *u2s);
// sets the data at the current u2s offset to 0, 1, 2, or 3, by taking `value & 3`.
void data_u2s_set(const data_u2s_t *u2s, uint8_t *data, int value);
// sets the data at the current u2s offset, then increments.
void data_u2s_set_and_increment(data_u2s_t *u2s, uint8_t *data, int value);
// returns a number from 0 to 3, depending on what the value at data is.
int data_u2s_get(const data_u2s_t *u2s, const uint8_t *data);
// returns a number from 0 to 3, and increments.
int data_u2s_get_and_increment(data_u2s_t *u2s, const uint8_t *data);
