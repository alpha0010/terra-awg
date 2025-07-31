#include "biomes/shattered/ShatteredLand.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include <iostream>

std::pair<int, double>
distToBasin(int x, int y, int guessX, const std::vector<double> &basin)
{
    int minDistAt = guessX;
    double dist = basin.size();
    for (int basinX = guessX; basinX < static_cast<int>(basin.size());
         ++basinX) {
        double curDist = std::hypot(x - basinX, y - basin[basinX]);
        if (curDist < dist) {
            minDistAt = basinX;
            dist = curDist;
        } else {
            break;
        }
    }
    for (int basinX = guessX - 1; basinX >= 0; --basinX) {
        double curDist = std::hypot(x - basinX, y - basin[basinX]);
        if (curDist < dist) {
            minDistAt = basinX;
            dist = curDist;
        } else {
            break;
        }
    }
    return {minDistAt, dist};
}

bool markIslandAt(int x, int y, int size, Random &rnd, World &world)
{
    if (!world.regionPasses(
            x + 0.15 * size,
            y,
            0.7 * size,
            0.4 * size,
            [](Tile &tile) { return !tile.wireRed; })) {
        return false;
    }
    double surfaceScale = world.conf.surfaceAmplitude * rnd.getDouble(28, 35);
    double dropScale =
        size < 99 ? rnd.getDouble(18, 26) : rnd.getDouble(30, 42);
    for (int i = 0; i < size; ++i) {
        int surface = y + surfaceScale * rnd.getCoarseNoise(x + i, y);
        int maxJ = 1.9 * i * (size - i) / size +
                   dropScale * rnd.getFineNoise(x + i, y);
        for (int j = 0; j < maxJ; ++j) {
            world.getTile(x + i, surface + j).wireRed = true;
        }
    }
    return true;
}

void genShatteredLand(Random &rnd, World &world)
{
    std::cout << "Observing cataclysm\n";
    rnd.shuffleNoise();
    int centerSurface = world.getSurfaceLevel(world.getWidth() / 2) - 10;
    double totalDrop = 0.99 * world.getUnderworldLevel() - centerSurface;
    std::vector<double> basin;
    basin.reserve(world.getWidth());
    for (int x = 0; x < world.getWidth(); ++x) {
        basin.push_back(
            centerSurface +
            totalDrop *
                (1 / (1 + std::exp(0.017 * (600 - x))) +
                 1 / (1 + std::exp(0.017 * (600 + x - world.getWidth())))) -
            totalDrop);
    }
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [&basin, &rnd, &world](int x) {
            int guessX = x;
            double dist;
            for (int y = 0; y < world.getHeight(); ++y) {
                std::tie(guessX, dist) = distToBasin(x, y, guessX, basin);
                if (y > basin[x]) {
                    dist = -dist;
                }
                if (dist > 10 * (std::midpoint(rnd.getFineNoise(x, y), 0.0) +
                                 rnd.getCoarseNoise(x, y))) {
                    continue;
                }
                world.getTile(x, y).wireRed = true;
            }
        });
    int centerIslandWidth = rnd.getInt(250, 350);
    markIslandAt(
        (world.getWidth() - centerIslandWidth) / 2,
        centerSurface,
        centerIslandWidth,
        rnd,
        world);
    int numFails = 0;
    while (numFails < 5000) {
        if (!markIslandAt(
                rnd.getInt(400, world.getWidth() - 400),
                rnd.getInt(centerSurface, world.getUnderworldLevel() - 120),
                rnd.getInt(100, 400),
                rnd,
                world)) {
            ++numFails;
        }
    }
    double numSmall = world.getWidth() * world.getHeight() / 115200;
    while (numSmall > 0) {
        if (markIslandAt(
                rnd.getInt(400, world.getWidth() - 400),
                rnd.getInt(centerSurface, world.getUnderworldLevel() - 120),
                rnd.getInt(20, 80),
                rnd,
                world)) {
            numSmall -= 1;
        } else {
            numSmall -= 0.1;
        }
    }
    parallelFor(std::views::iota(0, world.getWidth()), [&world](int x) {
        bool foundSurface = false;
        for (int y = 0.5 * world.getUndergroundLevel(); y < world.getHeight();
             ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.wireRed) {
                tile.wireRed = false;
                if (!foundSurface) {
                    foundSurface = true;
                    if (y < world.getUndergroundLevel()) {
                        world.getSurfaceLevel(x) =
                            std::max(world.getSurfaceLevel(x), y - 1);
                    } else {
                        world.getSurfaceLevel(x) = std::max(
                            world.getSurfaceLevel(x),
                            world.getSurfaceLevel(x - 1));
                    }
                }
            } else if (
                foundSurface || tile.blockID != TileID::hive ||
                y >= world.getUndergroundLevel()) {
                Flag flag = tile.flag;
                tile = {};
                tile.flag = flag;
            }
        }
        if (x != 0) {
            for (int y = 0.5 * world.getUndergroundLevel();
                 y < world.getUnderworldLevel();
                 ++y) {
                Tile &tile = world.getTile(x - 1, y);
                if (tile.blockID == TileID::mud &&
                    world.getBiome(x - 1, y).active == Biome::jungle &&
                    world.isExposed(x - 1, y)) {
                    tile.blockID = TileID::jungleGrass;
                } else if (
                    tile.blockID == TileID::dirt &&
                    y < world.getUndergroundLevel() &&
                    world.getBiome(x - 1, y).active == Biome::forest &&
                    world.isExposed(x - 1, y)) {
                    tile.blockID = TileID::grass;
                }
            }
        }
    });
}
