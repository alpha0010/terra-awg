#include "GraniteCave.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>

void fillGraniteCave(int centerX, int centerY, Random &rnd, World &world)
{
    double caveSize = world.conf.graniteSize * rnd.getDouble(70, 150);
    int noiseShuffleX = rnd.getInt(0, world.getWidth());
    int noiseShuffleY = rnd.getInt(0, world.getHeight());
    constexpr auto bgOverrideExcl = frozen::make_set<int>(
        {TileID::sand,
         TileID::hardenedSand,
         TileID::sandstone,
         TileID::ice,
         TileID::snow,
         TileID::marble,
         TileID::mud,
         TileID::mushroomGrass,
         TileID::jungleGrass});
    if (!world.conf.shattered) {
        fillLargeWallGaps(
            {centerX - 0.71 * caveSize, centerY - 0.71 * caveSize},
            {centerX + 0.71 * caveSize, centerY + 0.71 * caveSize},
            rnd,
            world);
    }
    for (int x = std::max(centerX - caveSize, 0.0); x < centerX + caveSize;
         ++x) {
        for (int y = std::max(centerY - caveSize, 0.0); y < centerY + caveSize;
             ++y) {
            double threshold = std::hypot(x - centerX, y - centerY) / caveSize;
            if (rnd.getFineNoise(x + noiseShuffleX, y + noiseShuffleY) <
                6 * threshold - 5) {
                continue;
            }
            bool shouldClear =
                std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) <
                    0.2 - 0.05 * threshold &&
                rnd.getFineNoise(x, y) > std::min(-0.16, 4 * threshold - 4.16);
            Tile &tile = world.getTile(x, y);
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::grass:
            case TileID::stone:
                tile.blockID = shouldClear ? TileID::empty : TileID::granite;
                break;
            case TileID::sand:
            case TileID::clay:
                tile.blockID =
                    shouldClear ? TileID::empty : TileID::smoothGranite;
                break;
            case TileID::ice:
            case TileID::sandstone:
            case TileID::mud:
            case TileID::jungleGrass:
            case TileID::mushroomGrass:
            case TileID::marble:
                if (rnd.getFineNoise(x + noiseShuffleX, y + noiseShuffleY) >
                    0) {
                    tile.blockID =
                        shouldClear ? TileID::empty : TileID::granite;
                }
                break;
            default:
                break;
            }
            if ((tile.wallID != WallID::empty || shouldClear) &&
                y < world.getUnderworldLevel() &&
                !bgOverrideExcl.contains(tile.blockID) &&
                (tile.wallID != WallID::Unsafe::marble ||
                 rnd.getFineNoise(x + noiseShuffleX, y + noiseShuffleY) > 0)) {
                tile.wallID = WallID::Unsafe::granite;
            }
        }
    }
    caveSize += 60;
    if (centerY - caveSize < world.getCavernLevel() || world.conf.dontDigUp) {
        return;
    }
    for (int x = std::max(centerX - caveSize, 0.0); x < centerX + caveSize;
         ++x) {
        for (int y = std::max(centerY - caveSize, 0.0); y < centerY + caveSize;
             ++y) {
            if (std::hypot(x - centerX, y - centerY) < caveSize) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::dirt) {
                    tile.blockID = TileID::granite;
                }
            }
        }
    }
}

void genGraniteCave(Random &rnd, World &world)
{
    std::cout << "Smoothing granite\n";
    int numCaves =
        world.conf.graniteFreq * world.getWidth() * world.getHeight() / 2000000;
    rnd.restoreShuffleState();
    int minY = world.conf.biomes == BiomeLayout::layers && !world.conf.hiveQueen
                   ? 0.526 * world.getHeight()
                   : std::midpoint(
                         world.getUndergroundLevel(),
                         world.getCavernLevel());
    for (int i = 0; i < numCaves; ++i) {
        auto [x, y] =
            findStoneCave(minY, world.getUnderworldLevel(), rnd, world, 30);
        if (x != -1) {
            fillGraniteCave(x, y, rnd, world);
        }
    }
}
