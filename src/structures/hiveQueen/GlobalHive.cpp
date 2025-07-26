#include "structures/hiveQueen/GlobalHive.h"

#include "Config.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
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
                        if (!world.conf.unpainted) {
                            tile.blockPaint = Paint::deepYellow;
                        }
                        break;
                    case Flag::orange:
                        if (!world.conf.unpainted) {
                            tile.blockPaint = Paint::deepOrange;
                        }
                        break;
                    default:
                        break;
                    }
                } else if (
                    tile.blockID == TileID::mud && tile.flag == Flag::hive &&
                    world.getBiome(x, y).active == Biome::jungle) {
                    std::vector<Point> locations;
                    iterateZone(
                        {x, y},
                        world,
                        [centroid = getHexCentroid(x, y, 10)](Point pt) {
                            return centroid == getHexCentroid(pt, 10);
                        },
                        [&locations, &world](Point pt) {
                            Tile &tile = world.getTile(pt);
                            if (tile.blockID == TileID::mud &&
                                tile.flag == Flag::hive && !tile.actuated &&
                                tile.blockPaint == Paint::none) {
                                locations.push_back(pt);
                            }
                        });
                    if (locations.size() > 115) {
                        for (Point pt : locations) {
                            world.getTile(pt).blockID = TileID::hive;
                        }
                    } else {
                        for (Point pt : locations) {
                            world.getTile(pt).flag = Flag::none;
                        }
                    }
                }
            }
        });
}
