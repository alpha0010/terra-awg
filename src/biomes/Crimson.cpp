#include "Crimson.h"

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

void genCrimson(Random &rnd, World &world)
{
    std::cout << "Infecting the world\n";
    // Avoid selecting too near spawn.
    int surfaceX = world.getWidth() * rnd.getDouble(0.12, 0.39);
    if (rnd.getBool()) {
        surfaceX = world.getWidth() - surfaceX;
    }
    // Register location for use in other generators.
    world.surfaceEvilCenter = surfaceX;
    genCrimsonAt(
        surfaceX,
        world.getWidth() * rnd.getDouble(0.08, 0.92),
        rnd,
        world);
}

void genCrimsonAt(int surfaceX, int undergroundX, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    int surfaceY = rnd.getInt(
        0.95 * world.getUndergroundLevel(),
        (2 * world.getUndergroundLevel() + world.getCavernLevel()) / 3);
    int scanDist = 0.08 * world.getWidth();
    // Conversion mappings.
    constexpr auto crimsonBlocks = frozen::make_map<int, int>(
        {{TileID::stone, TileID::crimstone},
         {TileID::grass, TileID::crimsonGrass},
         {TileID::ironOre, TileID::crimstone},
         {TileID::leadOre, TileID::crimstone},
         {TileID::silverOre, TileID::crimtane},
         {TileID::tungstenOre, TileID::crimtane},
         {TileID::goldOre, TileID::crimtane},
         {TileID::platinumOre, TileID::crimtane},
         {TileID::leaf, TileID::empty},
         {TileID::mahoganyLeaf, TileID::empty},
         {TileID::sand, TileID::crimsand},
         {TileID::jungleGrass, TileID::crimsonJungleGrass},
         {TileID::mushroomGrass, TileID::crimsonJungleGrass},
         {TileID::silt, TileID::crimstone},
         {TileID::slime, TileID::crimstone},
         {TileID::ice, TileID::crimsonIce},
         {TileID::slush, TileID::crimstone},
         {TileID::sandstone, TileID::crimsandstone},
         {TileID::hardenedSand, TileID::hardenedCrimsand}});
    std::map<int, int> crimsonWalls{
        {WallID::Safe::livingLeaf, WallID::empty},
        {WallID::Unsafe::grass, WallID::Unsafe::crimsonGrass},
        {WallID::Unsafe::flower, WallID::Unsafe::crimsonGrass},
        {WallID::Unsafe::jungle, WallID::Unsafe::crimsonGrass},
        {WallID::Unsafe::sandstone, WallID::Unsafe::crimsandstone},
        {WallID::Unsafe::hardenedSand, WallID::Unsafe::hardenedCrimsand}};
    for (int wallId : WallVariants::dirt) {
        crimsonWalls[wallId] = rnd.select(WallVariants::crimson);
    }
    for (int wallId : WallVariants::stone) {
        crimsonWalls[wallId] = rnd.select(WallVariants::crimson);
    }
    for (int wallId : WallVariants::jungle) {
        crimsonWalls[wallId] = rnd.select(WallVariants::crimson);
    }
    for (int wallId :
         {WallID::Unsafe::snow,
          WallID::Unsafe::ice,
          WallID::Unsafe::mushroom,
          WallID::Unsafe::marble,
          WallID::Unsafe::granite}) {
        crimsonWalls[wallId] = rnd.select(WallVariants::crimson);
    }
    constexpr auto tunnelBorderExclusions = frozen::make_set<int>(
        {TileID::empty,
         TileID::ironOre,
         TileID::leadOre,
         TileID::silverOre,
         TileID::tungstenOre});
    constexpr auto tunnelSkipTiles = frozen::make_set<int>(
        {TileID::livingWood,
         TileID::leaf,
         TileID::livingMahogany,
         TileID::mahoganyLeaf});
    int scaleFactor =
        std::midpoint<int>(world.getWidth(), 3.5 * world.getHeight());
    // Dig surface smooth tunnel network, edged with crimstone.
    for (int x = surfaceX - scanDist; x < surfaceX + scanDist; ++x) {
        for (int y = 0.45 * world.getUndergroundLevel();
             y < world.getCavernLevel();
             ++y) {
            double threshold = std::min(
                {2 - 50.0 * std::abs(x - surfaceX) / scaleFactor,
                 0.01 * (world.getCavernLevel() - y),
                 0.16});
            if (std::abs(rnd.getBlurNoise(2 * x, 2 * y) + 0.1) < threshold) {
                Tile &tile = world.getTile(x, y);
                if (tunnelSkipTiles.contains(tile.blockID) ||
                    tile.wallID == WallID::Unsafe::livingWood) {
                    continue;
                }
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
                for (int y = std::max(sourceY - scanDist, 0);
                     y < sourceY + scanDist;
                     ++y) {
                    double threshold =
                        1 - std::sqrt(
                                18 * std::hypot(x - sourceX, y - sourceY) /
                                scaleFactor);
                    if (std::abs(rnd.getBlurNoise(x, y)) < threshold) {
                        Tile &tile = world.getTile(x, y);
                        threshold =
                            1 - std::pow(
                                    21 * std::hypot(x - sourceX, y - sourceY) /
                                        scaleFactor,
                                    0.028);
                        if (std::abs(rnd.getBlurNoise(x, y)) < threshold) {
                            // Crimson spreads from tendrils of flesh blocks.
                            // Fill the core of central tendrils with crimtane.
                            tile.blockID = std::abs(rnd.getBlurNoise(x, y)) <
                                                   threshold - 0.045
                                               ? y > world.getUndergroundLevel()
                                                     ? TileID::crimtane
                                                     : TileID::crimstone
                                               : TileID::flesh;
                            if (tile.wallID != WallID::empty) {
                                tile.wallID = WallID::Unsafe::crimsonBlister;
                            }
                            // Handle aether bubble.
                            tile.echoCoatBlock = false;
                        } else {
                            auto blockItr = crimsonBlocks.find(tile.blockID);
                            if (blockItr != crimsonBlocks.end()) {
                                tile.blockID = blockItr->second;
                            } else if (
                                tile.blockID == TileID::livingWood ||
                                tile.blockID == TileID::livingMahogany) {
                                tile.blockPaint = Paint::gray;
                            } else if (
                                tile.blockID == TileID::ash &&
                                y < world.getUnderworldLevel() +
                                        10 * rnd.getFineNoise(x, y) - 20) {
                                tile.blockID = TileID::crimstone;
                            }
                            auto wallItr = crimsonWalls.find(tile.wallID);
                            if (wallItr != crimsonWalls.end()) {
                                tile.wallID = wallItr->second;
                            } else if (
                                tile.wallID == WallID::Unsafe::livingWood) {
                                tile.wallPaint = Paint::gray;
                            }
                        }
                    }
                }
            });
    };
    // Surface crimson.
    applyCrimson(surfaceX, surfaceY);
    // Underground crimson.
    applyCrimson(
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
