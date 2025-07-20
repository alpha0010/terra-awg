#include "biomes/hiveQueen/Aether.h"

#include "Random.h"
#include "World.h"
#include "biomes/Aether.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include <iostream>

Point selectAetherLocation(Random &rnd, World &world)
{
    while (true) {
        int x = world.getWidth() * rnd.getDouble(0.08, 0.30);
        if (rnd.getBool()) {
            x = world.getWidth() - x;
        }
        int y = rnd.getInt(
            (world.getUndergroundLevel() + 2 * world.getCavernLevel()) / 3,
            (world.getCavernLevel() + 5 * world.getUnderworldLevel()) / 6);
        if (world.regionPasses(x - 50, y - 50, 100, 100, [](Tile &tile) {
                return tile.flag != Flag::border;
            })) {
            return {x, y};
        }
    }
}

void genAetherHiveQueen(Random &rnd, World &world)
{
    std::cout << "Bridging realities\n";
    rnd.shuffleNoise();
    auto [x, y] = selectAetherLocation(rnd, world);
    int maxBubblePos = y;
    int maxEditPos = y;
    std::vector<Point> mossLocations;
    iterateZone(
        {x, y},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [x, y, &maxBubblePos, &maxEditPos, &mossLocations, &rnd, &world](
            Point pt) {
            double centralPropo = hypot(pt, {x, y}) / 75;
            auto blurHexNoise = [&rnd](int a, int b) {
                Point centroid = getHexCentroid(a, b, 10);
                return rnd.getBlurNoise(4 * centroid.x, 4 * centroid.y);
            };
            double noiseVal = std::max(
                                  std::abs(blurHexNoise(pt.x, pt.y)),
                                  std::abs(blurHexNoise(pt.x + x, pt.y + y))) *
                              std::min(1.0, 3 * (1 - centralPropo));
            Tile &tile = world.getTile(pt);
            if (tile.flag == Flag::yellow || tile.flag == Flag::orange) {
                tile.flag = Flag::none;
            }
            if (noiseVal > 0.45) {
                tile.blockID = TileID::bubble;
                maxBubblePos = std::max(maxBubblePos, pt.y);
            } else if (noiseVal > 0.09) {
                tile.blockID = TileID::empty;
            } else if (noiseVal > 0.02) {
                tile.blockID = tile.blockID == TileID::empty ? TileID::aetherium
                               : centralPropo < 0.6 ? TileID::heliumMossStone
                                                    : TileID::stone;
                mossLocations.push_back(pt);
            }
            if (noiseVal > 0.019) {
                tile.wallID = WallID::empty;
                maxEditPos = std::max(maxEditPos, pt.y);
            }
        });
    world.queuedDeco.emplace_back([mossLocations](Random &, World &world) {
        for (auto [x, y] : mossLocations) {
            growMossOn(x, y, world);
        }
    });
    fillAetherShimmer(x, y, 75, maxBubblePos, maxEditPos, rnd, world);
}
