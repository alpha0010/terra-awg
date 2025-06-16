#include "biomes/patches/Hive.h"

#include "Random.h"
#include "World.h"
#include "biomes/Hive.h"
#include <iostream>

bool isValidHiveLocation(int x, int y, World &world)
{
    int scanDist = 15 + world.getWidth() / 120;
    double threshold = 2 * scanDist;
    threshold *= threshold;
    threshold *= 0.05;
    for (int i = -scanDist; i < scanDist; ++i) {
        for (int j = -scanDist; j < scanDist; ++j) {
            threshold += world.getBiome(x + i, y + j).jungle - 1;
            if (threshold < 0) {
                return false;
            }
        }
    }
    return true;
}

std::pair<int, int> selectHiveLocation(Random &rnd, World &world)
{
    for (int numTries = 0; numTries < 100; ++numTries) {
        int x = rnd.getInt(350, world.getWidth() - 350);
        int y = rnd.getInt(
            (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
            (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3);
        if (isValidHiveLocation(x, y, world)) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void genHivePatches(Random &rnd, World &world)
{
    std::cout << "Importing bees\n";
    double numHives =
        1 + rnd.getDouble(0, world.getWidth() * world.getHeight() / 5750000.0);
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
