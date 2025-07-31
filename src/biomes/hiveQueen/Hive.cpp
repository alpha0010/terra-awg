#include "biomes/hiveQueen/Hive.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/Hive.h"

void genHiveHiveQueen(Random &rnd, World &world)
{
    genHive(rnd, world);
    fillHive(
        (0.5 + rnd.getDouble(-0.05, 0.05)) * world.getWidth(),
        std::midpoint(
            world.getSurfaceLevel(world.getWidth() / 2),
            world.getUnderworldLevel()) +
            rnd.getDouble(-0.05, 0.05) * world.getHeight(),
        std::midpoint(world.conf.hiveSize, 1.0) *
            std::midpoint<double>(world.getWidth(), 3.56 * world.getHeight()) /
            rnd.getDouble(19, 22),
        rnd,
        world);
}
