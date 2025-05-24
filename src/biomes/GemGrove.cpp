#include "GemGrove.h"

#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <iostream>
#include <set>

Point selectGroveLocation(int groveSize, Random &rnd, World &world)
{
    std::set<int> allowedTiles{
        TileID::empty,
        TileID::dirt,
        TileID::stone,
        TileID::clay,
        TileID::mud,
        TileID::sand,
        TileID::ironOre,
        TileID::leadOre,
        TileID::silverOre,
        TileID::tungstenOre,
        TileID::goldOre,
        TileID::platinumOre};
    std::set<int> allowedWalls{
        WallVariants::dirt.begin(),
        WallVariants::dirt.end()};
    allowedWalls.insert(WallID::empty);
    while (true) {
        auto [x, y] = findStoneCave(
            world.getCavernLevel() + groveSize,
            world.getUnderworldLevel() - groveSize,
            rnd,
            world);
        if (world.regionPasses(
                x - groveSize,
                y - groveSize,
                2 * groveSize,
                2 * groveSize,
                [&allowedTiles, &allowedWalls](Tile &tile) {
                    return allowedTiles.contains(tile.blockID) &&
                           allowedWalls.contains(tile.wallID);
                })) {
            return {x, y};
        }
    }
}

void genGemGrove(Random &rnd, World &world)
{
    std::cout << "Imbuing gems\n";
    rnd.restoreShuffleState();
    int noiseShuffleX = rnd.getInt(0, world.getWidth());
    int noiseShuffleY = rnd.getInt(0, world.getHeight());
    int groveSize =
        world.getWidth() * world.getHeight() / 329000 + rnd.getInt(45, 75);
    auto [x, y] = selectGroveLocation(groveSize, rnd, world);
    for (int i = -groveSize; i < groveSize; ++i) {
        for (int j = -groveSize; j < groveSize; ++j) {
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
            Tile &tile = world.getTile(x + i, y + j);
            if (shouldClear && tile.blockID != TileID::empty) {
                tile.blockID = shouldFill ? TileID::stone : TileID::empty;
            }
        }
    }
    world.gemGroveX = x;
    world.gemGroveY = y;
    world.gemGroveSize = groveSize;
}
