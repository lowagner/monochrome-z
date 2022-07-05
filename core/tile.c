#include "tile.h"

#include "data.h"
#include "playdate.h"
#ifndef NDEBUG
#include "error.h"
#endif

#include <string.h> // memcpy

tile_t tiles[256];

int tile_load(tile_t *load_here, const char *file_name) {
    SDFile* file = playdate->file->open(file_name, kFileRead | kFileReadData);
    uint8_t buffer[33];
    int read_bytes = playdate->file->read(file, buffer, 33);
    if (read_bytes != 33) {
        return 0;
    }
    load_here->type = *buffer;
    memcpy(load_here->data1, buffer + 1, 32);
    if (playdate->file->close(file)) {
        playdate->system->logToConsole(
            "error closing read file %s: %s",
            file_name, playdate->file->geterr()
        );
    }
    return 1;
}

int tile_write(tile_t *from_here, const char *file_name) {
    SDFile* file = playdate->file->open(file_name, kFileWrite);
    if (playdate->file->write(file, &from_here->type, 1) != 1) {
        return 0;
    }
    if (playdate->file->write(file, from_here->data1, 32) != 32) {
        return 0;
    }
    if (playdate->file->close(file)) {
        playdate->system->logToConsole(
            "error closing written file %s: %s",
            file_name, playdate->file->geterr()
        );
    }
    return 1;
}

#ifndef NDEBUG
void test__core__tile() {
    tile_t test_written_tile;
    tile_t test_loaded_tile;
    TEST(
        test_written_tile.type = 140;
        for (int i = 0; i < 32; ++i) {
            test_written_tile.data1[i] = 64 + 3 * i;
        }
        EXPECT_INT_EQUAL_LOGGED(tile_write(&test_written_tile, "test-tile-123"), 1);
        EXPECT_INT_EQUAL_LOGGED(tile_load(&test_loaded_tile, "test-tile-123"), 1);
        EXPECT_INT_EQUAL_LOGGED(test_loaded_tile.type, test_written_tile.type);
        for (int i = 0; i < 32; ++i) {
            EXPECT_INT_EQUAL_LOGGED(test_loaded_tile.data1[i], 64 + 3 * i);
        },
        "%s: loading after saving", AT
    );
}
#endif
