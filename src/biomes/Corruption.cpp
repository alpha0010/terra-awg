#include "Corruption.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>
#include <map>

void genCorruption(Random &rnd, World &world)
{
    std::cout << "Corrupting the world\n";
    // Avoid selecting too near spawn.
    int surfaceX = world.getWidth() * rnd.getDouble(0.12, 0.39);
    if (rnd.getBool()) {
        surfaceX = world.getWidth() - surfaceX;
    }
    // Register location for use in other generators.
    world.surfaceEvilCenter = surfaceX;
    genCorruptionAt(
        surfaceX,
        world.getWidth() * rnd.getDouble(0.08, 0.92),
        rnd,
        world);
}

void genCorruptionAt(int surfaceX, int undergroundX, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    int surfaceY = rnd.getInt(
        0.95 * world.getUndergroundLevel(),
        (2 * world.getUndergroundLevel() + world.getCavernLevel()) / 3);
    int scanDist = 0.08 * world.getWidth();
    // Conversion mappings.
    constexpr auto corruptBlocks = frozen::make_map<int, int>(
        {{TileID::stone, TileID::ebonstone},
         {TileID::grass, TileID::corruptGrass},
         {TileID::ironOre, TileID::ebonstone},
         {TileID::leadOre, TileID::ebonstone},
         {TileID::silverOre, TileID::demonite},
         {TileID::tungstenOre, TileID::demonite},
         {TileID::goldOre, TileID::demonite},
         {TileID::platinumOre, TileID::demonite},
         {TileID::leaf, TileID::empty},
         {TileID::mahoganyLeaf, TileID::empty},
         {TileID::sand, TileID::ebonsand},
         {TileID::jungleGrass, TileID::corruptJungleGrass},
         {TileID::mushroomGrass, TileID::corruptJungleGrass},
         {TileID::silt, TileID::ebonstone},
         {TileID::slime, TileID::ebonstone},
         {TileID::ice, TileID::corruptIce},
         {TileID::slush, TileID::ebonstone},
         {TileID::sandstone, TileID::ebonsandstone},
         {TileID::hardenedSand, TileID::hardenedEbonsand}});
    std::map<int, int> corruptWalls{
        {WallID::Safe::livingLeaf, WallID::empty},
        {WallID::Unsafe::grass, WallID::Unsafe::corruptGrass},
        {WallID::Unsafe::flower, WallID::Unsafe::corruptGrass},
        {WallID::Unsafe::jungle, WallID::Unsafe::corruptGrass},
        {WallID::Unsafe::sandstone, WallID::Unsafe::ebonsandstone},
        {WallID::Unsafe::hardenedSand, WallID::Unsafe::hardenedEbonsand}};
    for (int wallId : WallVariants::dirt) {
        corruptWalls[wallId] = rnd.select(WallVariants::corruption);
    }
    for (int wallId : WallVariants::stone) {
        corruptWalls[wallId] = rnd.select(WallVariants::corruption);
    }
    for (int wallId : WallVariants::jungle) {
        corruptWalls[wallId] = rnd.select(WallVariants::corruption);
    }
    for (int wallId :
         {WallID::Unsafe::snow,
          WallID::Unsafe::ice,
          WallID::Unsafe::mushroom,
          WallID::Unsafe::marble,
          WallID::Unsafe::granite}) {
        corruptWalls[wallId] = rnd.select(WallVariants::corruption);
    }
    constexpr auto chasmSkipTiles = frozen::make_set<int>(
        {TileID::livingWood,
         TileID::leaf,
         TileID::livingMahogany,
         TileID::mahoganyLeaf});
    int scaleFactor =
        std::midpoint<int>(world.getWidth(), 3.5 * world.getHeight());
    // Dig surface chasms, edged with ebonstone.
    for (int x = surfaceX - scanDist; x < surfaceX + scanDist; ++x) {
        for (int y = 0.45 * world.getUndergroundLevel();
             y < world.getCavernLevel();
             ++y) {
            double threshold = std::min(
                {2 - 50.0 * std::abs(x - surfaceX) / scaleFactor,
                 0.01 * (world.getCavernLevel() - y),
                 0.16});
            if (std::abs(rnd.getCoarseNoise(3 * x, y) + 0.1) < threshold) {
                Tile &tile = world.getTile(x, y);
                if (chasmSkipTiles.contains(tile.blockID) ||
                    tile.wallID == WallID::Unsafe::livingWood) {
                    continue;
                }
                if (std::abs(rnd.getCoarseNoise(3 * x, y) + 0.1) <
                    threshold - 0.07) {
                    tile.blockID = TileID::empty;
                } else if (tile.blockID != TileID::empty) {
                    tile.blockID = TileID::ebonstone;
                }
            }
        }
    }
    auto applyCorruption = [&](int sourceX, int sourceY) {
        parallelFor(
            std::views::iota(sourceX - scanDist, sourceX + scanDist),
            [&, scanDist, sourceX, sourceY](int x) {
                for (int y = std::max(sourceY - scanDist, 0);
                     y < sourceY + scanDist;
                     ++y) {
                    double threshold =
                        1 - std::sqrt(
                                18 * std::hypot(x - sourceX, y - sourceY) /
                                scaleFactor);
                    if (std::abs(rnd.getCoarseNoise(x, y)) < threshold) {
                        Tile &tile = world.getTile(x, y);
                        threshold =
                            1 - std::pow(
                                    21 * std::hypot(x - sourceX, y - sourceY) /
                                        scaleFactor,
                                    0.04);
                        if (std::abs(rnd.getCoarseNoise(x, y)) < threshold) {
                            // Corruption spreads from tendrils of lesion
                            // blocks. Fill the core of central tendrils with
                            // demonite.
                            tile.blockID = std::abs(rnd.getCoarseNoise(x, y)) <
                                                   threshold - 0.065
                                               ? y > world.getUndergroundLevel()
                                                     ? TileID::demonite
                                                     : TileID::ebonstone
                                               : TileID::lesion;
                            if (tile.wallID != WallID::empty) {
                                tile.wallID = WallID::Unsafe::corruptTendril;
                            }
                            // Handle aether bubble.
                            tile.echoCoatBlock = false;
                        } else {
                            auto blockItr = corruptBlocks.find(tile.blockID);
                            if (blockItr != corruptBlocks.end()) {
                                tile.blockID = blockItr->second;
                            } else if (
                                tile.blockID == TileID::livingWood ||
                                tile.blockID == TileID::livingMahogany) {
                                tile.blockPaint = Paint::purple;
                            } else if (
                                tile.blockID == TileID::ash &&
                                y < world.getUnderworldLevel() +
                                        10 * rnd.getFineNoise(x, y) - 20) {
                                tile.blockID = TileID::ebonstone;
                            }
                            auto wallItr = corruptWalls.find(tile.wallID);
                            if (wallItr != corruptWalls.end()) {
                                tile.wallID = wallItr->second;
                            } else if (
                                tile.wallID == WallID::Unsafe::livingWood) {
                                tile.wallPaint = Paint::purple;
                            }
                        }
                    }
                }
            });
    };
    // Surface corruption.
    applyCorruption(surfaceX, surfaceY);
    // Underground corruption.
    applyCorruption(
        undergroundX,
        rnd.getInt(
            (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
            world.getUnderworldLevel()));
    // Remove high above surface unconnected tendrils.
    bool clearFloating = false;
    for (int y = 0.8 * world.getUndergroundLevel(); y > 0; --y) {
        bool foundBlock = false;
        for (int x = surfaceX - scanDist; x < surfaceX + scanDist; ++x) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::lesion) {
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
