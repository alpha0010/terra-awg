#ifndef PATCHES_BASE_H
#define PATCHES_BASE_H

#include "World.h"

class Random;

void genWorldBasePatches(Random &rnd, World &world);
BiomeData computeBiomeData(int x, int y, Random &rnd);
void identifySurfaceBiomes(World &world);

#endif // PATCHES_BASE_H
