#include "Jungle.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <numbers>
#include <set>

int levitateIsland(int center, int width, Random &rnd, World &world)
{
    int yMin = 0.45 * world.getUndergroundLevel();
    int xMin = center - width;
    int xMax = center + width;
    int minSurface = world.getHeight();
    for (int x = xMin; x < xMax; ++x) {
        minSurface = std::min(world.getSurfaceLevel(x), minSurface);
    }
    int floatHeight = rnd.getDouble(0.07, 0.2) * world.getUndergroundLevel();
    yMin = minSurface - floatHeight - 5;
    for (int x = xMin; x < xMax; ++x) {
        int yMax = yMin - 1.9 * (x - xMin) * (x - xMax) / width +
                   37 * rnd.getFineNoise(x, 0);
        if (world.conf.hiveQueen) {
            for (int y = yMin; y < yMax; ++y) {
                Tile &a = world.getTile(x, y);
                Tile &b = world.getTile(x, y + floatHeight);
                if (a.flag == Flag::border) {
                    if (b.flag != Flag::border) {
                        b = {};
                    }
                } else if (b.flag == Flag::border) {
                    if (y + floatHeight > world.getSurfaceLevel(x)) {
                        a.blockID = TileID::mud;
                    }
                } else {
                    std::swap(a, b);
                }
            }
        } else {
            for (int y = yMin; y < yMax; ++y) {
                std::swap(
                    world.getTile(x, y),
                    world.getTile(x, y + floatHeight));
            }
        }
    }
    return minSurface;
}

bool drawMahoganySegment(Pointf from, Pointf to, double width, World &world)
{
    std::set<Point> fillTiles;
    for (double t = 0; t <= 1; t += 0.1) {
        double centerX = std::lerp(from.x, to.x, t);
        double centerY = std::lerp(from.y, to.y, t);
        for (int x = centerX - width; x < centerX + width; ++x) {
            for (int y = centerY - width; y < centerY + width; ++y) {
                if (std::hypot(x - centerX, y - centerY) < width) {
                    fillTiles.emplace(x, y);
                }
            }
        }
    }
    size_t numCrossed = 0;
    for (auto [x, y] : fillTiles) {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::livingMahogany) {
            ++numCrossed;
        } else {
            tile.blockID = TileID::livingMahogany;
            tile.wallID = WallID::Unsafe::livingWood;
        }
    }
    return 2 * numCrossed > fillTiles.size();
}

void growMahoganyVine(
    Pointf from,
    double weight,
    double angle,
    Random &rnd,
    World &world)
{
    if (weight < 0.6) {
        return;
    }
    angle += std::clamp(
        1.8 * rnd.getFineNoise(from.x, from.y),
        -std::numbers::pi / 3,
        std::numbers::pi / 3);
    Pointf to{
        from.x + 3.9 * weight * std::cos(angle),
        from.y + 3.9 * weight * std::sin(angle)};
    bool crossedSegment = drawMahoganySegment(from, to, weight, world);
    if (crossedSegment && weight < 1.9) {
        return;
    }
    growMahoganyVine(to, rnd.getDouble(0.97, 0.98) * weight, angle, rnd, world);
    switch (static_cast<int>(99999 * (1 + rnd.getFineNoise(from.x, from.y))) %
            11) {
    case 0:
        growMahoganyVine(
            to,
            0.65 * weight,
            angle - std::numbers::pi / 2,
            rnd,
            world);
        break;
    case 1:
        growMahoganyVine(
            to,
            0.65 * weight,
            angle + std::numbers::pi / 2,
            rnd,
            world);
        break;
    case 2:
    case 3:
    case 4:
        for (int x = from.x - 6; x < from.x + 6; ++x) {
            for (int y = from.y - 6; y < from.y + 6; ++y) {
                Tile &tile = world.getTile(x, y);
                double threshold = hypot(from, {x, y}) / 3 - 1;
                if (tile.blockID == TileID::empty &&
                    rnd.getFineNoise(x, y) > threshold) {
                    tile.blockID = TileID::mahoganyLeaf;
                    tile.wallID = WallID::Safe::livingLeaf;
                }
            }
        }
        break;
    }
}

void growMahoganyVines(
    int minX,
    int maxX,
    int minY,
    int maxY,
    Random &rnd,
    World &world)
{
    int numVines = (maxY - minY) * (maxX - minX) / rnd.getInt(6910, 8640);
    while (numVines > 0) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        if (!world.regionPasses(x - 4, y - 4, 8, 8, [](Tile &tile) {
                return tile.blockID != TileID::empty &&
                       tile.blockID != TileID::livingMahogany &&
                       tile.blockID != TileID::mahoganyLeaf;
            })) {
            continue;
        }
        growMahoganyVine(
            {x, y},
            rnd.getDouble(2.1, 2.5),
            rnd.getDouble(-std::numbers::pi, std::numbers::pi),
            rnd,
            world);
        --numVines;
    }
}

void connectSurfaceCaves(int xMin, int xMax, Random &rnd, World &world)
{
    rnd.restoreShuffleState();
    for (int x = xMin; x < xMax; ++x) {
        int surface = world.getSurfaceLevel(x);
        for (int y = surface; y < world.getUndergroundLevel(); ++y) {
            double t = std::min(
                std::min({y - surface, x - xMin, xMax - x}) / 75.0,
                1.0);
            double threshold = std::lerp(
                2.94 - 3.1 * y / world.getUndergroundLevel(),
                -0.16,
                t);
            if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                rnd.getFineNoise(x, y) > threshold) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID != TileID::hive) {
                    tile.blockID = TileID::empty;
                }
            }
        }
    }
    rnd.shuffleNoise();
}

void levitateIslands(int lb, int ub, Random &rnd, World &world)
{
    if (world.conf.shattered) {
        growMahoganyVines(
            lb,
            ub,
            0.7 * world.getUndergroundLevel(),
            std::midpoint(world.getUndergroundLevel(), world.getCavernLevel()),
            rnd,
            world);
        return;
    }
    int minSurface = world.getUndergroundLevel();
    int xMin = lb;
    int xMax = lb;
    while (xMax < ub) {
        int width = rnd.getInt(20, 35);
        xMax += width;
        minSurface =
            std::min(levitateIsland(xMax, width, rnd, world), minSurface);
        xMax += width;
    }
    connectSurfaceCaves(xMin, xMax, rnd, world);
    int yMin = minSurface - 0.22 * world.getUndergroundLevel();
    int yMax = minSurface + 0.1 * world.getUndergroundLevel();
    growMahoganyVines(xMin, xMax, yMin, yMax, rnd, world);
    for (int x = xMin - 100; x < xMax + 100; ++x) {
        for (int y = yMin - 100; y < yMax + 100; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::livingMahogany ||
                static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 7 !=
                    0) {
                continue;
            }
            std::map<int, int> neighbors;
            for (auto [i, j] : {std::pair{1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
                ++neighbors[world.getTile(x + i, y + j).blockID];
            }
            if (neighbors[TileID::empty] == 1 &&
                neighbors[TileID::livingMahogany] == 3) {
                tile.blockID = TileID::jungleGrass;
            }
        }
    }
}

void genJungle(Random &rnd, World &world)
{
    std::cout << "Generating jungle\n";
    rnd.shuffleNoise();
    int minX;
    int maxX;
    if (world.conf.biomes == BiomeLayout::columns && !world.conf.hiveQueen) {
        double islandScale = world.conf.jungleSize > 1
                                 ? std::sqrt(world.conf.jungleSize)
                                 : world.conf.jungleSize;
        double center = world.jungleCenter + islandScale *
                                                 rnd.getDouble(-0.05, 0.05) *
                                                 world.getWidth();
        double scanDist =
            islandScale * rnd.getDouble(0.03, 0.035) * world.getWidth();
        minX = center - scanDist;
        maxX = center + scanDist;
    } else {
        int scanDist = rnd.getDouble(0.03, 0.035) * world.getWidth();
        int jungleCenter = world.jungleCenter;
        if (world.conf.hiveQueen) {
            if (jungleCenter > world.getWidth() / 2) {
                jungleCenter += 0.018 * world.getWidth();
            } else {
                jungleCenter -= 0.018 * world.getWidth();
            }
        }
        minX = jungleCenter;
        while (minX > 350 && minX > jungleCenter - scanDist &&
               world.getBiome(minX, world.getSurfaceLevel(minX)).jungle > 0.8) {
            --minX;
        }
        maxX = jungleCenter;
        while (
            maxX < world.getWidth() - 350 && maxX < jungleCenter + scanDist &&
            world.getBiome(maxX + 25, world.getSurfaceLevel(maxX + 25)).jungle >
                0.8) {
            ++maxX;
        }
    }
    levitateIslands(minX, maxX, rnd, world);
    for (int x = minX; x < maxX + 45; ++x) {
        for (int y = 0.45 * world.getUndergroundLevel();
             y < world.getUndergroundLevel();
             ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::mud && world.isExposed(x, y)) {
                tile.blockID = TileID::jungleGrass;
            }
        }
    }
}
