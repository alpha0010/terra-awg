#ifndef GEMGROVE_H
#define GEMGROVE_H

#include <utility>

class World;
class Random;

typedef std::pair<int, int> Point;

void genGemGrove(Random &rnd, World &world);
Point selectGemGroveLocation(double &groveSize, Random &rnd, World &world);
void placeGroveDecoGems(Random &rnd, World &world);
void placeGemChest(Random &rnd, World &world);

#endif // GEMGROVE_H
