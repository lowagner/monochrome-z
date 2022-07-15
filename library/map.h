#pragma once

#include "room.h"

#include <stdint.h>

// between two rooms, we share tiles on the border (so that room doors/walls/open areas don't
// need to coordinate between the two rooms).  this means that map rooms are effectively one less
// tile in each dimension than a standard room, since we decrement the room dimension.
// but maps can choose to a have a different width/height as long as their tile count is less
// than the following:
#define MAX_MAP_TILES ( \
        (16 /* map width */ * (ROOM_WIDTH_IN_TILES - 1) + 1 /* final tile column on east rooms) */) \
    *   (16 /* map height */ * (ROOM_HEIGHT_IN_TILES - 1) + 1 /* final tile row on south rooms) */) \
)

typedef struct map {
    // global offset, in tiles, for where this map should be located.
    struct {
        int16_t x;
        int16_t y;
        // height offset, e.g., for tower floors up (<0) or underground caves/basements (>0).
        // ground level is z = 0.
        int8_t z;
    }
        global_offset;

    // NOTE: [width * (ROOM_WIDTH_IN_TILES - 1) + 1] * [height * (ROOM_HEIGHT_IN_TILES - 1) + 1]
    // should be less than MAX_MAP_TILES
    uint8_t width;
    uint8_t height;
    uint8_t tiles[MAX_MAP_TILES];

    struct {
        // from 0 to width - 1
        uint8_t x;
        // from 0 to height - 1
        uint8_t y;
    }
        current_room;
}
    map_t;

// only used to set room.tile information based on map.room and related fields.
void map_set_room_tile(room_t *room, const map_t *map);
