#include "SpiderNest.h"

#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>

void fillSpiderNest(int x, int y, Random &rnd, World &world)
{
    int nestSize = rnd.getInt(15, 50);
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
    int numNests = world.getWidth() * world.getHeight() / 900000;
    for (int i = 0; i < numNests; ++i) {
        auto [x, y] = findStoneCave(
            world.getCavernLevel(),
            world.getUnderworldLevel() - 20,
            rnd,
            world);
        fillSpiderNest(x, y, rnd, world);
    }
}
