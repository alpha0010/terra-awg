#include "structures/GlobalEcho.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>

void genGlobalEcho(Random &rnd, World &world)
{
    std::cout << "Decaying memories\n";
    rnd.shuffleNoise();
    double fadedMemories = std::clamp(world.conf.fadedMemories, 0.0, 1.0);
    double threshold =
        0.200700427 *
            std::log((fadedMemories + 0.048) / (1.048 - fadedMemories)) -
        0.000403907;
    if (fadedMemories < 0.05) {
        threshold = std::lerp(-1.12, threshold, 20 * fadedMemories);
    } else if (fadedMemories > 0.95) {
        threshold = std::lerp(threshold, 1.12, 20 * fadedMemories - 19);
    }
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [threshold, &rnd, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                if (rnd.getCoarseNoise(x, y) > threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x, y);
                if (tile.blockID != TileID::empty) {
                    tile.echoCoatBlock = true;
                    for (int i = -1; i < 2; ++i) {
                        for (int j = -1; j < 2; ++j) {
                            Tile &wallTile = world.getTile(x + i, y + j);
                            if (wallTile.wallID != WallID::empty) {
                                wallTile.echoCoatWall = true;
                            }
                        }
                    }
                } else if (tile.wallID != WallID::empty) {
                    tile.echoCoatWall = true;
                }
            }
        });
    for (int i = -25; i < 25; ++i) {
        for (int j = -25; j < 25; ++j) {
            double dist = std::hypot(i, j);
            if (dist < 25) {
                Tile &tile = world.getTile(world.spawn + Point{i, j});
                tile.echoCoatBlock = false;
                if (dist < 24) {
                    tile.echoCoatWall = false;
                }
            }
        }
    }
}
