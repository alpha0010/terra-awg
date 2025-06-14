#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include "Tile.h"
#include <utility>

class World;
class Random;

typedef std::pair<int, int> Point;

void embedWaterfalls(
    Point from,
    Point to,
    std::initializer_list<int> allowedBlocks,
    Liquid liquid,
    int proximity,
    Random &rnd,
    World &world);

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

void growMossOn(int x, int y, World &world);

#endif // BIOMEUTIL_H
