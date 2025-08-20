#ifndef BASE_H
#define BASE_H

#include <vector>

class World;
class Random;

void computeSurfaceLevel(Random &rnd, World &world);
double computeStoneThreshold(int y, World &world);
std::vector<std::tuple<int, int, int, int>> getOreLayers(World &world);
void applyBaseTerrain(Random &rnd, World &world);
void genWorldBase(Random &rnd, World &world);

#endif // BASE_H
