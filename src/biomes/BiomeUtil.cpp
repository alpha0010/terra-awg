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
                // Most biome generation code coverts dirt walls to biome
                // specific variants.
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
            // Slowly reduce size requirements if finding a cave is taking too
            // long.
            --minSize;
        }
        ++numTries;
        int x = rnd.getInt(50, world.getWidth() - 50);
        int y = rnd.getInt(yMin, yMax);
        if (world.getTile(x, y).blockID != TileID::empty) {
            continue;
        }
        // Scan up.
        int caveRoof = y - 1;
        while (caveRoof > 0 &&
               world.getTile(x, caveRoof).blockID == TileID::empty) {
            --caveRoof;
        }
        if (world.getTile(x, caveRoof).blockID != TileID::stone) {
            continue;
        }
        // Scan down.
        int caveFloor = y + 1;
        while (caveFloor < world.getHeight() &&
               world.getTile(x, caveFloor).blockID == TileID::empty) {
            ++caveFloor;
        }
        if (world.getTile(x, caveFloor).blockID != TileID::stone ||
            caveFloor - caveRoof < minSize) {
            continue;
        }
        // Scan left.
        int left = x - 1;
        while (left > 0 && world.getTile(left, y).blockID == TileID::empty) {
            --left;
        }
        if (world.getTile(left, y).blockID != TileID::stone) {
            continue;
        }
        // Scan right.
        int right = x + 1;
        while (right < world.getWidth() &&
               world.getTile(right, y).blockID == TileID::empty) {
            ++right;
        }
        if (world.getTile(right, y).blockID != TileID::stone ||
            right - left < minSize) {
            continue;
        }
        // Success.
        return {(left + right) / 2, (caveFloor + caveRoof) / 2};
    }
}

inline const std::map<int, int> mossFrameX{
    {TileID::lavaMossStone, 110},
    {TileID::kryptonMossStone, 132},
    {TileID::xenonMossStone, 154},
    {TileID::argonMossStone, 176},
    {TileID::neonMossStone, 198},
    {TileID::heliumMossStone, 220}};

void growMossOn(int x, int y, World &world)
{
    Tile &baseTile = world.getTile(x, y);
    auto itr = mossFrameX.find(baseTile.blockID);
    if (itr == mossFrameX.end() || baseTile.slope != Slope::none) {
        return;
    }
    for (auto [i, j, frameY] :
         {std::tuple{0, -1, 0}, {0, 1, 54}, {1, 0, 108}, {-1, 0, 162}}) {
        Tile &tile = world.getTile(x + i, y + j);
        if (tile.blockID == TileID::empty) {
            tile.blockID = TileID::mossPlant;
            tile.frameX = itr->second;
            tile.frameY = frameY + 18 * ((x + i + y + j) % 3);
        }
    }
}
