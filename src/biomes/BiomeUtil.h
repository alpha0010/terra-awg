#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include <utility>

class World;
class Random;

std::pair<int, int>
findStoneCave(int yMin, int yMax, Random &rnd, World &world);

#endif // BIOMEUTIL_H
