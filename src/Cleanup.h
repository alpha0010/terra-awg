#ifndef CLEANUP_H
#define CLEANUP_H

class Random;
class World;

void smoothSurfaces(World &world);
void finalizeWalls(Random &rnd, World &world);

#endif // CLEANUP_H
