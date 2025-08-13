#include "SpiderNest.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>

void fillSpiderNest(int x, int y, Random &rnd, World &world)
{
    int nestSize = world.conf.spiderNestSize * rnd.getInt(15, 50);
    for (int i = -nestSize; i < nestSize; ++i) {
        for (int j = -nestSize; j < nestSize; ++j) {
            double threshold = 2 * std::hypot(i, j) / nestSize - 1;
            if (rnd.getFineNoise(x + i, y + j) > threshold) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::empty) {
                    tile.wallID = WallID::Unsafe::spider;
                }
            }
        }
    }
}

void genSpiderNest(Random &rnd, World &world)
{
    std::cout << "Hatching spiders\n";
    rnd.shuffleNoise();
    int numNests = world.conf.spiderNestFreq * world.getWidth() *
                   world.getHeight() / 900000;
    int minY = world.conf.dontDigUp ? world.getUndergroundLevel()
                                    : world.getCavernLevel();
    int maxY = world.conf.dontDigUp ? world.getCavernLevel()
                                    : world.getUnderworldLevel() - 20;
    for (int i = 0; i < numNests; ++i) {
        auto [x, y] = findStoneCave(minY, maxY, rnd, world);
        if (x != -1) {
            fillSpiderNest(x, y, rnd, world);
        }
    }
}
