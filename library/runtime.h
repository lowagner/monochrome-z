#pragma once

struct runtime {
    // readonly `int update(void *)` function pointer:
    int (*const update)(void *);
    int mode;
};

extern struct runtime runtime;

// TODO: add enum for different runtime modes

// You should define this in your main.c (or elsewhere).
int default_update();
