#pragma once

#include <stdint.h>

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

// TODO: this probably should be in an iterator.h/c file as `iterator_booleans`
typedef struct data_booleans {
    uint32_t byte_offset;
    // should be between 0 and 7.
    uint8_t bit_offset;
}
    data_booleans;

void data_booleans_initialize(data_booleans *booleans, uint32_t offset);
void data_booleans_increment(data_booleans *booleans);
// sets the data at the current booleans offset to 0 or 1, depending on
// whether the value is 0 or non-zero.
void data_booleans_set(const data_booleans *booleans, uint8_t *data, int value);
// returns a 0 or a 1, depending on whether the bit is not set or set.
int data_booleans_get(const data_booleans *booleans, const uint8_t *data);
// flips the bit specified by booleans in data, i.e., from 0 to 1, or from 1 to 0.
int data_booleans_flip(const data_booleans *booleans, uint8_t *data);

// TODO: also should probably be in iterator.h/c as iterator_u2s
typedef struct data_u2s {
    uint32_t byte_offset;
    // should be 0, 2, 4, or 6.
    uint8_t bit_offset;
}
    data_u2s;

void data_u2s_initialize(data_u2s *u2s, uint32_t offset);
void data_u2s_increment(data_u2s *u2s);
// sets the data at the current u2s offset to 0, 1, 2, or 3, by taking `value & 3`.
void data_u2s_set(const data_u2s *u2s, uint8_t *data, int value);
// returns a number from 0 to 3, depending on what the value at data is.
int data_u2s_get(const data_u2s *u2s, const uint8_t *data);
