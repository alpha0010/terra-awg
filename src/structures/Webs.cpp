#include "structures/Webs.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>

void genWebs(Random &rnd, World &world)
{
    double mainThreshold = computeOreThreshold(1.5 * world.conf.cobwebs);
    if (mainThreshold < -1.002) {
        return;
    }
    std::cout << "Spreading webs\n";
    rnd.shuffleNoise();
    double secondaryThreshold = computeOreThreshold(0.08 * world.conf.cobwebs);
    constexpr auto targetWalls = frozen::make_set<int>(
        {WallID::Unsafe::dirt,
         WallID::Unsafe::ebonstone,
         WallID::Unsafe::mud,
         WallID::Unsafe::snow,
         WallID::Unsafe::rockyDirt,
         WallID::Unsafe::oldStone,
         WallID::Unsafe::spider,
         WallID::Unsafe::jungle,
         WallID::Unsafe::ice,
         WallID::Unsafe::crimstone,
         WallID::Unsafe::caveDirt,
         WallID::Unsafe::roughDirt,
         WallID::Unsafe::granite,
         WallID::Unsafe::craggyStone,
         WallID::Unsafe::corruptGrowth,
         WallID::Unsafe::corruptMass,
         WallID::Unsafe::corruptPustule,
         WallID::Unsafe::corruptTendril,
         WallID::Unsafe::crimsonCrust,
         WallID::Unsafe::crimsonScab,
         WallID::Unsafe::crimsonTeeth,
         WallID::Unsafe::crimsonBlister,
         WallID::Unsafe::layeredDirt,
         WallID::Unsafe::crumblingDirt,
         WallID::Unsafe::crackedDirt,
         WallID::Unsafe::wavyDirt,
         WallID::Unsafe::hallowedPrism,
         WallID::Unsafe::hallowedCavern,
         WallID::Unsafe::hallowedShard,
         WallID::Unsafe::hallowedCrystalline,
         WallID::Unsafe::lichenStone,
         WallID::Unsafe::leafyJungle,
         WallID::Unsafe::ivyStone,
         WallID::Unsafe::jungleVine,
         WallID::Unsafe::wornStone,
         WallID::Unsafe::stalactiteStone,
         WallID::Unsafe::mottledStone,
         WallID::Unsafe::fracturedStone});
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [mainThreshold, secondaryThreshold, &targetWalls, &rnd, &world](int x) {
            for (int y = world.getSurfaceLevel(x); y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID != TileID::empty ||
                    tile.liquid != Liquid::none ||
                    tile.wallID == WallID::empty) {
                    continue;
                }
                double patchThreshold = mainThreshold;
                if (!targetWalls.contains(tile.wallID)) {
                    if (secondaryThreshold > -0.8) {
                        patchThreshold = secondaryThreshold;
                    } else {
                        continue;
                    }
                }
                double threshold =
                    1.5 - 2 * (rnd.getCoarseNoise(2 * x, 2 * y) + 1.1) /
                              (patchThreshold + 1.1);
                if (rnd.getFineNoise(4 * x, 4 * y) <
                    std::min(threshold, 0.15)) {
                    tile.blockID = TileID::cobweb;
                }
            }
        });
}
