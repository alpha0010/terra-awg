#include "GemGrove.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/DecoGems.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>
#include <set>

typedef std::array<std::pair<int, int>, 5> GemSwatch;

inline const std::array gemSwatches = std::to_array<GemSwatch>({
    {{{TileID::amethystGemspark, Paint::pink},
      {TileID::amethystGemspark, Paint::violet},
      {TileID::amethystGemspark, Paint::none},
      {TileID::amethystGemspark, Paint::purple},
      {TileID::amethystGemspark, Paint::deepPurple}}},
    {{{TileID::diamondGemspark, Paint::none},
      {TileID::topazGemspark, Paint::yellow},
      {TileID::topazGemspark, Paint::none},
      {TileID::topazGemspark, Paint::brown},
      {TileID::topazGemspark, Paint::orange}}},
    {{{TileID::sapphireGemspark, Paint::skyBlue},
      {TileID::sapphireGemspark, Paint::deepSkyBlue},
      {TileID::sapphireGemspark, Paint::none},
      {TileID::sapphireGemspark, Paint::blue},
      {TileID::sapphireGemspark, Paint::deepBlue}}},
    {{{TileID::emeraldGemspark, Paint::yellow},
      {TileID::emeraldGemspark, Paint::teal},
      {TileID::emeraldGemspark, Paint::lime},
      {TileID::emeraldGemspark, Paint::none},
      {TileID::emeraldGemspark, Paint::green}}},
    {{{TileID::rubyGemspark, Paint::deepViolet},
      {TileID::rubyGemspark, Paint::pink},
      {TileID::rubyGemspark, Paint::deepPink},
      {TileID::rubyGemspark, Paint::none},
      {TileID::rubyGemspark, Paint::deepRed}}},
    {{{TileID::amberGemspark, Paint::yellow},
      {TileID::amberGemspark, Paint::brown},
      {TileID::amberGemspark, Paint::orange},
      {TileID::amberGemspark, Paint::deepOrange},
      {TileID::amberGemspark, Paint::none}}},
    {{{TileID::diamondGemspark, Paint::white},
      {TileID::diamondGemspark, Paint::gray},
      {TileID::diamondGemspark, Paint::none},
      {TileID::diamondGemspark, Paint::black},
      {TileID::diamondGemspark, Paint::shadow}}},
});

TileBuffer getDecoGem(Random &rnd, World &world)
{
    TileBuffer gem =
        Data::getDecoGem(rnd.select(Data::gems), world.getFramedTiles());
    const GemSwatch &swatch = rnd.pool(gemSwatches);
    int offset = rnd.getInt(0, 6 - gem.getWidth() - gem.getHeight());
    for (int i = 0; i < gem.getWidth(); ++i) {
        for (int j = 0; j < gem.getHeight(); ++j) {
            Tile &tile = gem.getTile(i, j);
            if (tile.blockID != TileID::empty) {
                int idx = offset + i + j;
                tile.blockID = swatch[idx].first;
                tile.blockPaint = swatch[idx].second;
            }
        }
    }
    return gem;
}

void placeGroveDecoGems(Random &rnd, World &world)
{
    int groveSize = world.gemGroveSize;
    std::vector<Point> rawLocations;
    for (int i = -groveSize; i < groveSize; ++i) {
        for (int j = -groveSize; j < groveSize; ++j) {
            if (world.getTile(world.gemGrove + Point{i, j}).blockID ==
                    TileID::empty &&
                std::hypot(i, j) < groveSize) {
                rawLocations.push_back(world.gemGrove + Point{i, j});
            }
        }
    }
    std::vector<Point> locations;
    std::sample(
        rawLocations.begin(),
        rawLocations.end(),
        std::back_inserter(locations),
        rawLocations.size() / 70,
        rnd.getPRNG());
    for (auto [x, y] : locations) {
        if (world.regionPasses(
                x - 2,
                y - 2,
                7,
                7,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            world.regionPasses(x, y, 3, 3, [](Tile &tile) {
                return tile.wallID != WallID::empty &&
                       tile.liquid == Liquid::none && !tile.wireRed;
            })) {
            world.placeBuffer(x, y, getDecoGem(rnd, world));
        }
    }
}

void placeGemChest(Random &rnd, World &world)
{
    int scanDist = 1.5 * world.gemGroveSize;
    for (int iSwap = 0; iSwap < scanDist; ++iSwap) {
        int i = iSwap / 2;
        if (iSwap % 2 == 0) {
            i = -i;
        }
        for (int jSwap = 0; jSwap < scanDist; ++jSwap) {
            int j = jSwap / 2;
            if (jSwap % 2 == 0) {
                j = -j;
            }
            if (world.regionPasses(
                    world.gemGrove.x + i,
                    world.gemGrove.y + j - 1,
                    2,
                    3,
                    [](Tile &tile) {
                        return tile.blockID == TileID::empty &&
                               tile.liquid == Liquid::none;
                    }) &&
                world.regionPasses(
                    world.gemGrove.x + i,
                    world.gemGrove.y + j + 2,
                    2,
                    1,
                    [](Tile &tile) { return tile.blockID == TileID::stone; })) {
                Chest &chest = world.placeChest(
                    world.gemGrove.x + i,
                    world.gemGrove.y + j,
                    Variant::crystal);
                fillCrystalChest(chest, rnd, world);
                return;
            }
        }
    }
}

Point selectGemGroveLocation(double &groveSize, Random &rnd, World &world)
{
    constexpr auto allowedTiles = frozen::make_set<int>(
        {TileID::empty,
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
         TileID::platinumOre,
         TileID::cobaltOre,
         TileID::palladiumOre,
         TileID::mythrilOre,
         TileID::orichalcumOre,
         TileID::adamantiteOre,
         TileID::titaniumOre});
    std::set<int> allowedWalls{
        WallVariants::dirt.begin(),
        WallVariants::dirt.end()};
    allowedWalls.insert(WallID::empty);
    if (world.conf.hiveQueen) {
        allowedWalls.insert(WallID::Unsafe::hive);
    }
    constexpr auto partialTiles = frozen::make_set<int>(
        {TileID::snow,
         TileID::ice,
         TileID::thinIce,
         TileID::slush,
         TileID::hardenedSand,
         TileID::sandstone,
         TileID::desertFossil,
         TileID::jungleGrass,
         TileID::silt,
         TileID::ash,
         TileID::ashGrass,
         TileID::marble,
         TileID::granite});
    std::set<int> partialWalls{
        WallID::Unsafe::snow,
        WallID::Unsafe::ice,
        WallID::Unsafe::hardenedSand,
        WallID::Unsafe::sandstone,
        WallID::Unsafe::marble,
        WallID::Unsafe::granite};
    partialWalls.insert(
        WallVariants::jungle.begin(),
        WallVariants::jungle.end());
    partialWalls.insert(WallVariants::stone.begin(), WallVariants::stone.end());
    partialWalls.insert(
        WallVariants::underworld.begin(),
        WallVariants::underworld.end());
    int safeMinY =
        world.conf.dontDigUp
            ? (4 * world.getUndergroundLevel() + world.getCavernLevel()) / 5
            : (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3;
    int safeMaxY =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    double shrink = groveSize / 10000.0;
    for (int numTries = 0; numTries < 5000; ++numTries, groveSize -= shrink) {
        auto [x, y] = findStoneCave(
            std::min<int>(
                (world.conf.dontDigUp ? world.getUndergroundLevel()
                                      : world.getCavernLevel()) +
                    groveSize,
                safeMinY),
            std::max<int>(world.getUnderworldLevel() - groveSize, safeMaxY),
            rnd,
            world);
        if (x < 75 + groveSize || x > world.getWidth() - 75 - groveSize) {
            continue;
        }
        int threshold = groveSize * groveSize *
                        (world.conf.biomes == BiomeLayout::patches ? 0.8 : 0.1);
        if (world.regionPasses(
                x - groveSize,
                y - groveSize,
                2 * groveSize,
                2 * groveSize,
                [&threshold,
                 &allowedTiles,
                 &allowedWalls,
                 &partialTiles,
                 &partialWalls](Tile &tile) {
                    if (partialTiles.contains(tile.blockID) ||
                        (tile.blockID == TileID::empty &&
                         partialWalls.contains(tile.wallID))) {
                        --threshold;
                        return threshold > 0;
                    }
                    return (allowedTiles.contains(tile.blockID) ||
                            (tile.flag == Flag::border &&
                             tile.blockID == TileID::hive)) &&
                           allowedWalls.contains(tile.wallID);
                })) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void genGemGrove(Random &rnd, World &world)
{
    std::cout << "Imbuing gems\n";
    rnd.restoreShuffleState();
    int noiseShuffleX = rnd.getInt(0, world.getWidth());
    int noiseShuffleY = rnd.getInt(0, world.getHeight());
    double groveSize =
        world.getWidth() * world.getHeight() / 329000 + rnd.getInt(60, 75);
    auto [x, y] = selectGemGroveLocation(groveSize, rnd, world);
    if (x == -1) {
        return;
    }
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
    world.gemGrove = {x, y};
    world.gemGroveSize = groveSize;
    world.queuedDeco.emplace_back(placeGroveDecoGems);
    world.queuedTreasures.emplace_back(placeGemChest);
}
