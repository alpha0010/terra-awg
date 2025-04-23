#ifndef TREASURE_H
#define TREASURE_H

#include "structures/StructureUtil.h"

class World;
class Random;

LocationBins genTreasure(Random &rnd, World &world);

#endif // TREASURE_H
