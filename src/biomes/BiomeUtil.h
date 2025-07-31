#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include "Point.h"
#include "Tile.h"
#include "ids/Biome.h"
#include <functional>

class World;
class Random;

double computeOreThreshold(double oreMultiplier);

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

Point findCave(
    int yMin,
    int yMax,
    Random &rnd,
    World &world,
    int minSize,
    std::initializer_list<int> allowedBlocks);

Point getHexCentroid(Point pt, int scale);
Point getHexCentroid(int x, int y, int scale);

void growMossOn(int x, int y, World &world);

bool isInBiome(int x, int y, int scanDist, Biome biome, World &world);

void iterateZone(
    Point start,
    World &world,
    std::function<bool(Point)> isValid,
    std::function<void(Point)> f);

#endif // BIOMEUTIL_H
