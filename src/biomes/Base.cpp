#include "Base.h"

#include "Random.h"
#include "World.h"
#include <algorithm>
#include <iostream>
#include <map>

void scatterResource(Random &rnd, World &world, int resource)
{
    rnd.shuffleNoise();
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            if (rnd.getFineNoise(x, y) > 0.7) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID != TileID::empty) {
                    tile.blockID = resource;
                }
            }
        }
    }
}

void genOreVeins(Random &rnd, World &world, int oreRoof, int oreFloor, int ore)
{
    rnd.shuffleNoise();
    for (int numDeposits = world.getWidth() * (oreFloor - oreRoof) / 40000;
         numDeposits > 0;
         --numDeposits) {
        int x = rnd.getInt(0, world.getWidth());
        int y = rnd.getInt(oreRoof, oreFloor);
        double depositSize = rnd.getDouble(4, 10);
        for (int i = -10; i < 10; ++i) {
            for (int j = -10; j < 10; ++j) {
                double threshold =
                    1 - std::pow(std::hypot(i, j) / depositSize, 0.3);
                if (std::abs(rnd.getFineNoise(x + i, y + j)) < threshold) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.blockID != TileID::empty) {
                        tile.blockID = ore;
                    }
                }
            }
        }
    }
}

void genWorldBase(Random &rnd, World &world)
{
    rnd.initNoise(world.getWidth(), world.getHeight(), 0.07);
    std::cout << "Generating base terrain\n";
    double surfaceLevel = rnd.getDouble(
        0.7 * world.getUndergroundLevel(),
        0.8 * world.getUndergroundLevel());
    int center = world.getWidth() / 2;
    std::vector<std::tuple<int, int, int>> wallVarNoise;
    for (int wallId : WallVariants::dirt) {
        wallVarNoise.emplace_back(
            rnd.getInt(0, world.getWidth()),
            rnd.getInt(0, world.getHeight()),
            wallId);
    }
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = surfaceLevel +
                     std::min(
                         {0.1 * std::abs(center - x) + 15,
                          0.08 * std::min(x, world.getWidth() - x) + 5,
                          50.0}) *
                         rnd.getCoarseNoise(x, 0);
             y < world.getHeight();
             ++y) {
            double threshold =
                y < world.getUndergroundLevel()
                    ? 3.0 * y / world.getUndergroundLevel() - 3
                    : static_cast<double>(y - world.getUndergroundLevel()) /
                          (world.getHeight() - world.getUndergroundLevel());
            Tile &tile = world.getTile(x, y);
            tile.blockID = rnd.getFineNoise(x, y) > threshold ? TileID::dirt
                                                              : TileID::stone;
            for (auto [i, j, wallId] : wallVarNoise) {
                if (std::abs(rnd.getCoarseNoise(x + i, y + j)) < 0.07) {
                    tile.wallID = wallId;
                    break;
                }
            }
            if (tile.wallID == WallID::empty &&
                y < world.getUndergroundLevel()) {
                tile.wallID = tile.blockID == TileID::stone
                                  ? WallID::Unsafe::rockyDirt
                                  : WallID::Unsafe::dirt;
            }
        }
    }

    scatterResource(rnd, world, TileID::clay);
    scatterResource(rnd, world, TileID::sand);
    scatterResource(rnd, world, TileID::mud);

    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    std::map<int, int> underworldWalls{{WallID::empty, WallID::empty}};
    for (int wallId : WallVariants::dirt) {
        underworldWalls[wallId] = rnd.select(
            WallVariants::underworld.begin(),
            WallVariants::underworld.end());
    }
    for (int x = 0; x < world.getWidth(); ++x) {
        int underworldRoof =
            world.getUnderworldLevel() + 0.1 * underworldHeight +
            20 * rnd.getCoarseNoise(x, 0.33 * world.getHeight());
        int underworldFloor =
            world.getUnderworldLevel() + 0.5 * underworldHeight +
            20 * rnd.getCoarseNoise(x, 0.66 * world.getHeight());
        for (int y = world.getUnderworldLevel() + 20 * rnd.getCoarseNoise(x, 0);
             y < world.getHeight();
             ++y) {
            Tile &tile = world.getTile(x, y);
            tile.blockID = y < underworldRoof || y > underworldFloor
                               ? TileID::ash
                               : TileID::empty;
            tile.wallID = underworldWalls[tile.wallID];
        }
    }

    std::cout << "Generating ore veins\n";
    genOreVeins(
        rnd,
        world,
        0.6 * world.getUndergroundLevel(),
        (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
        world.copperVariant);
    genOreVeins(
        rnd,
        world,
        0.85 * world.getUndergroundLevel(),
        (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
        world.ironVariant);
    genOreVeins(
        rnd,
        world,
        (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
        (world.getCavernLevel() + world.getUnderworldLevel()) / 2,
        world.silverVariant);
    genOreVeins(
        rnd,
        world,
        (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
        world.getUnderworldLevel(),
        world.goldVariant);

    std::cout << "Digging caves\n";
    rnd.shuffleNoise();
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            double threshold = y < world.getUndergroundLevel()
                                   ? 3 - 3.1 * y / world.getUndergroundLevel()
                               : y > world.getUnderworldLevel()
                                   ? 3.1 * (y - world.getUnderworldLevel()) /
                                             underworldHeight -
                                         0.1
                                   : -0.1;
            if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                rnd.getFineNoise(x, y) > threshold) {
                Tile &tile = world.getTile(x, y);
                tile.blockID = TileID::empty;
            }
        }
    }
}
