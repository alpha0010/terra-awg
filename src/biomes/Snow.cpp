#include "Snow.h"

#include "Random.h"
#include "World.h"
#include <iostream>
#include <map>

void genSnow(Random &rnd, World &world)
{
    std::cout << "Freezing land\n";
    rnd.shuffleNoise();
    double center = world.snowCenter;
    double scanDist = 0.08 * world.getWidth();
    double snowFloor =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    std::map<int, int> snowWalls;
    for (int wallId : WallVariants::dirt) {
        snowWalls[wallId] = rnd.select(
            {WallID::Unsafe::snow,
             WallID::Unsafe::ice,
             rnd.select(
                 WallVariants::stone.begin(),
                 WallVariants::stone.end())});
    }
    for (int x = center - scanDist; x < center + scanDist; ++x) {
        for (int y = 0; y < world.getUnderworldLevel(); ++y) {
            double threshold = std::max(
                std::abs(x - center) / 100.0 - (world.getWidth() / 1700.0),
                15 * (y - snowFloor) / world.getHeight());
            if (rnd.getCoarseNoise(x, y) < threshold) {
                continue;
            }
            Tile &tile = world.getTile(x, y);
            switch (tile.blockID) {
            case TileID::dirt:
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
        }
    }
}
