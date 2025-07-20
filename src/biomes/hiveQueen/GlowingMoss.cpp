#include "biomes/hiveQueen/GlowingMoss.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>
#include <set>

Point selectGlowingMossLocation(
    std::set<Point> &visited,
    Random &rnd,
    World &world)
{
    for (int tries = 0; tries < 500; ++tries) {
        int x = rnd.getInt(75, world.getWidth() - 75);
        int y = rnd.getInt(
            std::midpoint(world.getUndergroundLevel(), world.getCavernLevel()),
            world.getUnderworldLevel());
        if (!visited.contains({x, y}) &&
            world.getBiome(x, y).active == Biome::forest &&
            world.regionPasses(x - 4, y - 4, 8, 8, [](Tile &tile) {
                return tile.blockID == TileID::stone;
            })) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void fillGlowingMossHex(
    int x,
    int y,
    int mossType,
    std::set<Point> &visited,
    Random &rnd,
    World &world)
{
    iterateZone(
        {x, y},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [&visited, &rnd, &world](Point pt) {
            visited.insert(pt);
            Tile &tile = world.getTile(pt);
            if (tile.blockID == TileID::dirt || tile.blockID == TileID::stone) {
                Point centroid = getHexCentroid(pt, 6);
                if (std::max(
                        std::abs(
                            rnd.getBlurNoise(5 * centroid.x, 5 * centroid.y)),
                        std::abs(
                            rnd.getBlurNoise(7 * centroid.x, 7 * centroid.y))) >
                    0.6) {
                    tile.blockID = TileID::empty;
                }
            }
        });
    std::map<int, int> wallRepl;
    for (int wallId : WallVariants::dirt) {
        wallRepl[wallId] = mossType == TileID::lavaMossStone
                               ? rnd.select(WallVariants::underworld)
                               : rnd.select(WallVariants::stone);
    }
    std::vector<Point> mossLocations;
    iterateZone(
        {x, y},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [mossType, &wallRepl, &mossLocations, &world](Point pt) {
            Tile &tile = world.getTile(pt);
            auto itr = wallRepl.find(tile.wallID);
            if (itr != wallRepl.end()) {
                tile.wallID = itr->second;
            }
            if (tile.blockID == TileID::stone && tile.flag != Flag::hive &&
                tile.flag != Flag::crispyHoney && world.isExposed(pt.x, pt.y)) {
                tile.blockID = mossType;
                mossLocations.push_back(pt);
            }
        });
    world.queuedDeco.emplace_back([mossLocations](Random &, World &world) {
        for (auto [x, y] : mossLocations) {
            growMossOn(x, y, world);
        }
    });
}

void genGlowingMossHiveQueen(Random &rnd, World &world)
{
    std::cout << "Energizing moss\n";
    int numCaves = world.conf.glowingMossFreq * rnd.getDouble(2, 9);
    int scanDist = world.conf.glowingMossSize * 48;
    std::set<Point> visited;
    for (int iter = 0; iter < numCaves; ++iter) {
        auto [x, y] = selectGlowingMossLocation(visited, rnd, world);
        if (x == -1) {
            continue;
        }
        int mossType = rnd.select(
            {TileID::kryptonMossStone,
             TileID::xenonMossStone,
             TileID::argonMossStone,
             TileID::neonMossStone});
        if (y > (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5) {
            mossType = rnd.select({mossType, (int)TileID::lavaMossStone});
        }
        rnd.shuffleNoise();
        fillGlowingMossHex(x, y, mossType, visited, rnd, world);
        for (int probes = world.conf.glowingMossSize * 10; probes > 0;
             --probes) {
            int i = rnd.getInt(-scanDist, scanDist);
            int j = rnd.getInt(-scanDist, scanDist);
            if (!visited.contains({x + i, y + j}) &&
                world.getBiome(x + i, y + j).active == Biome::forest &&
                world.getTile(x + i, y + j).blockID == TileID::stone) {
                fillGlowingMossHex(x + i, y + j, mossType, visited, rnd, world);
            }
        }
    }
}
