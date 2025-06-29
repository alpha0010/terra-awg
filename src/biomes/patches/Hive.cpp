#include "biomes/patches/Hive.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "biomes/Hive.h"
#include <iostream>

std::pair<int, int> selectHiveLocation(Random &rnd, World &world)
{
    for (int numTries = 0; numTries < 100; ++numTries) {
        int x = rnd.getInt(350, world.getWidth() - 350);
        int y = rnd.getInt(
            (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
            (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3);
        if (isInBiome(
                x,
                y,
                15 + world.getWidth() / 120,
                Biome::jungle,
                world)) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void genHivePatches(Random &rnd, World &world)
{
    std::cout << "Importing bees\n";
    double numHives =
        world.conf.hiveFreq *
        (1 +
         rnd.getDouble(0, world.getWidth() * world.getHeight() / 5750000.0));
    while (numHives > 0) {
        auto [x, y] = selectHiveLocation(rnd, world);
        if (x == -1) {
            numHives -= 0.1;
            continue;
        }
        --numHives;
        fillHive(x, y, rnd, world);
    }
}
