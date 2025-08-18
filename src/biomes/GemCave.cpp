#include "GemCave.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>

/**
 * Concentrated deposits of two gem types, and associated gem walls for added
 * visibility.
 */
void fillGemCave(
    int x,
    int y,
    int blockA,
    int wallA,
    int blockB,
    int wallB,
    Random &rnd,
    World &world)
{
    int caveSize = rnd.getInt(20, 35);
    for (int i = -caveSize; i < caveSize; ++i) {
        for (int j = -caveSize; j < caveSize; ++j) {
            double threshold = std::max(std::hypot(i, j) / caveSize, 0.35);
            Tile &tile = world.getTile(x + i, y + j);
            // Intermix regular stone within gem deposits.
            int dispersal =
                static_cast<int>(99999 * (1 + rnd.getFineNoise(x + i, y + j))) %
                5;
            if (rnd.getFineNoise(x + i, y + j) > threshold) {
                if (dispersal > 1 && tile.blockID == TileID::stone) {
                    tile.blockID = world.conf.hiveQueen && dispersal < 4
                                       ? TileID::crispyHoney
                                       : blockA;
                }
                if (tile.wallID != WallID::empty) {
                    tile.wallID = wallA;
                }
            } else if (rnd.getFineNoise(x + i, y + j) < -threshold) {
                if (dispersal > 1 && tile.blockID == TileID::stone) {
                    tile.blockID = world.conf.hiveQueen && dispersal < 4
                                       ? TileID::crispyHoney
                                       : blockB;
                }
                if (tile.wallID != WallID::empty) {
                    tile.wallID = wallB;
                }
            }
        }
    }
}

void genGemCave(Random &rnd, World &world)
{
    std::cout << "Burying gems\n";
    rnd.shuffleNoise();
    // Place higher value gems deeper. Excludes amber.
    std::vector<std::pair<int, int>> gemTypes{
        {TileID::amethystStone, WallID::Unsafe::amethystStone},
        {TileID::amethystStone, WallID::Unsafe::amethystStone},
        {TileID::topazStone, WallID::Unsafe::topazStone},
        {TileID::sapphireStone, WallID::Unsafe::sapphireStone},
        {TileID::emeraldStone, WallID::Unsafe::emeraldStone},
        {TileID::rubyStone, WallID::Unsafe::rubyStone},
        {TileID::diamondStone, WallID::Unsafe::diamondStone},
        {TileID::diamondStone, WallID::Unsafe::diamondStone}};
    if (world.conf.dontDigUp) {
        std::reverse(gemTypes.begin(), gemTypes.end());
    }
    int bandHeight =
        ((world.conf.dontDigUp
              ? (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3
              : world.getUnderworldLevel()) -
         world.getUndergroundLevel()) /
        gemTypes.size();
    int numCaves =
        world.conf.gems * world.getWidth() * world.getHeight() / 900000;
    for (size_t band = 0; band + 1 < gemTypes.size(); ++band) {
        for (int i = 0; i < numCaves; ++i) {
            auto [x, y] = findStoneCave(
                world.getUndergroundLevel() + bandHeight * band,
                world.getUndergroundLevel() + bandHeight * (band + 1),
                rnd,
                world);
            if (x != -1) {
                fillGemCave(
                    x,
                    y,
                    gemTypes[band].first,
                    gemTypes[band].second,
                    gemTypes[band + 1].first,
                    gemTypes[band + 1].second,
                    rnd,
                    world);
            }
        }
    }
}
