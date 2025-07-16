#ifndef TRAPS_H
#define TRAPS_H

#include "structures/StructureUtil.h"

class World;
class Random;

void addChestTraps(int x, int y, Random &rnd, World &world);
void maybeAddChestPressureTraps(int x, int y, Random &rnd, World &world);
void addOreTraps(std::vector<Point> &&locations, Random &rnd, World &world);
void genTraps(Random &rnd, World &world);

#endif // TRAPS_H
