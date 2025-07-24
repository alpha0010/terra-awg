#include "biomes/hiveQueen/GraniteCave.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>
#include <set>

void fillGraniteCaveHex(int x, int y, World &world)
{
    std::set<Point> clearCenters;
    iterateZone(
        {x, y},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [&clearCenters, &world](Point pt) {
            Point centroid = getHexCentroid(pt, 12);
            if (world.getTile(pt).blockID == TileID::empty ||
                world.getTile(centroid).blockID == TileID::empty) {
                clearCenters.insert(centroid);
            }
        });
    iterateZone(
        {x, y},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [&clearCenters, &world](Point pt) {
            Tile &tile = world.getTile(pt);
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::grass:
            case TileID::stone:
            case TileID::mud:
                tile.blockID = clearCenters.contains(getHexCentroid(pt, 12))
                                   ? TileID::empty
                                   : TileID::granite;
                break;
            case TileID::sand:
            case TileID::clay:
                tile.blockID = clearCenters.contains(getHexCentroid(pt, 12))
                                   ? TileID::empty
                                   : TileID::smoothGranite;
                break;
            }
            if ((!world.conf.shattered && pt.y < world.getUnderworldLevel()) ||
                tile.wallID != WallID::empty) {
                tile.wallID = WallID::Unsafe::granite;
            }
        });
}

void genGraniteCaveHiveQueen(Random &rnd, World &world)
{
    std::cout << "Smoothing granite\n";
    int numCaves =
        world.conf.graniteFreq * world.getWidth() * world.getHeight() / 2000000;
    int scanDist = world.conf.graniteSize * 90;
    for (int iter = 0; iter < numCaves; ++iter) {
        auto [x, y] = findStoneCave(
            std::midpoint(world.getUndergroundLevel(), world.getCavernLevel()),
            world.getUnderworldLevel(),
            rnd,
            world,
            30);
        if (x != -1 && world.getTile(x, y).flag != Flag::border) {
            fillGraniteCaveHex(x, y, world);
            for (int probes = world.conf.graniteSize * 20; probes > 0;
                 --probes) {
                int i = rnd.getInt(-scanDist, scanDist);
                int j = rnd.getInt(-scanDist, scanDist);
                if (world.getBiome(x + i, y + j).active == Biome::forest &&
                    world.getTile(x + i, y + j).blockID == TileID::stone) {
                    fillGraniteCaveHex(x + i, y + j, world);
                }
            }
        }
    }
}
