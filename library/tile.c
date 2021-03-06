#include "tile.h"

#include "../core/data.h"
#include "../core/playdate.h"
#ifndef NDEBUG
#include "../core/error.h"
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
            "error closing loaded tile file %s: %s",
            file_name, playdate->file->geterr()
        );
    } else {
        playdate->system->logToConsole("successfully loaded tile from %s", file_name);
    }
    return 1;
}

int tile_save(const tile_t *from_here, const char *file_name) {
    SDFile* file = playdate->file->open(file_name, kFileWrite);
    if (playdate->file->write(file, &from_here->type, 1) != 1) {
        return 0;
    }
    if (playdate->file->write(file, from_here->data1, 32) != 32) {
        return 0;
    }
    if (playdate->file->close(file)) {
        playdate->system->logToConsole(
            "error closing saved tile file %s: %s",
            file_name, playdate->file->geterr()
        );
    } else {
        playdate->system->logToConsole("successfully saved tile to %s", file_name);
    }
    return 1;
}

#ifndef NDEBUG
void test__library__tile() {
    tile_t test_saved_tile;
    tile_t test_loaded_tile;
    TEST(
        test_saved_tile.type = 140;
        for (int i = 0; i < 32; ++i) {
            test_saved_tile.data1[i] = 64 + 3 * i;
        }
        EXPECT_INT_EQUAL_LOGGED(tile_save(&test_saved_tile, "test-tile-123"), 1);
        EXPECT_INT_EQUAL_LOGGED(tile_load(&test_loaded_tile, "test-tile-123"), 1);
        EXPECT_INT_EQUAL_LOGGED(test_loaded_tile.type, test_saved_tile.type);
        for (int i = 0; i < 32; ++i) {
            EXPECT_INT_EQUAL_LOGGED(test_loaded_tile.data1[i], test_saved_tile.data1[i]);
        },
        "%s: loading after saving", AT
    );
}
#endif
