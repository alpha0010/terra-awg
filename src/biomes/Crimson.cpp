#include "Crimson.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>

void genCrimson(Random &rnd, World &world)
{
    std::cout << "Infecting the world\n";
    rnd.shuffleNoise();
    int surfaceX = world.getWidth() * rnd.getDouble(0.12, 0.39);
    if (rnd.getBool()) {
        surfaceX = world.getWidth() - surfaceX;
    }
    int surfaceY = rnd.getInt(
        0.95 * world.getUndergroundLevel(),
        (2 * world.getUndergroundLevel() + world.getCavernLevel()) / 3);
    int scanDist = 0.08 * world.getWidth();
    std::map<int, int> crimsonBlocks{
        {TileID::stone, TileID::crimstone},
        {TileID::grass, TileID::crimsonGrass},
        {TileID::ironOre, TileID::crimtane},
        {TileID::leadOre, TileID::crimtane},
        {TileID::silverOre, TileID::crimtane},
        {TileID::tungstenOre, TileID::crimtane},
        {TileID::goldOre, TileID::crimtane},
        {TileID::platinumOre, TileID::crimtane},
        {TileID::sand, TileID::crimsand},
        {TileID::jungleGrass, TileID::crimsonJungleGrass},
        {TileID::mushroomGrass, TileID::crimsonJungleGrass},
        {TileID::silt, TileID::crimstone},
        {TileID::ice, TileID::crimsonIce},
        {TileID::slush, TileID::crimstone},
        {TileID::sandstone, TileID::crimsandstone},
        {TileID::hardenedSand, TileID::hardenedCrimsand}};
    std::map<int, int> crimsonWalls{
        {WallID::Unsafe::grass, WallID::Unsafe::crimsonGrass},
        {WallID::Unsafe::flower, WallID::Unsafe::crimsonGrass},
        {WallID::Unsafe::jungle, WallID::Unsafe::crimsonGrass},
        {WallID::Unsafe::sandstone, WallID::Unsafe::crimsandstone},
        {WallID::Unsafe::hardenedSand, WallID::Unsafe::hardenedCrimsand}};
    for (int wallId : WallVariants::dirt) {
        crimsonWalls[wallId] = rnd.select(
            WallVariants::crimson.begin(),
            WallVariants::crimson.end());
    }
    for (int wallId : WallVariants::stone) {
        crimsonWalls[wallId] = rnd.select(
            WallVariants::crimson.begin(),
            WallVariants::crimson.end());
    }
    for (int wallId : WallVariants::jungle) {
        crimsonWalls[wallId] = rnd.select(
            WallVariants::crimson.begin(),
            WallVariants::crimson.end());
    }
    for (int wallId :
         {WallID::Unsafe::snow,
          WallID::Unsafe::ice,
          WallID::Unsafe::mushroom}) {
        crimsonWalls[wallId] = rnd.select(
            WallVariants::crimson.begin(),
            WallVariants::crimson.end());
    }
    std::set<int> tunnelBorderExclusions{
        TileID::empty,
        TileID::ironOre,
        TileID::leadOre,
        TileID::silverOre,
        TileID::tungstenOre};
    for (int x = surfaceX - scanDist; x < surfaceX + scanDist; ++x) {
        for (int y = 0.4 * world.getUndergroundLevel();
             y < world.getCavernLevel();
             ++y) {
            double threshold = std::min(
                {2 - 50.0 * std::abs(x - surfaceX) / world.getWidth(),
                 0.01 * (world.getCavernLevel() - y),
                 0.16});
            if (std::abs(rnd.getBlurNoise(2 * x, 2 * y) + 0.1) < threshold) {
                Tile &tile = world.getTile(x, y);
                if (std::abs(rnd.getBlurNoise(2 * x, 2 * y) + 0.1) <
                    threshold - 0.07) {
                    tile.blockID = TileID::empty;
                } else if (!tunnelBorderExclusions.contains(tile.blockID)) {
                    tile.blockID = TileID::crimstone;
                }
            }
        }
    }
    auto applyCrimson = [&](int sourceX, int sourceY) {
        parallelFor(
            std::views::iota(sourceX - scanDist, sourceX + scanDist),
            [&, scanDist, sourceX, sourceY](int x) {
                for (int y = sourceY - scanDist; y < sourceY + scanDist; ++y) {
                    double threshold =
                        1 - std::sqrt(
                                18 * std::hypot(x - sourceX, y - sourceY) /
                                world.getWidth());
                    if (std::abs(rnd.getBlurNoise(x, y)) < threshold) {
                        Tile &tile = world.getTile(x, y);
                        threshold =
                            1 - std::pow(
                                    21 * std::hypot(x - sourceX, y - sourceY) /
                                        world.getWidth(),
                                    0.028);
                        if (std::abs(rnd.getBlurNoise(x, y)) < threshold) {
                            tile.blockID = std::abs(rnd.getBlurNoise(x, y)) <
                                                   threshold - 0.045
                                               ? y > world.getUndergroundLevel()
                                                     ? TileID::crimtane
                                                     : TileID::crimstone
                                               : TileID::flesh;
                            if (tile.wallID != WallID::empty) {
                                tile.wallID = WallID::Unsafe::crimsonBlister;
                            }
                            tile.echoCoatBlock = false;
                        } else {
                            auto blockItr = crimsonBlocks.find(tile.blockID);
                            if (blockItr != crimsonBlocks.end()) {
                                tile.blockID = blockItr->second;
                            }
                            auto wallItr = crimsonWalls.find(tile.wallID);
                            if (wallItr != crimsonWalls.end()) {
                                tile.wallID = wallItr->second;
                            }
                        }
                    }
                }
            });
    };
    applyCrimson(surfaceX, surfaceY);
    applyCrimson(
        world.getWidth() * rnd.getDouble(0.08, 0.92),
        rnd.getInt(
            (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
            world.getUnderworldLevel()));
    bool clearFloating = false;
    for (int y = 0.8 * world.getUndergroundLevel(); y > 0; --y) {
        bool foundBlock = false;
        for (int x = surfaceX - scanDist; x < surfaceX + scanDist; ++x) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::flesh) {
                if (clearFloating) {
                    tile.blockID = TileID::empty;
                } else {
                    foundBlock = true;
                    break;
                }
            }
        }
        if (!foundBlock) {
            clearFloating = true;
        }
    }
}
