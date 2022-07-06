#include "tile-editor.h"

#include "../core/buttons.h"
#include "../core/data.h"
#include "../core/display.h"
#include "../core/runtime.h"

#include <string.h> // memcpy

static void tile_editor_draw_big_pixel(int x, int y);
static void tile_editor_color_pixel(int x, int y);
static void tile_editor_change_brush_color();
static void tile_editor_draw_brush_color();
static void tile_editor_draw_field();

enum tile_editor_color_t {
    TileEditorColorClear = 0,
    TileEditorColorBlack = 1,
    TileEditorColorInvert,
    TileEditorColorCheckerboardOdd,
    TileEditorColorCheckerboardEven,
    // do not actually use
    TileEditorColorSentinel,
};

tile_editor_t tile_editor = {
    .tile = {
        // mostly ignored for tile_editor:
        .index = 0,
        .type = 0,
        .data1 = {0},
    },
    .initialization = 16,
    .drawing = {
        .color = TileEditorColorBlack,
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
    if (success) {
        // TODO: redraw screen
    } else {
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
        tile_editor.initialization = 16,
        tile_editor_draw_brush_color();
        tile_editor_draw_field();
        runtime_add_menu(&tile_editor_save_menu);
        runtime_add_menu(&tile_editor_action_menu);
    } else if (tile_editor.initialization > 0) {
        int tile_row = --tile_editor.initialization;
        for (int x = 0; x < 16; ++x) {
            tile_editor_draw_big_pixel(x, tile_row);
        }
    } else {
        if (buttons.pushed & kButtonB) {
            tile_editor_change_brush_color();
        }
        uint8_t previous_x = tile_editor.drawing.cursor_x;
        uint8_t previous_y = tile_editor.drawing.cursor_y;
        tile_editor.drawing.cursor_x = (
                tile_editor.drawing.cursor_x + (
                        ( (buttons.pushed & kButtonLeft) ? 15 : 0 )
                    +   ( (buttons.pushed & kButtonRight) ? 1 : 0 )
                )
        ) % 16;
        tile_editor.drawing.cursor_y = (
                tile_editor.drawing.cursor_y + (
                        ( (buttons.pushed & kButtonUp) ? 15 : 0 )
                    +   ( (buttons.pushed & kButtonDown) ? 1 : 0 )
                )
        ) % 16;
        if (
                tile_editor.drawing.cursor_x != previous_x
            ||  tile_editor.drawing.cursor_y != previous_y
        ) {
            playdate->system->logToConsole(
                "cursor to (%d, %d)",
                tile_editor.drawing.cursor_x, tile_editor.drawing.cursor_y
            );
            // redraw the previous pixel to remove the cursor:
            tile_editor_draw_big_pixel(previous_x, previous_y);
            if (buttons.current & kButtonA) {
                // this also redraws the pixel:
                tile_editor_color_pixel(tile_editor.drawing.cursor_x, tile_editor.drawing.cursor_y);
            } else {
                // redraw the pixel to show the cursor:
                tile_editor_draw_big_pixel(tile_editor.drawing.cursor_x, tile_editor.drawing.cursor_y);
            }
        } else if (buttons.pushed & kButtonA) {
            tile_editor_color_pixel(tile_editor.drawing.cursor_x, tile_editor.drawing.cursor_y);
        }
    }
}

static void tile_editor_draw_big_pixel(int x, int y) {
    playdate->system->logToConsole("drawing tile (%d, %d)", x, y);
    data_u1s_t u1s;
    data_u1s_initialize(&u1s, 16 * y + x);
    uint8_t fill_value = 255 * data_u1s_get(&u1s, tile_editor.tile.data1);
    display_box_fill(fill_value, $(display_box){
        .start_x = 13 * (x + 1),
        .end_x = 13 * (x + 1) + 12,
        .start_y = 13 * (y + 1),
        .end_y = 13 * (y + 1) + 12,
    });
    if (
            x == tile_editor.drawing.cursor_x
        &&  y == tile_editor.drawing.cursor_y
    ) {
        display_box_fill(~fill_value, $(display_box){
            .start_x = 13 * (x + 1) + 4,
            .end_x = 13 * (x + 1) + 8,
            .start_y = 13 * (y + 1) + 4,
            .end_y = 13 * (y + 1) + 8,
        });
    }
}

static void tile_editor_color_pixel(int x, int y) {
    data_u1s_t u1s;
    data_u1s_initialize(&u1s, y * 16 + x);
    switch (tile_editor.drawing.color) {
        case TileEditorColorInvert:
            data_u1s_flip(&u1s, tile_editor.tile.data1);
            break;
        case TileEditorColorCheckerboardOdd:
            data_u1s_set(&u1s, tile_editor.tile.data1, (x + y) % 2);
            break;
        case TileEditorColorCheckerboardEven:
            data_u1s_set(&u1s, tile_editor.tile.data1, 1 - (x + y) % 2);
            break;
        default:
            data_u1s_set(&u1s, tile_editor.tile.data1, tile_editor.drawing.color);
            break;
    }
    tile_editor_draw_big_pixel(x, y);
    tile_editor_draw_field();
}

static void tile_editor_change_brush_color() {
    tile_editor.drawing.color = (tile_editor.drawing.color + 1) % TileEditorColorSentinel;
    tile_editor_draw_brush_color();
}

static void tile_editor_draw_brush_color() {
    static const uint8_t checkerboard_colors[6] = {
        204, 204, 51, 51, 204, 204,
    };
    display_box_t brush_box = {
        .start_x = LCD_COLUMNS - 16,
        .end_x = LCD_COLUMNS - 4,
        .start_y = LCD_ROWS - 16,
        .end_y = LCD_ROWS - 4,
    };
    switch (tile_editor.drawing.color) {
        case TileEditorColorInvert:
            display_box_fill_alternating(85, 170, brush_box);
            break;
        case TileEditorColorCheckerboardOdd:
            display_box_fill_multicolor(4, checkerboard_colors + 2, brush_box);
            break;
        case TileEditorColorCheckerboardEven:
            display_box_fill_multicolor(4, checkerboard_colors + 0, brush_box);
            break;
        default:
            display_box_fill(255 * tile_editor.drawing.color, brush_box);
            break;
    }
}

static void tile_editor_draw_field() {
    for (int row = 0; row < 3; ++row)
    for (int column = 0; column < 3; ++column) {
        display_tile_draw($(display_tile) {
            .data1 = tile_editor.tile.data1,
            .x_over_8 = (LCD_COLUMNS - 4 * 16 + column * 16) / 8,
            .y = 16 + 16 * row,
        });
    }
}
