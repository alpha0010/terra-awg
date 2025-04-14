#ifndef TILECOLOR_H
#define TILECOLOR_H

#include <cstdint>

class World;

const uint8_t *getTileColor(int x, int y, World &world);

#endif // TILECOLOR_H
