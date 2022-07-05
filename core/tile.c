#include "tile.h"

#include "data.h"
#include "playdate.h"

tile_t tiles[256];

int tile_load(tile_t *load_here, const char *file_name) {
    SDFile* file = playdate->file->open(file_name, kFileRead | kFileReadData);
    uint8_t buffer[64];
    int read_bytes = playdate->file->read(file, buffer, 64);
    if (read_bytes != 64) {
        return 0;
    }
    // TODO
}

int tile_write(tile_t *from_here, const char *file_name) {
    SDFile* file = playdate->file->open(file_name, kFileWrite);
    // TODO
}

int tile_type(uint8_t *tile_data1) {
    data_u1s_t u1s;
    // TODO
    return 0; 
}
