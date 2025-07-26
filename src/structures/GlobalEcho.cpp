#include "structures/GlobalEcho.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
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
                }
                if (tile.wallID != WallID::empty) {
                    tile.echoCoatWall = true;
                }
            }
        });
    for (int i = -25; i < 25; ++i) {
        for (int j = -25; j < 25; ++j) {
            if (std::hypot(i, j) < 25) {
                Tile &tile = world.getTile(world.spawn + Point{i, j});
                tile.echoCoatBlock = false;
                tile.echoCoatWall = false;
            }
        }
    }
}
