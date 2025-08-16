#include "Crimson.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/Corruption.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>
#include <map>

void genCloudCrimson(Random &rnd, World &world)
{
    int maxY = 0.45 * world.getUndergroundLevel();
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [maxY, &rnd, &world](int x) {
            for (int y = 0; y < maxY; ++y) {
                Tile &tile = world.getTile(x, y);
                int crimsonBlock = TileID::empty;
                switch (tile.blockID) {
                case TileID::grass:
                    tile.blockID = TileID::crimsonGrass;
                    break;
                case TileID::jungleGrass:
                    tile.blockID = TileID::crimsonJungleGrass;
                    break;
                case TileID::sand:
                    tile.blockID = TileID::crimsand;
                    break;
                case TileID::cloud:
                case TileID::rainCloud:
                case TileID::snowCloud:
                    crimsonBlock = TileID::flesh;
                    break;
                case TileID::snow:
                    crimsonBlock = TileID::crimsonIce;
                    break;
                case TileID::goldOre:
                case TileID::platinumOre:
                    crimsonBlock = TileID::crimtane;
                    break;
                }
                if (crimsonBlock != TileID::empty &&
                    std::abs(rnd.getBlurNoise(3 * x, 3 * y)) < 0.1) {
                    tile.blockID = crimsonBlock;
                    if (tile.wallID != WallID::empty) {
                        tile.wallID = WallID::Unsafe::crimsonBlister;
                    }
                }
            }
        });
}

void genCrimson(Random &rnd, World &world)
{
    std::cout << "Infecting the world\n";
    rnd.shuffleNoise();
    int minX = 0;
    int maxX = world.getWidth();
    if (world.conf.dontDigUp && world.conf.doubleTrouble) {
        if (rnd.getBool()) {
            minX = world.getWidth() / 2;
        } else {
            maxX = world.getWidth() / 2;
        }
    }
    for (auto [surfaceX, undergroundX] : selectEvilLocations(rnd, world)) {
        if (surfaceX < minX || surfaceX > maxX) {
            continue;
        }
        // Register location for use in other generators.
        world.surfaceEvilCenter = surfaceX;
        genCrimsonAt(surfaceX, undergroundX, rnd, world);
    }
    if (world.conf.forTheWorthy || world.conf.dontDigUp) {
        genCloudCrimson(rnd, world);
    }
}

void genCrimsonAt(int surfaceX, int undergroundX, Random &rnd, World &world)
{
    int surfaceY =
        world.conf.dontDigUp
            ? rnd.getInt(
                  0.7 * world.getUndergroundLevel(),
                  0.9 * world.getUndergroundLevel())
            : rnd.getInt(
                  0.95 * world.getUndergroundLevel(),
                  (2 * world.getUndergroundLevel() + world.getCavernLevel()) /
                      3);
    int scanDist = world.conf.evilSize * 0.08 * world.getWidth();
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
         {TileID::hive, TileID::crimstone},
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
    int halfWidth = world.getWidth() / 2;
    int scaleFactor =
        world.conf.evilSize *
        std::midpoint<double>(world.getWidth(), 3.5 * world.getHeight()) *
        (world.conf.dontDigUp ? std::abs(surfaceX - halfWidth) < halfWidth / 2
                                    ? rnd.getDouble(0.45, 0.65)
                                    : rnd.getDouble(0.7, 1)
                              : 1);
    // Dig surface smooth tunnel network, edged with crimstone.
    if (!world.conf.dontDigUp || rnd.getInt(0, 4) == 0) {
        int maxY = world.conf.dontDigUp ? std::midpoint(
                                              world.getUndergroundLevel(),
                                              world.getCavernLevel())
                                        : world.getCavernLevel();
        for (int x = std::max(surfaceX - scanDist, 0);
             x < std::min(surfaceX + scanDist, world.getWidth());
             ++x) {
            for (int y = 0.45 * world.getUndergroundLevel(); y < maxY; ++y) {
                double threshold = std::min(
                    {2 - 50.0 * std::abs(x - surfaceX) / scaleFactor,
                     0.01 * (maxY - y),
                     0.16});
                if (std::abs(rnd.getBlurNoise(2 * x, 2 * y) + 0.1) <
                    threshold) {
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
    }
    auto applyCrimson = [&](int sourceX, int sourceY) {
        if (sourceX == -1) {
            return;
        }
        parallelFor(
            std::views::iota(
                std::max(sourceX - scanDist, 0),
                std::min(sourceX + scanDist, world.getWidth())),
            [&, scanDist, sourceX, sourceY](int x) {
                int tendrilMinY = world.conf.trimEvilTendrils
                                      ? world.getSurfaceLevel(x) - 20 +
                                            10 * rnd.getFineNoise(x, 0)
                                      : 0;
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
                            if (tile.blockID != TileID::empty ||
                                tile.wallID != WallID::empty ||
                                y > tendrilMinY) {
                                tile.blockID =
                                    std::abs(rnd.getBlurNoise(x, y)) <
                                            threshold - 0.045
                                        ? y > world.getUndergroundLevel()
                                              ? TileID::crimtane
                                              : TileID::crimstone
                                        : TileID::flesh;
                                if (tile.wallID != WallID::empty) {
                                    tile.wallID =
                                        WallID::Unsafe::crimsonBlister;
                                }
                                // Handle aether bubble.
                                tile.echoCoatBlock = false;
                            }
                        } else {
                            auto blockItr = crimsonBlocks.find(tile.blockID);
                            if (blockItr != crimsonBlocks.end()) {
                                if (y > world.getUndergroundLevel() ||
                                    blockItr->second != TileID::crimtane) {
                                    tile.blockID = blockItr->second;
                                }
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
            world.conf.dontDigUp
                ? (world.getUndergroundLevel() + 2 * world.getCavernLevel()) / 3
                : (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
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
