#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include <utility>

class World;
class Random;

std::pair<int, int>
findStoneCave(int yMin, int yMax, Random &rnd, World &world, int minSize = 6);

#endif // BIOMEUTIL_H
