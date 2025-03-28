#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include <utility>

class World;
class Random;

typedef std::pair<int, int> Point;

void fillLargeWallGaps(Point from, Point to, Random &rnd, World &world);

Point findStoneCave(
    int yMin,
    int yMax,
    Random &rnd,
    World &world,
    int minSize = 6);

#endif // BIOMEUTIL_H
