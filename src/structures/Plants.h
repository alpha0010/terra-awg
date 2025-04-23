#ifndef PLANTS_H
#define PLANTS_H

#include "structures/StructureUtil.h"

class World;
class Random;

void genPlants(const LocationBins &locations, Random &rnd, World &world);
void genGrasses(const LocationBins &locations, Random &rnd, World &world);

#endif // PLANTS_H
