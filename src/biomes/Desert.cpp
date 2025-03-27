#include "Snow.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>
#include <map>

void genDesert(Random &rnd, World &world)
{
    std::cout << "Desertification\n";
    rnd.shuffleNoise();
    double center = world.desertCenter;
    double scanDist = 0.08 * world.getWidth();
    double desertFloor =
        (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5;
    std::map<int, int> sandWalls{
        {WallID::Unsafe::marble, WallID::Unsafe::marble}};
    for (int wallId : WallVariants::dirt) {
        sandWalls[wallId] = rnd.select(
            {WallID::Unsafe::sandstone, WallID::Unsafe::hardenedSand});
    }
    for (int x = center - scanDist; x < center + scanDist; ++x) {
        for (int y = 0; y < world.getUnderworldLevel(); ++y) {
            double threshold = std::max(
                std::abs(x - center) / 100.0 - (world.getWidth() / 1700.0),
                15 * (y - desertFloor) / world.getHeight());
            if (rnd.getCoarseNoise(x, y) < threshold) {
                continue;
            }
            Tile &tile = world.getTile(x, y);
            switch (tile.blockID) {
            case TileID::dirt:
                tile.blockID = TileID::sand;
                break;
            case TileID::stone:
            case TileID::smoothMarble:
                tile.blockID = TileID::sandstone;
                if (y > world.getCavernLevel()) {
                    if (std::abs(rnd.getCoarseNoise(x, y) + 0.23) < 0.04) {
                        tile.blockID = TileID::sand;
                    } else if (
                        std::abs(rnd.getCoarseNoise(x, y) - 0.23) < 0.04) {
                        tile.blockID = TileID::hardenedSand;
                    }
                }
                break;
            case TileID::clay:
            case TileID::mud:
                tile.blockID = TileID::desertFossil;
                break;
            case TileID::sand:
                tile.blockID = y > world.getCavernLevel() ? TileID::desertFossil
                               : y > world.getUndergroundLevel()
                                   ? TileID::hardenedSand
                                   : TileID::sand;
                break;
            default:
                break;
            }
            if (tile.blockID == TileID::sandstone) {
                tile.wallID = WallID::Unsafe::sandstone;
            } else if (
                tile.blockID == TileID::sand ||
                tile.blockID == TileID::hardenedSand) {
                tile.wallID = WallID::Unsafe::hardenedSand;
            } else {
                tile.wallID = sandWalls[tile.wallID];
            }
        }
    }
}
