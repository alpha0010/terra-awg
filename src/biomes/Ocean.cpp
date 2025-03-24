#include "Ocean.h"

#include "Random.h"
#include "World.h"
#include <iostream>

void genOceans(Random &rnd, World &world)
{
    std::cout << "Filling oceans\n";
    rnd.shuffleNoise();
    int waterTable = 0;
    while (world.getTile(300, waterTable).blockID == TileID::empty ||
           world.getTile(world.getWidth() - 300, waterTable).blockID ==
               TileID::empty) {
        ++waterTable;
    }
    waterTable += rnd.getInt(4, 12);
    for (int x = 0; x < 390; ++x) {
        double drop = 90 * (1 - 1 / (1 + std::exp(0.041 * (200 - x))));
        double sandDepth = (40 + 9 * rnd.getCoarseNoise(x, 0)) *
                           std::min(1.0, (400.0 - x) / 160);
        auto fillColumn = [&](int effectiveX) {
            for (int y = 0.3 * world.getUndergroundLevel();
                 y < world.getUndergroundLevel();
                 ++y) {
                if (world.getTile(effectiveX, y).blockID != TileID::empty) {
                    for (int i = 0; i < drop; ++i) {
                        Tile &tile = world.getTile(effectiveX, y + i);
                        tile.wallID = WallID::empty;
                        tile.blockID = TileID::empty;
                        if (y + i > waterTable) {
                            tile.liquid = Liquid::water;
                        }
                    }
                    for (int i = drop + 1; i < drop + sandDepth; ++i) {
                        Tile &tile = world.getTile(effectiveX, y + i);
                        if (tile.wallID == WallID::Unsafe::dirt) {
                            tile.wallID = WallID::Unsafe::wornStone;
                        } else {
                            tile.wallID = WallID::empty;
                        }
                        if (y + i > waterTable + 20) {
                            if (tile.blockID == TileID::stone) {
                                tile.blockID = TileID::coralstone;
                            } else {
                                tile.blockID = TileID::sand;
                            }
                        } else if (tile.blockID != TileID::dirt) {
                            tile.blockID = TileID::shellPile;
                        } else {
                            tile.blockID = TileID::sand;
                        }
                    }
                    break;
                }
            }
        };
        fillColumn(x);
        fillColumn(world.getWidth() - x - 1);
    }
}
