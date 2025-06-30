#include "Snow.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>
#include <map>

void genSnow(Random &rnd, World &world)
{
    std::cout << "Freezing land\n";
    rnd.shuffleNoise();
    double center = world.snowCenter;
    double scanDist = world.conf.snowSize * 0.08 * world.getWidth();
    double snowFloor =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    std::map<int, int> snowWalls{{WallID::Safe::cloud, WallID::Safe::cloud}};
    for (int wallId : WallVariants::dirt) {
        snowWalls[wallId] = rnd.select(
            {WallID::Unsafe::snow,
             WallID::Unsafe::snow,
             WallID::Unsafe::ice,
             WallID::Unsafe::ice,
             rnd.select(WallVariants::stone)});
    }
    parallelFor(
        std::views::iota(
            std::max<int>(center - scanDist, 0),
            std::min<int>(center + scanDist, world.getWidth())),
        [center, snowFloor, &snowWalls, &rnd, &world](int x) {
            for (int y = 0; y < world.getUnderworldLevel(); ++y) {
                double threshold = std::max(
                    std::abs(x - center) / 100.0 -
                        (world.conf.snowSize * world.getWidth() / 1700.0),
                    15 * (y - snowFloor) / world.getHeight());
                if (rnd.getCoarseNoise(x, y) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x, y);
                switch (tile.blockID) {
                case TileID::dirt:
                case TileID::smoothMarble:
                    tile.blockID = TileID::snow;
                    tile.wallID = WallID::Unsafe::snow;
                    break;
                case TileID::stone:
                    tile.blockID = TileID::ice;
                    tile.wallID = WallID::Unsafe::ice;
                    break;
                case TileID::clay:
                    tile.blockID = TileID::stone;
                    break;
                case TileID::sand:
                    tile.blockID = TileID::thinIce;
                    break;
                case TileID::mud:
                    tile.blockID = TileID::slush;
                    break;
                case TileID::cloud:
                    tile.blockID = TileID::snowCloud;
                    break;
                default:
                    break;
                }
                auto itr = snowWalls.find(tile.wallID);
                if (itr != snowWalls.end()) {
                    tile.wallID = itr->second;
                }
                threshold = std::max(
                    threshold,
                    15.0 * (world.getCavernLevel() - y) / world.getHeight());
                if (std::abs(rnd.getCoarseNoise(2 * x, y) + 0.1) < 0.12 &&
                    rnd.getFineNoise(x, y) > std::max(-0.1, 1 + threshold)) {
                    tile.blockID = tile.blockID == TileID::snow
                                       ? TileID::thinIce
                                       : TileID::empty;
                }
            }
        });
}
