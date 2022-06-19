#pragma once

#include <stdint.h>

// ensure starting with an seed that is non-zero.
void lfsr32_next(uint32_t *lfsr);
