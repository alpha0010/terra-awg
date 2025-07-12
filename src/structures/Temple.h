#ifndef TEMPLE_H
#define TEMPLE_H

#include <utility>

class World;
class Random;

typedef std::pair<int, int> Point;

void genTemple(Random &rnd, World &world);
void clearTempleSurface(Point center, int scanDist, Random &rnd, World &world);

#endif // TEMPLE_H
