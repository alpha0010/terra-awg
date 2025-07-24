#include "biomes/hiveQueen/MarbleCave.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>

void fillMarbleCaveHex(int x, int y, Random &rnd, World &world)
{
    int lastX = -1;
    int stalactiteLen = 0;
    int stalacIter = 0;
    iterateZone(
        {x, y},
        world,
        [&world](Point pt) { return world.getTile(pt).flag != Flag::border; },
        [&](Point pt) {
            if (pt.x != lastX) {
                lastX = pt.x;
                stalactiteLen = 0;
                stalacIter = 0;
            }
            Tile &tile = world.getTile(pt);
            bool nextTileIsEmpty =
                world.getTile(pt.x, pt.y + 1).blockID == TileID::empty;
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::stone:
            case TileID::mud:
                tile.blockID = TileID::marble;
                if (nextTileIsEmpty) {
                    stalactiteLen = std::max(
                        0.0,
                        16 * rnd.getFineNoise(4 * pt.x, 100 * stalacIter));
                    ++stalacIter;
                }
                break;
            case TileID::clay:
            case TileID::sand:
                tile.blockID = TileID::smoothMarble;
                break;
            case TileID::empty:
                if (nextTileIsEmpty && stalactiteLen > 0) {
                    tile.blockID = TileID::marble;
                    --stalactiteLen;
                }
            }
            if (!world.conf.shattered || tile.wallID != WallID::empty) {
                tile.wallID = WallID::Unsafe::marble;
            }
        });
}

void genMarbleCaveHiveQueen(Random &rnd, World &world)
{
    std::cout << "Excavating marble\n";
    int numCaves =
        world.conf.marbleFreq * world.getWidth() * world.getHeight() / 1200000;
    for (int i = 0; i < numCaves; ++i) {
        auto [x, y] = findStoneCave(
            std::midpoint(world.getUndergroundLevel(), world.getCavernLevel()),
            world.getUnderworldLevel(),
            rnd,
            world,
            30);
        if (x != -1 && world.getTile(x, y).flag != Flag::border) {
            fillMarbleCaveHex(x, y, rnd, world);
        }
    }
}
