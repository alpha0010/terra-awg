#include "GlowingMoss.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>

bool isSolidArea(int x, int y, int size, World &world)
{
    constexpr auto allowedTiles =
        frozen::make_set<int>({TileID::dirt, TileID::stone, TileID::clay});
    return world.regionPasses(x, y, size, size, [&allowedTiles](Tile &tile) {
        return allowedTiles.contains(tile.blockID);
    });
}

Point findSolidArea(int minY, int maxY, Random &rnd, World &world)
{
    int size = 8;
    while (true) {
        int x = rnd.getInt(0, world.getWidth() - size);
        int y = rnd.getInt(minY, maxY - size);
        if (isSolidArea(x, y, size, world)) {
            return {x + size / 2, y + size / 2};
        }
    }
}

void fillGlowingMossCave(Random &rnd, World &world)
{
    rnd.shuffleNoise();
    auto [centerX, centerY] = findSolidArea(
        world.getCavernLevel(),
        world.getUnderworldLevel(),
        rnd,
        world);
    double size = world.conf.glowingMossSize * world.getWidth() *
                  world.getHeight() / rnd.getDouble(95000, 210000);
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::dirt || tile.blockID == TileID::stone) {
                double threshold = std::hypot(x - centerX, y - centerY) / size;
                if (std::max(
                        std::abs(rnd.getBlurNoise(5 * x, 5 * y)),
                        std::abs(rnd.getBlurNoise(7 * x, 7 * y))) >
                    std::max(threshold, 0.53)) {
                    // Dig small pockets in the stone for more moss edge
                    // targets.
                    tile.blockID = TileID::empty;
                }
            }
        }
    }
    int mossType = rnd.select(
        {TileID::kryptonMossStone,
         TileID::xenonMossStone,
         TileID::argonMossStone,
         TileID::neonMossStone});
    if (centerY >
        (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5) {
        mossType = rnd.select({mossType, (int)TileID::lavaMossStone});
    }
    std::map<int, int> wallRepl;
    for (int wallId : WallVariants::dirt) {
        wallRepl[wallId] = mossType == TileID::lavaMossStone
                               ? rnd.select(WallVariants::underworld)
                               : rnd.select(WallVariants::stone);
    }
    std::vector<Point> mossLocations;
    for (int x = centerX - size; x < centerX + size; ++x) {
        for (int y = centerY - size; y < centerY + size; ++y) {
            double threshold =
                4 * std::hypot(x - centerX, y - centerY) / size - 3;
            if (rnd.getFineNoise(x, y) > threshold) {
                Tile &tile = world.getTile(x, y);
                auto itr = wallRepl.find(tile.wallID);
                if (itr != wallRepl.end()) {
                    tile.wallID = itr->second;
                }
                if (tile.blockID == TileID::stone && world.isExposed(x, y)) {
                    // Coat edges in moss.
                    tile.blockID = mossType;
                    mossLocations.emplace_back(x, y);
                }
            }
        }
    }
    world.queuedDeco.emplace_back([mossLocations](Random &, World &world) {
        for (auto [x, y] : mossLocations) {
            growMossOn(x, y, world);
        }
    });
}

void genGlowingMoss(Random &rnd, World &world)
{
    std::cout << "Energizing moss\n";
    int numCaves = world.conf.glowingMossFreq * rnd.getDouble(2, 9);
    for (int i = 0; i < numCaves; ++i) {
        fillGlowingMossCave(rnd, world);
    }
}
