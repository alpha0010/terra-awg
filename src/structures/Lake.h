#ifndef LAKE_H
#define LAKE_H

#include "Tile.h"

class Random;
class World;

void genLake(Random &rnd, World &world);
void convertLiquid(
    int startX,
    int startY,
    Liquid from,
    Liquid to,
    World &world);

#endif // LAKE_H
