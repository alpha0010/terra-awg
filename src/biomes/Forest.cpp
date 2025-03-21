#include "Forest.h"

#include "Random.h"
#include "World.h"
#include <iostream>

void genForest(Random &rnd, World &world)
{
    std::cout << "Nurturing forest\n";
    rnd.shuffleNoise();
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getUndergroundLevel(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::dirt) {
                if (world.isExposed(x, y) || rnd.getInt(0, 100) == 0) {
                    tile.blockID = TileID::grass;
                }
            } else if (
                tile.blockID == TileID::empty &&
                tile.wallID == WallID::Unsafe::dirt) {
                tile.wallID = rnd.getFineNoise(x, y) > 0
                                  ? WallID::Unsafe::grass
                                  : WallID::Unsafe::flower;
            }
        }
    }
}
