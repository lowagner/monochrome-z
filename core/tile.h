#pragma once

#include <stdint.h>

enum tile_type {
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


