#include "playdate.h"

PlaydateAPI *playdate = NULL;

void playdate_init(PlaydateAPI *_playdate, PDCallbackFunction *update_callback) {
    playdate = _playdate;

    // Set an update callback to indicate the game is pure C (no Lua):
    playdate->system->setUpdateCallback(update_callback, playdate);
}
