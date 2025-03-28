#include "BiomeUtil.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"

void fillLargeWallGaps(Point from, Point to, Random &rnd, World &world)
{
    int yMax = std::min(to.second, world.getUnderworldLevel());
    for (int x = from.first; x < to.first; ++x) {
        for (int y = from.second; y < yMax; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.wallID == WallID::empty && rnd.getFineNoise(x, y) < 0.55) {
                tile.wallID = WallID::Unsafe::dirt;
            }
        }
    }
}

Point findStoneCave(int yMin, int yMax, Random &rnd, World &world, int minSize)
{
    int numTries = 0;
    while (true) {
        if (numTries % 100 == 99 && minSize > 3) {
            --minSize;
        }
        ++numTries;
        int x = rnd.getInt(50, world.getWidth() - 50);
        int y = rnd.getInt(yMin, yMax);
        if (world.getTile(x, y).blockID != TileID::empty) {
            continue;
        }
        int caveRoof = y - 1;
        while (caveRoof > 0 &&
               world.getTile(x, caveRoof).blockID == TileID::empty) {
            --caveRoof;
        }
        if (world.getTile(x, caveRoof).blockID != TileID::stone) {
            continue;
        }
        int caveFloor = y + 1;
        while (caveFloor < world.getHeight() &&
               world.getTile(x, caveFloor).blockID == TileID::empty) {
            ++caveFloor;
        }
        if (world.getTile(x, caveFloor).blockID != TileID::stone ||
            caveFloor - caveRoof < minSize) {
            continue;
        }
        int left = x - 1;
        while (left > 0 && world.getTile(left, y).blockID == TileID::empty) {
            --left;
        }
        if (world.getTile(left, y).blockID != TileID::stone) {
            continue;
        }
        int right = x + 1;
        while (right < world.getWidth() &&
               world.getTile(right, y).blockID == TileID::empty) {
            ++right;
        }
        if (world.getTile(right, y).blockID != TileID::stone ||
            right - left < minSize) {
            continue;
        }
        return {(left + right) / 2, (caveFloor + caveRoof) / 2};
    }
}
