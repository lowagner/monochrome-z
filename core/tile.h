#pragma once

#include <stdint.h>

enum tile_type_t {
    kTileTypePassable,
    kTileTypeImpassable,
    /* TODO
    kTileTypeLadderNorth,
    kTileTypeDoorwayNorth,
    kTileTypeHole,
    kTileTypeDamage,
    kTileTypeDamageNorth,
    kTileTypeDamageSouth,
    kTileTypeDamageEast,
    kTileTypeDamageWest,
    kTileTypeClimableNorth,
    kTileTypeClimableSouth,
    kTileTypeClimableEast,
    kTileTypeClimableWest,
    kTileTypePassableNorth,
    kTileTypePassableSouth,
    kTileTypePassableEast,
    kTileTypePassableWest,
    */
};

typedef struct tile {
    uint8_t index;
    uint8_t type;
    uint8_t data1[32];
}
    tile_t;

extern tile_t tiles[256];

// returns 0 if not loaded, 1 if loaded.
int tile_load(tile_t *load_here, const char *file_name);

// returns 0 if not written, 1 if written.
int tile_write(tile_t *from_here, const char *file_name);

int tile_type(uint8_t *tile_data1);
