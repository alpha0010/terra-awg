#include "Jungle.h"

#include "Random.h"
#include "World.h"
#include <iostream>

void genJungle(Random &rnd, World &world)
{
    std::cout << "Generating jungle\n";
    rnd.shuffleNoise();
    double center = world.jungleCenter;
    double scanDist = 0.11 * world.getWidth();
    for (int x = center - scanDist; x < center + scanDist; ++x) {
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
            }
            switch (tile.blockID) {
            case TileID::dirt:
            case TileID::stone:
                threshold =
                    std::abs(x - center) / 260.0 - (world.getWidth() / 2700.0);
                if (rnd.getFineNoise(x, y) > threshold) {
                    tile.blockID = world.isExposed(x, y) ? TileID::jungleGrass
                                                         : TileID::mud;
                }
                break;
            case TileID::grass:
                tile.blockID = TileID::jungleGrass;
                break;
            case TileID::sand:
            case TileID::mud:
                tile.blockID = TileID::silt;
                break;
            case TileID::cloud:
                tile.blockID = TileID::rainCloud;
                break;
            default:
                break;
            }
        }
    }
}
