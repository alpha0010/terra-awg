#ifndef GEMGROVE_H
#define GEMGROVE_H

#include "Point.h"

class World;
class Random;

void genGemGrove(Random &rnd, World &world);
Point selectGemGroveLocation(double &groveSize, Random &rnd, World &world);
void placeGroveDecoGems(Random &rnd, World &world);
void placeGemChest(Random &rnd, World &world);

#endif // GEMGROVE_H
