#include "tile-editor.h"

#include "../core/buttons.h"
#include "../core/display.h"
#include "../core/runtime.h"

#include <string.h> // memcpy

tile_editor_t tile_editor = {
    .tile = {
        // mostly ignored for tile_editor:
        .index = 0,
        .type = 0,
        .data1 = {0},
    },
    .drawing = {
        .color = 1,
        .cursor_x = 0,
        .cursor_y = 0,
    },
};

const char *tile_editor_tiles[] = {
    "000", "001", "002", "003", "004", "005", "006", "007", "008", "009",
    "010", "011", "012", "013", "014", "015", "016", "017", "018", "019",
    "020", "021", "022", "023", "024", "025", "026", "027", "028", "029",
    "030", "031", "032", "033", "034", "035", "036", "037", "038", "039",
    "040", "041", "042", "043", "044", "045", "046", "047", "048", "049",
    "050", "051", "052", "053", "054", "055", "056", "057", "058", "059",
    "060", "061", "062", "063", "064", "065", "066", "067", "068", "069",
    "070", "071", "072", "073", "074", "075", "076", "077", "078", "079",
    "080", "081", "082", "083", "084", "085", "086", "087", "088", "089",
    "090", "091", "092", "093", "094", "095", "096", "097", "098", "099",
    "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
    "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
    "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
    "130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
    "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
    "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
    "160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
    "170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
    "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
    "190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
    "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
    "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
    "220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
    "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
    "240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
    "250", "251", "252", "253", "254", "255",
};

void tile_editor_tile_set_value(int index) {
    tile_editor.tile.index = index;
}

int tile_editor_tile_get_index() {
    return tile_editor.tile.index;
}

runtime_menu_t tile_editor_save_menu = {
    .pd_menu = NULL,
    .title = "tile",
    .options = tile_editor_tiles,
    .option_count = 256,
    .set_value_from_index = tile_editor_tile_set_value,
    .get_index_from_value = tile_editor_tile_get_index,
};

const char *tile_editor_actions[] = {
    "none",
    "save",
    "load",
};

void tile_editor_action_set_value(int action_index) {
    if (action_index == 0) {
        return;
    }
    const char *tile_name = tile_editor_tiles[tile_editor.tile.index];
    char tile_file_name[9] = {
        tile_name[0], tile_name[1], tile_name[2],
        '.', 't', 'i', 'l', 'e',
        0
    };
    int success = action_index == 1
        ?   tile_save(&tile_editor.tile, tile_file_name)
        :   tile_load(&tile_editor.tile, tile_file_name);
    if (!success) {
        playdate->system->logToConsole("could not load/save %s", tile_file_name);
    }
}

int tile_editor_action_get_index() {
    // always default to "none" so we don't save/load accidentally.
    return 0;
}

runtime_menu_t tile_editor_action_menu = {
    .pd_menu = NULL,
    .title = "action",
    .options = tile_editor_actions,
    .option_count = 3,
    .set_value_from_index = tile_editor_action_set_value,
    .get_index_from_value = tile_editor_action_get_index,
};

void tile_editor_update(display_slice_t slice) {
    if (runtime.transition.counter || runtime.transition.next_mode != kRuntimeModeTileEditor) {
        display_slice_fill(255, slice);
        return;
    }
    if (tile_editor_save_menu.pd_menu == NULL) {
        display_slice_fill(0, slice);
        runtime_add_menu(&tile_editor_save_menu);
        runtime_add_menu(&tile_editor_action_menu);
    }
}
