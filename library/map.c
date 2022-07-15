#include "map.h"

void map_set_room_tile(room_t *room, const map_t *map) {
    room->tile.data = map->tiles;
    // see discussion around MAX_MAP_TILES for why we decrement/increment here and below:
    int row_stride = (ROOM_WIDTH_IN_TILES - 1) * map->width + 1;
    room->tile.row_stride = row_stride;
    room->tile.offset =
            row_stride * map->current_room.y
        +   (ROOM_WIDTH_IN_TILES - 1) * map->current_room.x;
}
