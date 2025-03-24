#include "Underworld.h"

#include "Random.h"
#include "World.h"
#include <iostream>

void genUnderworld(Random &rnd, World &world)
{
    std::cout << "Igniting the depths\n";
    rnd.shuffleNoise();
    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    int lavaLevel = world.getUnderworldLevel() + 0.54 * underworldHeight;
    int resourceLevel = world.getUnderworldLevel() + 0.33 * underworldHeight;
    double aspectRatio =
        static_cast<double>(world.getHeight()) / world.getWidth();
    for (int x = 0; x < world.getWidth(); ++x) {
        int stalactiteLen =
            std::max(0.0, 16 * rnd.getFineNoise(4 * x, aspectRatio * x));
        bool foundRoof = false;
        for (int y = resourceLevel;
             y > world.getUnderworldLevel() && stalactiteLen < 10;
             --y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::ash) {
                foundRoof = true;
            }
            if (foundRoof) {
                if (tile.blockID == TileID::ash) {
                    tile.blockID = TileID::empty;
                    ++stalactiteLen;
                } else {
                    break;
                }
            }
        }
        for (int y = world.getUnderworldLevel(); y < world.getHeight(); ++y) {
            double threshold = 0.25 - 0.5 *
                                          std::abs(
                                              y - underworldHeight / 2 -
                                              world.getUnderworldLevel()) /
                                          underworldHeight;
            Tile &tile = world.getTile(x, y);
            if (std::abs(rnd.getCoarseNoise(2 * x, y + aspectRatio * x)) <
                threshold) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
            }
            if (y > lavaLevel && tile.blockID == TileID::empty) {
                tile.liquid = Liquid::lava;
            }
            if (y > resourceLevel && tile.blockID == TileID::ash &&
                rnd.getFineNoise(x, y) > 0.5) {
                tile.blockID = TileID::hellstone;
            }
        }
    }
    int skipFrom = 0.15 * world.getWidth();
    int skipTo = 0.85 * world.getWidth();
    for (int x = 0; x < world.getWidth(); ++x) {
        if (x == skipFrom) {
            x = skipTo;
        }
        for (int y = world.getUnderworldLevel(); y < lavaLevel; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::ash && world.isExposed(x, y)) {
                tile.blockID = TileID::ashGrass;
            }
        }
    }
}
