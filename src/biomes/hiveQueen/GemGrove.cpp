#include "biomes/hiveQueen/GemGrove.h"

#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "biomes/GemGrove.h"
#include <iostream>

void genGemGroveHiveQueen(Random &rnd, World &world)
{
    std::cout << "Imbuing gems\n";
    rnd.restoreShuffleState();
    int noiseShuffleX = rnd.getInt(0, world.getWidth());
    int noiseShuffleY = rnd.getInt(0, world.getHeight());
    double groveSize =
        world.getWidth() * world.getHeight() / 260000 + rnd.getInt(60, 75);
    auto [x, y] = selectGemGroveLocation(groveSize, rnd, world);
    if (x == -1) {
        return;
    }
    for (int aI = -groveSize; aI < groveSize; ++aI) {
        for (int aJ = -groveSize; aJ < groveSize; ++aJ) {
            Tile &tile = world.getTile(x + aI, y + aJ);
            if (tile.flag == Flag::border) {
                continue;
            }
            auto [i, j] = getHexCentroid({x + aI, y + aJ}, 10) - Point{x, y};
            double threshold = std::min(std::hypot(i, j) / groveSize, 1.0);
            bool shouldClear =
                std::abs(rnd.getCoarseNoise(x + i, 2 * (y + j)) + 0.1) <
                    0.45 - 0.3 * threshold &&
                rnd.getFineNoise(x + i, y + j) > 4.5 * threshold - 4.66;
            bool shouldFill =
                std::max(
                    std::abs(rnd.getBlurNoise(x + i, 5 * (y + j))),
                    std::abs(rnd.getBlurNoise(
                        noiseShuffleX + x + i,
                        noiseShuffleY + 5 * (y + j)))) > 0.4;
            if (shouldClear && tile.blockID != TileID::empty) {
                tile.blockID = shouldFill ? TileID::stone : TileID::empty;
            }
        }
    }
    world.gemGrove = {x, y};
    world.gemGroveSize = groveSize;
    world.queuedDeco.emplace_back(placeGroveDecoGems);
    world.queuedTreasures.emplace_back(placeGemChest);
}
