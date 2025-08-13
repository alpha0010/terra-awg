#include "MarbleCave.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>

void fillMarbleCave(int x, int y, Random &rnd, World &world)
{
    double caveWidth = world.conf.marbleSize * rnd.getDouble(70, 150);
    double caveHeight = world.conf.marbleSize * rnd.getDouble(40, 70);
    rnd.shuffleNoise();
    if (!world.conf.shattered) {
        fillLargeWallGaps(
            {x - 0.7 * caveWidth, y - 0.7 * caveHeight},
            {x + 0.7 * caveWidth, y + 0.7 * caveHeight},
            rnd,
            world);
    }
    for (int i = std::max<int>(-caveWidth, -x); i < caveWidth; ++i) {
        int stalactiteLen = 0;
        int stalacIter = 0;
        for (int j = std::max<int>(-caveHeight, -y); j < caveHeight; ++j) {
            double threshold =
                4 * std::hypot(i / caveWidth, j / caveHeight) - 3;
            if (rnd.getFineNoise(x + i, y + j) < threshold) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            bool nextTileIsEmpty =
                world.getTile(x + i, y + j + 1).blockID == TileID::empty;
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::grass:
            case TileID::stone:
            case TileID::ice:
            case TileID::sandstone:
            case TileID::mud:
            case TileID::jungleGrass:
                tile.blockID = TileID::marble;
                if (nextTileIsEmpty) {
                    stalactiteLen = std::max(
                        0.0,
                        16 * rnd.getFineNoise(4 * (x + i), 100 * stalacIter));
                    ++stalacIter;
                }
                break;
            case TileID::clay:
            case TileID::sand:
                if (world.getBiome(x + i, y + j).active != Biome::desert) {
                    tile.blockID = TileID::smoothMarble;
                }
                break;
            case TileID::empty:
                if (nextTileIsEmpty && stalactiteLen > 0) {
                    tile.blockID = TileID::marble;
                    --stalactiteLen;
                }
            default:
                break;
            }
            if (tile.wallID != WallID::empty) {
                tile.wallID = WallID::Unsafe::marble;
            }
        }
    }
}

void genMarbleCave(Random &rnd, World &world)
{
    std::cout << "Excavating marble\n";
    int numCaves =
        world.conf.marbleFreq * world.getWidth() * world.getHeight() / 1200000;
    int minY =
        std::midpoint(world.getUndergroundLevel(), world.getCavernLevel());
    int maxY = world.conf.biomes == BiomeLayout::layers && !world.conf.hiveQueen
                   ? 0.526 * world.getHeight()
                   : world.getUnderworldLevel();
    for (int i = 0; i < numCaves; ++i) {
        auto [x, y] = i % 3 == 0 ? findStoneCave(minY, maxY, rnd, world, 30)
                                 : findCave(
                                       minY,
                                       maxY,
                                       rnd,
                                       world,
                                       30,
                                       {TileID::stone,
                                        TileID::snow,
                                        TileID::ice,
                                        TileID::sandstone,
                                        i % 2 == 0 ? TileID::jungleGrass
                                                   : TileID::hardenedSand});
        if (x != -1) {
            fillMarbleCave(x, y, rnd, world);
        }
    }
}
