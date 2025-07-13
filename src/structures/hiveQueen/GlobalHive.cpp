#include "structures/hiveQueen/GlobalHive.h"

#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>

void genGlobalHive(World &world)
{
    std::cout << "Globalizing bees\n";
    constexpr auto stoneTiles = frozen::make_set<int>(
        {TileID::stone,
         TileID::kryptonMossStone,
         TileID::xenonMossStone,
         TileID::argonMossStone,
         TileID::neonMossStone,
         TileID::lavaMossStone});
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [&stoneTiles, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (stoneTiles.contains(tile.blockID) && !tile.actuated &&
                    tile.blockPaint == Paint::none) {
                    switch (tile.flag) {
                    case Flag::hive:
                    case Flag::crispyHoney:
                        tile.blockID = tile.flag == Flag::hive
                                           ? TileID::hive
                                           : TileID::crispyHoney;
                        if (tile.wallID != WallID::empty &&
                            tile.wallPaint == Paint::none &&
                            !world.isExposed(x, y)) {
                            tile.wallID = WallID::Unsafe::hive;
                        }
                        break;
                    case Flag::yellow:
                        tile.blockPaint = Paint::deepYellow;
                        break;
                    case Flag::orange:
                        tile.blockPaint = Paint::deepOrange;
                        break;
                    default:
                        break;
                    }
                }
            }
        });
}
