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

typedef std::pair<double, double> Pointf;

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
        for (int y = yMin; y < yMax; ++y) {
            std::swap(world.getTile(x, y), world.getTile(x, y + floatHeight));
        }
    }
    return minSurface;
}

bool drawMahoganySegment(Pointf from, Pointf to, double width, World &world)
{
    std::set<Point> fillTiles;
    for (double t = 0; t <= 1; t += 0.1) {
        double centerX = std::lerp(from.first, to.first, t);
        double centerY = std::lerp(from.second, to.second, t);
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
        1.8 * rnd.getFineNoise(from.first, from.second),
        -std::numbers::pi / 3,
        std::numbers::pi / 3);
    Pointf to{
        from.first + 3.9 * weight * std::cos(angle),
        from.second + 3.9 * weight * std::sin(angle)};
    bool crossedSegment = drawMahoganySegment(from, to, weight, world);
    if (crossedSegment && weight < 1.9) {
        return;
    }
    growMahoganyVine(to, rnd.getDouble(0.97, 0.98) * weight, angle, rnd, world);
    switch (static_cast<int>(
                99999 * (1 + rnd.getFineNoise(from.first, from.second))) %
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
        for (int x = from.first - 6; x < from.first + 6; ++x) {
            for (int y = from.second - 6; y < from.second + 6; ++y) {
                Tile &tile = world.getTile(x, y);
                double threshold =
                    std::hypot(x - from.first, y - from.second) / 3 - 1;
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
                world.getTile(x, y).blockID = TileID::empty;
            }
        }
    }
    rnd.shuffleNoise();
}

void levitateIslands(int lb, int ub, Random &rnd, World &world)
{
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
    int numVines = (yMax - yMin) * (xMax - xMin) / rnd.getInt(6910, 8640);
    while (numVines > 0) {
        int x = rnd.getInt(xMin, xMax);
        int y = rnd.getInt(yMin, yMax);
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
    double islandScale = world.conf.jungleSize > 1
                             ? std::sqrt(world.conf.jungleSize)
                             : world.conf.jungleSize;
    double center = world.jungleCenter +
                    islandScale * rnd.getDouble(-0.05, 0.05) * world.getWidth();
    double scanDist =
        islandScale * rnd.getDouble(0.03, 0.035) * world.getWidth();
    levitateIslands(center - scanDist, center + scanDist, rnd, world);

    center = world.jungleCenter;
    scanDist = world.conf.jungleSize * 0.11 * world.getWidth();
    std::map<int, int> surfaceJungleWalls;
    for (int wallId : WallVariants::dirt) {
        surfaceJungleWalls[wallId] = rnd.select(
            {WallID::Unsafe::jungle,
             WallID::Unsafe::jungle,
             rnd.select(WallVariants::stone)});
    }
    std::map<int, int> undergroundJungleWalls;
    for (int wallId : WallVariants::dirt) {
        undergroundJungleWalls[wallId] = rnd.select(WallVariants::jungle);
    }
    parallelFor(
        std::views::iota(
            std::max<int>(center - scanDist, 0),
            std::min<int>(center + scanDist, world.getWidth())),
        [center, &surfaceJungleWalls, &undergroundJungleWalls, &rnd, &world](
            int x) {
            int lastTileID = TileID::empty;
            for (int y = 0; y < world.getHeight(); ++y) {
                double threshold =
                    std::abs(x - center) / 100.0 -
                    (world.conf.jungleSize * world.getWidth() / 1050.0);
                if (rnd.getCoarseNoise(x, y) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x, y);
                threshold = 2.0 * (y - world.getCavernLevel()) *
                                (y - world.getHeight()) /
                                std::pow(
                                    world.getHeight() - world.getCavernLevel(),
                                    2) +
                            0.75;
                if ((y > world.getCavernLevel() &&
                     rnd.getCoarseNoise(2 * x, 2 * y) > threshold) ||
                    (y > world.getUndergroundLevel() &&
                     y < world.getUnderworldLevel() &&
                     rnd.getFineNoise(2 * x, 2 * y) > 0.78)) {
                    tile.blockID = TileID::empty;
                    if (lastTileID == TileID::mud) {
                        Tile &prevTile = world.getTile(x, y - 1);
                        if (prevTile.blockID == TileID::mud) {
                            prevTile.blockID = TileID::jungleGrass;
                        }
                    }
                }
                switch (tile.blockID) {
                case TileID::ice:
                case TileID::sandstone:
                    if (rnd.getFineNoise(x, y) > -0.02) {
                        break;
                    }
                    [[fallthrough]];
                case TileID::dirt:
                case TileID::stone:
                    threshold =
                        std::abs(x - center) / 260.0 -
                        (world.conf.jungleSize * world.getWidth() / 2700.0);
                    if (rnd.getFineNoise(x, y) > threshold) {
                        tile.blockID = world.isExposed(x, y)
                                           ? TileID::jungleGrass
                                           : TileID::mud;
                        if (y < world.getUndergroundLevel() &&
                            tile.blockID == TileID::mud &&
                            static_cast<int>(
                                99999 * (1 + rnd.getFineNoise(x, y))) %
                                    100 ==
                                0) {
                            tile.blockID = TileID::jungleGrass;
                        }
                    }
                    break;
                case TileID::grass:
                    tile.blockID = TileID::jungleGrass;
                    break;
                case TileID::sand:
                case TileID::smoothMarble:
                    tile.blockID = TileID::silt;
                    break;
                case TileID::snow:
                    tile.blockID = TileID::slush;
                    break;
                case TileID::mud:
                    tile.blockID = TileID::stone;
                    break;
                case TileID::cloud:
                    tile.blockID = TileID::rainCloud;
                    break;
                default:
                    break;
                }
                if (y < world.getUndergroundLevel()) {
                    if (tile.blockID == TileID::empty) {
                        auto itr = surfaceJungleWalls.find(tile.wallID);
                        if (itr != surfaceJungleWalls.end()) {
                            tile.wallID = itr->second;
                        }
                    } else if (
                        tile.wallID != WallID::Unsafe::livingWood &&
                        tile.wallID != WallID::Safe::livingLeaf) {
                        tile.wallID = WallID::Unsafe::mud;
                    }
                } else {
                    auto itr = undergroundJungleWalls.find(tile.wallID);
                    if (itr != undergroundJungleWalls.end()) {
                        tile.wallID = itr->second;
                    }
                }
                lastTileID = tile.blockID;
            }
        });
}
