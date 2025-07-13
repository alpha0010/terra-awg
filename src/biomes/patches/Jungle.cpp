#include "biomes/patches/Jungle.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/Jungle.h"
#include <iostream>

void genJunglePatches(Random &rnd, World &world)
{
    std::cout << "Generating jungle\n";
    rnd.shuffleNoise();
    int scanDist = rnd.getDouble(0.03, 0.035) * world.getWidth();
    int jungleCenter = world.jungleCenter;
    if (world.conf.hiveQueen) {
        if (jungleCenter > world.getWidth() / 2) {
            jungleCenter += 0.018 * world.getWidth();
        } else {
            jungleCenter -= 0.018 * world.getWidth();
        }
    }
    int minX = jungleCenter;
    while (minX > 350 && minX > jungleCenter - scanDist &&
           world.getBiome(minX, world.getSurfaceLevel(minX)).jungle > 0.8) {
        --minX;
    }
    int maxX = jungleCenter;
    while (maxX < world.getWidth() - 350 && maxX < jungleCenter + scanDist &&
           world.getBiome(maxX + 25, world.getSurfaceLevel(maxX + 25)).jungle >
               0.8) {
        ++maxX;
    }
    levitateIslands(minX, maxX, rnd, world);
    for (int x = minX; x < maxX + 35; ++x) {
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
