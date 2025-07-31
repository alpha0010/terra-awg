#ifndef BASE_H
#define BASE_H

class World;
class Random;

void computeSurfaceLevel(Random &rnd, World &world);
void applyBaseTerrain(Random &rnd, World &world);
void genWorldBase(Random &rnd, World &world);

#endif // BASE_H
