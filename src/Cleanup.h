#ifndef CLEANUP_H
#define CLEANUP_H

#include <utility>

class Random;
class World;

std::pair<int, int> getAttachedOpenWall(World &world, int x, int y);
void smoothSurfaces(World &world);
void finalizeWalls(Random &rnd, World &world);

#endif // CLEANUP_H
