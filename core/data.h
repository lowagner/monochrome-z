#pragma once

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

