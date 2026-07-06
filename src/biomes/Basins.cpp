#include "Basins.h"

#include "Random.h"
#include "World.h"
#include <algorithm>
#include <iostream>
#include <numbers>

template <typename Func>
void genPondFor(Func isBiomeActive, Random &rnd, World &world)
{
    int minX = 0;
    int maxX = 0;
    int biomeStart = -1;
    for (int x = 350; x < world.getWidth() - 350; ++x) {
        bool posIsBiome =
            isBiomeActive(world.getBiome(x, world.getSurfaceLevel(x)));
        if (posIsBiome && biomeStart == -1) {
            biomeStart = x;
        } else if (!posIsBiome && biomeStart != -1) {
            if (x - biomeStart > maxX - minX) {
                minX = biomeStart;
                maxX = x;
            }
            biomeStart = -1;
        }
    }
    std::array localSurface{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    minX += localSurface.size();
    maxX -= localSurface.size();
    std::vector<int> lowPoints;
    for (int x = minX - localSurface.size(); x < maxX; ++x) {
        localSurface[x % localSurface.size()] =
            world.getSurfaceLevel(x + localSurface.size() / 2);
        if (x < minX) {
            continue;
        }
        int y = world.getSurfaceLevel(x);
        if (y == *std::max_element(localSurface.begin(), localSurface.end()) &&
            hypot(world.spawn, {x, y}) > 150) {
            lowPoints.push_back(x);
        }
    }
    if (lowPoints.empty()) {
        return;
    }
    int width = rnd.getInt(50, 60);
    double depthScale = rnd.getDouble(38, 42);
    minX = rnd.select(lowPoints) - 4 + width / 2;
    maxX = minX + width + 8;
    for (int x = minX; x < maxX; ++x) {
        int depth =
            depthScale * (std::sin(
                              std::numbers::pi * 0.5 *
                              (0.5 + (x - minX) / static_cast<double>(width))) -
                          0.7071);
        int surfaceY = world.getSurfaceLevel(x);
        for (int j = -3; j < depth; ++j) {
            Tile &tile = world.getTile(x, surfaceY + j);
            if (tile.flag != Flag::border) {
                tile = {};
                tile.flag = Flag::lake;
            }
        }
        Tile &baseTile = world.getTile(x, surfaceY + depth);
        if (baseTile.blockID == TileID::dirt) {
            baseTile.blockID = TileID::grass;
        }
    }
}

void genBasins(Random &rnd, World &world)
{
    std::cout << "Stocking ponds\n";
    genPondFor(
        [](const BiomeData &biome) { return biome.forest > 0.99; },
        rnd,
        world);
    genPondFor(
        [](const BiomeData &biome) { return biome.snow > 0.99; },
        rnd,
        world);
    genPondFor(
        [](const BiomeData &biome) { return biome.desert > 0.99; },
        rnd,
        world);
}
