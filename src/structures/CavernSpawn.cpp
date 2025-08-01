#include "structures/CavernSpawn.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>

Point selectCavernSpawn(Random &rnd, World &world)
{
    int minX = 0.12 * world.getWidth();
    int maxX = world.getWidth() - minX;
    int minY = world.getCavernLevel();
    int maxY = (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::empty,        TileID::dirt,          TileID::stone,
         TileID::clay,         TileID::mud,           TileID::sand,
         TileID::granite,      TileID::smoothGranite, TileID::marble,
         TileID::smoothMarble, TileID::ironOre,       TileID::leadOre,
         TileID::silverOre,    TileID::tungstenOre,   TileID::goldOre,
         TileID::platinumOre,  TileID::cobaltOre,     TileID::palladiumOre,
         TileID::mythrilOre,   TileID::orichalcumOre, TileID::adamantiteOre,
         TileID::titaniumOre});
    auto isAcceptableTile = [&clearableTiles](Tile &tile) {
        return !tile.guarded && (clearableTiles.contains(tile.blockID) ||
                                 tile.flag == Flag::border);
    };
    for (int tries = 0; tries < 20000; ++tries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        int numFilled = 0;
        int numEmpty = 0;
        if (world.regionPasses(
                x - 20,
                y - 14,
                40,
                15,
                [&numFilled, &isAcceptableTile](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < 100 && isAcceptableTile(tile);
                }) &&
            world.regionPasses(
                x - 16,
                y + 1,
                32,
                4,
                [&numEmpty, &isAcceptableTile](Tile &tile) {
                    if (tile.blockID == TileID::empty) {
                        ++numEmpty;
                    }
                    return numEmpty < 30 && isAcceptableTile(tile);
                })) {
            return {x, y};
        }
    }
    return {-1, -1};
}

int getNearestWall(Point pos, World &world)
{
    int i = 0;
    int j = 0;
    for (int iter = 0; iter < 225; ++iter) {
        int wallId = world.getTile(pos + Point{i, j}).wallID;
        if (wallId != WallID::empty) {
            return wallId;
        }
        if (std::abs(i) > std::abs(j) || (i == j && i < 0)) {
            j += i < 0 ? 1 : -1;
        } else {
            i += j < 0 ? -1 : 1;
        }
    }
    return WallID::Unsafe::dirt;
}

void genCavernSpawn(Random &rnd, World &world)
{
    std::cout << "Beginning expedition\n";
    Point pos = selectCavernSpawn(rnd, world);
    if (pos.x == -1) {
        return;
    }
    world.spawn = pos;
    for (int i = -15; i < 15; ++i) {
        int maxJ = 1 - std::abs(i) / 5;
        for (int j = -15; j < maxJ; ++j) {
            if (std::hypot(i, j) < 15) {
                world.getTile(pos + Point{i, j}).blockID = TileID::empty;
            }
        }
    }
    if (!world.conf.home) {
        for (Point delta :
             {Point{-11, -4},
              {-8, -8},
              {-5, -10},
              {0, -12},
              {5, -10},
              {8, -8},
              {11, -4}}) {
            Tile &tile = world.getTile(pos + delta);
            tile.blockID = TileID::torch;
            tile.wallID = getNearestWall(pos + delta, world);
        }
    }
}
