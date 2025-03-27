#include "Jungle.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>
#include <map>

void genJungle(Random &rnd, World &world)
{
    std::cout << "Generating jungle\n";
    rnd.shuffleNoise();
    double center = world.jungleCenter;
    double scanDist = 0.11 * world.getWidth();
    std::map<int, int> surfaceJungleWalls{{WallID::empty, WallID::empty}};
    for (int wallId : WallVariants::dirt) {
        surfaceJungleWalls[wallId] = rnd.select(
            {WallID::Unsafe::jungle,
             WallID::Unsafe::jungle,
             rnd.select(WallVariants::stone)});
    }
    std::map<int, int> undergroundJungleWalls;
    for (int wallId : WallVariants::dirt) {
        undergroundJungleWalls[wallId] = rnd.select(WallVariants::jungle);
    }
    for (int x = center - scanDist; x < center + scanDist; ++x) {
        int lastTileID = TileID::empty;
        for (int y = 0; y < world.getHeight(); ++y) {
            double threshold =
                std::abs(x - center) / 100.0 - (world.getWidth() / 1050.0);
            if (rnd.getCoarseNoise(x, y) < threshold) {
                continue;
            }
            Tile &tile = world.getTile(x, y);
            threshold =
                2.0 * (y - world.getCavernLevel()) * (y - world.getHeight()) /
                    std::pow(world.getHeight() - world.getCavernLevel(), 2) +
                0.75;
            if (y > world.getCavernLevel() &&
                rnd.getCoarseNoise(2 * x, 2 * y) > threshold) {
                tile.blockID = TileID::empty;
                if (lastTileID == TileID::mud) {
                    Tile &prevTile = world.getTile(x, y - 1);
                    if (prevTile.blockID == TileID::mud) {
                        prevTile.blockID = TileID::jungleGrass;
                    }
                }
            }
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::stone:
                threshold =
                    std::abs(x - center) / 260.0 - (world.getWidth() / 2700.0);
                if (rnd.getFineNoise(x, y) > threshold) {
                    tile.blockID = world.isExposed(x, y) ? TileID::jungleGrass
                                                         : TileID::mud;
                    if (y < world.getUndergroundLevel() &&
                        tile.blockID == TileID::mud &&
                        static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                                100 ==
                            0) {
                        tile.blockID = TileID::jungleGrass;
                    }
                }
                break;
            case TileID::grass:
                tile.blockID = TileID::jungleGrass;
                break;
            case TileID::sand:
            case TileID::smoothMarble:
                tile.blockID = TileID::silt;
                break;
            case TileID::mud:
                tile.blockID = TileID::stone;
                break;
            case TileID::cloud:
                tile.blockID = TileID::rainCloud;
                break;
            default:
                break;
            }
            if (y < world.getUndergroundLevel()) {
                if (tile.blockID == TileID::empty) {
                    tile.wallID = surfaceJungleWalls[tile.wallID];
                } else {
                    tile.wallID = WallID::Unsafe::mud;
                }
            } else {
                auto itr = undergroundJungleWalls.find(tile.wallID);
                if (itr != undergroundJungleWalls.end()) {
                    tile.wallID = itr->second;
                }
            }
            lastTileID = tile.blockID;
        }
    }
}
