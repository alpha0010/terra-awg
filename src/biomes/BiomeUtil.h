#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include <utility>

class World;
class Random;

typedef std::pair<int, int> Point;

/**
 * Cover most (but not all) gaps in between walls in a zone.
 */
void fillLargeWallGaps(Point from, Point to, Random &rnd, World &world);

/**
 * Find an open stone surrounded space. May reduce size restriction if searching
 * takes too long.
 */
Point findStoneCave(
    int yMin,
    int yMax,
    Random &rnd,
    World &world,
    int minSize = 6);

#endif // BIOMEUTIL_H
