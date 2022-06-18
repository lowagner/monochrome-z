#include "playdate.h"

#include "runtime.h"

PlaydateAPI *playdate = NULL;

void playdate_init(PlaydateAPI *_playdate) {
    playdate = _playdate;

    // Set an update callback to indicate the game is pure C (no Lua):
    playdate->system->setUpdateCallback(runtime.update, playdate);
}
