#include "HashProspector.h"

// https://github.com/skeeto/hash-prospector/issues/19
uint32_t lowbias32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0x735a2d97;
    x ^= x >> 15;
    return x;
}
