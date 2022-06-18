#pragma once

struct font {
    const char *path;
    int height;
    int width;
};

extern struct font font;

void font_load(const char *path);
