#pragma once

#include "pd_api.h"

extern PlaydateAPI *playdate;

void playdate_init(PlaydateAPI *_playdate, PDCallbackFunction *update_callback);
