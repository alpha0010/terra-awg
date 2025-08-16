#include "Corruption.h"

#include "Config.h"
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

void genCloudCorruption(Random &rnd, World &world)
{
    int maxY = 0.45 * world.getUndergroundLevel();
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [maxY, &rnd, &world](int x) {
            for (int y = 0; y < maxY; ++y) {
                Tile &tile = world.getTile(x, y);
                int corruptBlock = TileID::empty;
                switch (tile.blockID) {
                case TileID::grass:
                    tile.blockID = TileID::corruptGrass;
                    break;
                case TileID::jungleGrass:
                    tile.blockID = TileID::corruptJungleGrass;
                    break;
                case TileID::sand:
                    tile.blockID = TileID::ebonsand;
                    break;
                case TileID::cloud:
                case TileID::rainCloud:
                case TileID::snowCloud:
                    corruptBlock = TileID::lesion;
                    break;
                case TileID::snow:
                    corruptBlock = TileID::corruptIce;
                    break;
                case TileID::goldOre:
                case TileID::platinumOre:
                    corruptBlock = TileID::demonite;
                    break;
                }
                if (corruptBlock != TileID::empty &&
                    std::abs(rnd.getCoarseNoise(3 * x, 3 * y)) < 0.1) {
                    tile.blockID = corruptBlock;
                    if (tile.wallID != WallID::empty) {
                        tile.wallID = WallID::Unsafe::corruptTendril;
                    }
                }
            }
        });
}

std::pair<int, int> baseSelectEvilLocations(Random &rnd, World &world)
{
    // Avoid selecting too near spawn.
    int surfaceX = world.getWidth() * rnd.getDouble(0.12, 0.39);
    if (rnd.getBool()) {
        surfaceX = world.getWidth() - surfaceX;
    }
    int undergroundX;
    if (world.conf.hiveQueen) {
        undergroundX = world.getWidth() * rnd.getDouble(0.08, 0.42);
        if (rnd.getBool()) {
            undergroundX = world.getWidth() - undergroundX;
        }
    } else {
        undergroundX = world.getWidth() * rnd.getDouble(0.08, 0.92);
    }
    return {surfaceX, undergroundX};
}

std::pair<int, int> selectAvoidantEvilLocations(Random &rnd, World &world)
{
    int scanDist = 0.06 * world.getWidth();
    double maxForest = 0;
    int bestSurface = -1;
    int betUnderground = -1;
    int undergroundY =
        std::midpoint(world.getCavernLevel(), world.getUnderworldLevel());
    for (int tries = 0; tries < 10; ++tries) {
        auto [surfaceX, undergroundX] = baseSelectEvilLocations(rnd, world);
        double forestScore = 0;
        for (int i = -scanDist; i < scanDist; ++i) {
            for (int j = -scanDist; j < scanDist; ++j) {
                forestScore +=
                    world
                        .getBiome(surfaceX + i, world.getUndergroundLevel() + j)
                        .forest;
                forestScore +=
                    world.getBiome(undergroundX + i, undergroundY + j).forest;
            }
        }
        if (forestScore > maxForest) {
            maxForest = forestScore;
            bestSurface = surfaceX;
            betUnderground = undergroundX;
        }
    }
    return {bestSurface, betUnderground};
}

std::vector<std::pair<int, int>> selectEvilLocations(Random &rnd, World &world)
{
    if (world.conf.dontDigUp) {
        auto partitions =
            rnd.partitionRange(rnd.getInt(20, 24), world.getWidth());
        std::vector<std::pair<int, int>> locations;
        for (int partition : partitions) {
            locations.emplace_back(partition, -1);
        }
        locations[0].second = world.getWidth() * rnd.getDouble(0.02, 0.06);
        locations[1].second = world.getWidth() * rnd.getDouble(0.05, 0.09);
        locations[2].second = world.getWidth() * rnd.getDouble(0.08, 0.12);
        locations[locations.size() - 3].second =
            world.getWidth() * rnd.getDouble(0.88, 0.92);
        locations[locations.size() - 2].second =
            world.getWidth() * rnd.getDouble(0.91, 0.95);
        locations[locations.size() - 1].second =
            world.getWidth() * rnd.getDouble(0.94, 0.98);
        return locations;
    }
    return {
        world.conf.avoidantEvil ? selectAvoidantEvilLocations(rnd, world)
                                : baseSelectEvilLocations(rnd, world)};
}

void genCorruption(Random &rnd, World &world)
{
    std::cout << "Corrupting the world\n";
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
        genCorruptionAt(surfaceX, undergroundX, rnd, world);
    }
    if (world.conf.forTheWorthy || world.conf.dontDigUp) {
        genCloudCorruption(rnd, world);
    }
}

void genCorruptionAt(int surfaceX, int undergroundX, Random &rnd, World &world)
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
         {TileID::hive, TileID::ebonstone},
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
    int halfWidth = world.getWidth() / 2;
    int scaleFactor =
        world.conf.evilSize *
        std::midpoint<double>(world.getWidth(), 3.5 * world.getHeight()) *
        (world.conf.dontDigUp ? std::abs(surfaceX - halfWidth) < halfWidth / 2
                                    ? rnd.getDouble(0.45, 0.65)
                                    : rnd.getDouble(0.7, 1)
                              : 1);
    // Dig surface chasms, edged with ebonstone.
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
    }
    auto applyCorruption = [&](int sourceX, int sourceY) {
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
                            if (tile.blockID != TileID::empty ||
                                tile.wallID != WallID::empty ||
                                y > tendrilMinY) {
                                tile.blockID =
                                    std::abs(rnd.getCoarseNoise(x, y)) <
                                            threshold - 0.065
                                        ? y > world.getUndergroundLevel()
                                              ? TileID::demonite
                                              : TileID::ebonstone
                                        : TileID::lesion;
                                if (tile.wallID != WallID::empty) {
                                    tile.wallID =
                                        WallID::Unsafe::corruptTendril;
                                }
                                // Handle aether bubble.
                                tile.echoCoatBlock = false;
                            }
                        } else {
                            auto blockItr = corruptBlocks.find(tile.blockID);
                            if (blockItr != corruptBlocks.end()) {
                                if (y > world.getUndergroundLevel() ||
                                    blockItr->second != TileID::demonite) {
                                    tile.blockID = blockItr->second;
                                }
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
