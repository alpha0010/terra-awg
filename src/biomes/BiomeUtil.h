#ifndef BIOMEUTIL_H
#define BIOMEUTIL_H

#include "World.h"
#include <set>

class Random;

double computeOreThreshold(double oreMultiplier);

void queuedEmbedWaterfalls(
    Point from,
    Point to,
    std::initializer_list<int> allowedBlocks,
    Liquid liquid,
    int proximity,
    World &world);

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

/**
 * @tparam Func `(int, int)->void`
 */
template <typename Func>
void iterateDiamond(int topHeight, int centerHeight, Func f)
{
    for (int i = 0; i < 2 * topHeight; ++i) {
        for (int j = std::abs(topHeight - i - 0.5); j < topHeight; ++j) {
            f(i, j);
        }
        for (int j = 0; j < centerHeight; ++j) {
            f(i, j + topHeight);
        }
        int maxJ = topHeight - std::abs(topHeight - i - 0.5);
        for (int j = 0; j < maxJ; ++j) {
            f(i, j + topHeight + centerHeight);
        }
    }
}

/**
 * @tparam BoundsCheck `(Point)->bool`
 * @tparam Func `(Point)->void`
 */
template <typename BoundsCheck, typename Func>
void iterateZone(Point start, World &world, BoundsCheck isValid, Func f)
{
    std::set<Point> border;
    std::set<Point> visited;
    std::vector<Point> locations{start};
    while (!locations.empty()) {
        Point loc = locations.back();
        locations.pop_back();
        if (loc.x < 0 || loc.y < 0 || loc.x >= world.getWidth() ||
            loc.y >= world.getHeight() || visited.contains(loc) ||
            border.contains(loc)) {
            continue;
        }
        if (isValid(loc)) {
            visited.insert(loc);
            for (auto delta : {Point{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                locations.push_back(loc + delta);
            }
        } else {
            border.insert(loc);
        }
    }
    for (Point loc : visited) {
        f(loc);
    }
}

#endif // BIOMEUTIL_H
