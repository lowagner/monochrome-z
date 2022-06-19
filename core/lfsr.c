#include "lfsr.h"

void lfsr32_next(uint32_t *lfsr)
{   uint32_t new_bit = 0;
    uint32_t seed = *lfsr;
    if (seed & 0x80000000L) new_bit ^= 1;
    if (seed & 0x01000000L) new_bit ^= 1;
    if (seed & 0x00000040L) new_bit ^= 1;
    if (seed & 0x00000200L) new_bit ^= 1;
    *lfsr = (seed << 1) | new_bit;
}
