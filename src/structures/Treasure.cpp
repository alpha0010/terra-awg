#include "structures/Treasure.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/ItemID.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Traps.h"
#include "structures/UndergroundCabin.h"
#include "structures/data/Altars.h"
#include "structures/data/JungleShrines.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

enum class Gem {
    topaz = 18,
    sapphire = 36,
    emerald = 54,
    ruby = 72,
    diamond = 90,
    amber = 108
};

template <typename T, typename U>
bool listContains(const T &list, const U &value)
{
    return std::find(std::begin(list), std::end(list), value) != std::end(list);
}

inline constexpr auto placementAvoidTiles = frozen::make_set<int>({
    TileID::grate,
    TileID::leaf,
    TileID::livingWood,
    TileID::mahoganyLeaf,
    TileID::thinIce,
    TileID::spike,
    TileID::woodenSpike,
});

bool isPlacementCandidate(int x, int y, World &world)
{
    auto isFloorTile = [x, y, &world](const Tile &tile) {
        return isSolidBlock(tile.blockID) && tile.slope == Slope::none &&
               !tile.actuated && !placementAvoidTiles.contains(tile.blockID) &&
               ((tile.blockID != TileID::aetherium &&
                 tile.blockID != TileID::pearlstone) ||
                y > world.getSurfaceLevel(x));
    };
    return isFloorTile(world.getTile(x, y)) &&
           isFloorTile(world.getTile(x + 1, y)) &&
           world.regionPasses(x, y - 3, 2, 3, [](Tile &tile) {
               return tile.blockID == TileID::empty &&
                      tile.liquid != Liquid::lava &&
                      tile.liquid != Liquid::shimmer;
           });
}

int testOrbHeartCandidate(int x, int y, World &world)
{
    if (y < world.getUndergroundLevel() || y > world.getUnderworldLevel()) {
        return TileID::empty;
    }
    int tendrilID = TileID::empty;
    for (auto [i, j] : {std::pair{1, 1}, {1, 6}, {6, 1}, {6, 6}}) {
        int cornerID = world.getTile(x + i, y + j).blockID;
        if (cornerID == TileID::lesion || cornerID == TileID::flesh) {
            tendrilID = cornerID;
            break;
        }
    }
    if (tendrilID == TileID::empty) {
        return tendrilID;
    }
    constexpr auto allowedTiles = frozen::make_set<int>(
        {TileID::clay,
         TileID::mud,
         TileID::ebonstone,
         TileID::ebonsand,
         TileID::corruptIce,
         TileID::ebonsandstone,
         TileID::hardenedEbonsand,
         TileID::lesion,
         TileID::crimstone,
         TileID::crimsand,
         TileID::crimsonIce,
         TileID::crimsandstone,
         TileID::hardenedCrimsand,
         TileID::flesh});
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (((i == 0 || i == 7) && (j < 2 || j > 5)) ||
                ((j == 0 || j == 7) && (i < 2 || i > 5))) {
                continue;
            }
            if (!allowedTiles.contains(world.getTile(x + i, y + j).blockID)) {
                return TileID::empty;
            }
        }
    }
    return tendrilID;
}

bool attachGemTo(Gem gem, int x, int y, Random &rnd, World &world)
{
    auto [deltaX, deltaY, frameY] = rnd.select(
        {std::tuple{-1, 0, 162}, {1, 0, 108}, {0, -1, 0}, {0, 1, 54}});
    Tile &tile = world.getTile(x + deltaX, y + deltaY);
    if (tile.blockID != TileID::empty) {
        return false;
    }
    tile.blockID = TileID::gem;
    tile.frameX = static_cast<int>(gem);
    tile.frameY = frameY + 18 * (fnv1a32pt(x, y) % 3);
    return true;
}

void placeGems(Random &rnd, World &world)
{
    int numGems = world.conf.gems * world.getWidth() * world.getHeight() /
                  rnd.getInt(65800, 76800);
    int scanDist = world.conf.desertSize * 0.08 * world.getWidth();
    int minX = world.conf.patches ? 350 : world.desertCenter - scanDist;
    int maxX = world.conf.patches ? world.getWidth() - 350
                                  : world.desertCenter + scanDist;
    constexpr auto validAnchors = frozen::make_set<int>(
        {TileID::sand,
         TileID::hardenedSand,
         TileID::sandstone,
         TileID::ebonsand,
         TileID::hardenedEbonsand,
         TileID::ebonsandstone,
         TileID::crimsand,
         TileID::hardenedCrimsand,
         TileID::crimsandstone,
         TileID::pearlsand,
         TileID::hardenedPearlsand,
         TileID::pearlsandstone,
         TileID::coralstone});
    for (int tries = 800 * numGems; numGems > 0 && tries > 0; --tries) {
        int x = rnd.getInt(minX, maxX);
        int y =
            rnd.getInt(world.getUndergroundLevel(), world.getUnderworldLevel());
        if ((world.conf.patches && world.getBiome(x, y).desert < 0.99) ||
            !validAnchors.contains(world.getTile(x, y).blockID)) {
            continue;
        }
        if (attachGemTo(Gem::amber, x, y, rnd, world)) {
            --numGems;
        }
    }
    std::vector<Point> locations;
    int maxY = (5 * world.getCavernLevel() + world.getUnderworldLevel()) / 6;
    for (int x = 50; x < world.getWidth() - 50; ++x) {
        if (x == 200) {
            x = world.getWidth() - 200;
        }
        for (int y = world.getUndergroundLevel(); y < maxY; ++y) {
            if (validAnchors.contains(world.getTile(x, y).blockID) &&
                !world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
                    return tile.blockID != TileID::empty ||
                           tile.liquid == Liquid::none;
                })) {
                locations.emplace_back(x, y);
            }
        }
    }
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    locations.resize(locations.size() / 6);
    for (auto [x, y] : locations) {
        attachGemTo(
            y < world.getCavernLevel()
                ? rnd.select({Gem::topaz, Gem::sapphire, Gem::emerald})
                : rnd.select(
                      {Gem::sapphire, Gem::emerald, Gem::ruby, Gem::diamond}),
            x,
            y,
            rnd,
            world);
    }
}

void placeLifeCrystals(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int lifeCrystalCount =
        world.conf.lifeCrystals * world.getWidth() * world.getHeight() / 50000;
    int maxDungeonPlacements = 3;
    std::vector<Point> usedLocations;
    while (lifeCrystalCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (y > world.getUndergroundLevel() && y < world.getUnderworldLevel() &&
            isPlacementCandidate(x, y, world)) {
            int probeWall = world.getTile(x, y - 2).wallID;
            if (listContains(WallVariants::dungeon, probeWall)) {
                if (maxDungeonPlacements > 0) {
                    world.placeFramedTile(x, y - 2, TileID::lifeCrystalBoulder);
                    --maxDungeonPlacements;
                } else {
                    continue;
                }
            } else if (
                probeWall == WallID::Unsafe::lihzahrdBrick ||
                isLocationUsed(x, y, 50, usedLocations, 2)) {
                continue;
            } else {
                usedLocations.emplace_back(x, y);
                double threshold = world.conf.traps > 14
                                       ? 0.0076 * world.conf.traps + 0.036
                                       : 0;
                world.placeFramedTile(
                    x,
                    y - 2,
                    rnd.getDouble(0, 1) < threshold ? TileID::lifeCrystalBoulder
                                                    : TileID::lifeCrystal);
            }
            --lifeCrystalCount;
        }
    }
}

void placeFallenLogs(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int logCount = world.getWidth() / rnd.getInt(1066, 1600);
    while (logCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (y > 0.45 * world.getUndergroundLevel() &&
            y < world.getUndergroundLevel() &&
            world.getTile(x, y).blockID == TileID::grass &&
            isPlacementCandidate(x, y, world) &&
            isPlacementCandidate(x - 1, y, world)) {
            world.placeFramedTile(x - 1, y - 2, TileID::fallenLog);
            --logCount;
        }
    }
}

void placeAltars(int maxBin, LocationBins &locations, Random &rnd, World &world)
{
    int altarCount = std::max(8, world.getWidth() / 200);
    if (world.conf.doubleTrouble) {
        altarCount *= 2;
    }
    constexpr auto corruptTiles = frozen::make_set<int>(
        {TileID::ebonstone,
         TileID::corruptGrass,
         TileID::demonite,
         TileID::ebonsand,
         TileID::corruptJungleGrass,
         TileID::corruptIce,
         TileID::ebonsandstone,
         TileID::hardenedEbonsand,
         TileID::lesion});
    constexpr auto crimsonTiles = frozen::make_set<int>(
        {TileID::crimstone,
         TileID::crimsonGrass,
         TileID::crimtane,
         TileID::crimsand,
         TileID::crimsonJungleGrass,
         TileID::crimsonIce,
         TileID::crimsandstone,
         TileID::hardenedCrimsand,
         TileID::flesh});
    while (altarCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (y < 0.8 * world.getUndergroundLevel()) {
            continue;
        }
        Variant type = corruptTiles.contains(world.getTile(x, y).blockID)
                           ? Variant::corruption
                       : crimsonTiles.contains(world.getTile(x, y).blockID)
                           ? Variant::crimson
                           : Variant::none;
        if (type != Variant::none && isPlacementCandidate(x, y, world) &&
            isPlacementCandidate(x - 1, y, world)) {
            world.placeFramedTile(x - 1, y - 2, TileID::altar, type);
            --altarCount;
        }
    }
}

Point selectPurityAltarLocation(
    TileBuffer &altar,
    int lavaLevel,
    Random &rnd,
    World &world)
{
    constexpr auto avoidTiles = frozen::make_set<int>(
        {TileID::obsidianBrick,
         TileID::hellstoneBrick,
         TileID::ebonstoneBrick,
         TileID::lesion,
         TileID::crimstoneBrick,
         TileID::flesh,
         TileID::lamp,
         TileID::blueBrick,
         TileID::greenBrick,
         TileID::pinkBrick});
    int scanHeight = std::max<int>(1.5 * altar.getHeight(), 30);
    int scanY = lavaLevel - scanHeight;
    while (true) {
        int x = rnd.getInt(150, world.getWidth() - 150 - altar.getWidth());
        if (!world.regionPasses(
                x,
                scanY,
                altar.getWidth(),
                scanHeight,
                [&avoidTiles](Tile &tile) {
                    return !avoidTiles.contains(tile.blockID);
                })) {
            continue;
        }
        int y = lavaLevel;
        for (int i : {1, 2, altar.getWidth() - 3, altar.getWidth() - 2}) {
            y = std::min(
                y,
                scanWhileEmpty({x + i, scanY}, {0, 1}, world).second);
        }
        y += 2 - altar.getHeight();
        return {x, y};
    }
}

Point scanForAltarOffset(TileBuffer &altar)
{
    for (int i = 0; i < altar.getWidth(); ++i) {
        for (int j = 0; j < altar.getHeight(); ++j) {
            if (altar.getTile(i, j).blockID == TileID::altar) {
                return {i, j};
            }
        }
    }
    return {-1, -1};
}

void placePurityAltars(Random &rnd, World &world)
{
    std::vector<int> altars;
    if (world.conf.doubleTrouble) {
        altars.insert(
            altars.end(),
            Data::corruptAltars.begin(),
            Data::corruptAltars.end());
        std::shuffle(altars.begin(), altars.end(), rnd.getPRNG());
        altars.resize(4);
        altars.insert(
            altars.end(),
            Data::crimsonAltars.begin(),
            Data::crimsonAltars.end());
        std::shuffle(altars.begin() + 4, altars.end(), rnd.getPRNG());
        altars.resize(8);
    } else {
        if (world.isCrimson) {
            altars.insert(
                altars.end(),
                Data::crimsonAltars.begin(),
                Data::crimsonAltars.end());
        } else {
            altars.insert(
                altars.end(),
                Data::corruptAltars.begin(),
                Data::corruptAltars.end());
        }
        std::shuffle(altars.begin(), altars.end(), rnd.getPRNG());
        altars.resize(5);
    }
    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    int lavaLevel = world.getUnderworldLevel() + 0.46 * underworldHeight;
    for (size_t iter = 0; iter < altars.size(); ++iter) {
        TileBuffer altar = Data::getAltar(altars[iter], world.getFramedTiles());
        auto [x, y] = selectPurityAltarLocation(altar, lavaLevel, rnd, world);
        world.placeBuffer(x, y, altar);
        for (int i = 1; i < altar.getWidth() - 1; ++i) {
            int fillTile =
                world.getTile(x + i, y + altar.getHeight() - 2).blockID;
            for (int j = 1; j < altar.getHeight() - 1; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if ((tile.blockID == TileID::ebonstoneBrick ||
                     tile.blockID == TileID::crimstoneBrick) &&
                    fnv1a32pt(x + i, y + j) % 11 == 0) {
                    tile.blockID = TileID::hellstoneBrick;
                }
            }
            if (fillTile != TileID::ebonstoneBrick &&
                fillTile != TileID::crimstoneBrick) {
                continue;
            }
            int maxJ = altar.getHeight() + 30;
            for (int j = altar.getHeight() - 1; j < maxJ; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::empty) {
                    tile.blockID = fnv1a32pt(x + i, y + j) % 7 == 0
                                       ? TileID::hellstoneBrick
                                       : fillTile;
                    if (y + j == lavaLevel + 1) {
                        maxJ = std::min<int>(
                            maxJ,
                            j + 6 + 5 * rnd.getFineNoise(x + i, lavaLevel));
                    }
                } else {
                    break;
                }
            }
        }
        if (world.conf.hardmode && iter % 4 != 0) {
            auto delta = scanForAltarOffset(altar);
            x += delta.first;
            y += delta.second;
            int paint = world.getTile(x, y).frameX == 0 ? Paint::purple
                                                        : Paint::deepRed;
            world.placeFramedTile(x, y, TileID::largePileGroup2, Variant::ash);
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 2; ++j) {
                    world.getTile(x + i, y + j).blockPaint = paint;
                }
            }
        }
    }
}

void placeOrbHearts(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int orbHeartCount = world.getWidth() * world.getHeight() / 240000;
    if (world.conf.doubleTrouble) {
        orbHeartCount *= 2;
    }
    while (orbHeartCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        int tendrilID = testOrbHeartCandidate(x, y, world);
        if (tendrilID == TileID::empty) {
            continue;
        }
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (((i == 0 || i == 7) && (j < 2 || j > 5)) ||
                    ((j == 0 || j == 7) && (i < 2 || i > 5))) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID != tendrilID) {
                    tile.blockID = tendrilID == TileID::lesion
                                       ? TileID::ebonstone
                                       : TileID::crimstone;
                }
            }
        }
        world.placeFramedTile(
            x + 3,
            y + 3,
            TileID::orbHeart,
            tendrilID == TileID::lesion ? Variant::corruption
                                        : Variant::crimson);
        --orbHeartCount;
    }
}

void placeManaCrystals(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int manaCrystalCount =
        world.conf.manaCrystals * world.getWidth() * world.getHeight() / 310000;
    while (manaCrystalCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        int probeWall = world.getTile(x, y - 2).wallID;
        if (y > world.getUndergroundLevel() && y < world.getUnderworldLevel() &&
            isPlacementCandidate(x, y, world) &&
            !listContains(WallVariants::dungeon, probeWall) &&
            probeWall != WallID::Unsafe::lihzahrdBrick) {
            world.placeFramedTile(x, y - 2, TileID::manaCrystal);
            --manaCrystalCount;
        }
    }
}

Point selectShrineLocation(
    TileBuffer &shrine,
    std::vector<Point> &usedLocations,
    Random &rnd,
    World &world)
{
    int minOpenHeight = -1;
    for (int j = 0; j < shrine.getHeight(); ++j) {
        if (shrine.getTile(0, j).blockPaint == Paint::red ||
            shrine.getTile(shrine.getWidth() - 1, j).blockPaint == Paint::red) {
            minOpenHeight = j;
            break;
        }
    }
    if (minOpenHeight == -1) {
        minOpenHeight = shrine.getHeight();
    }
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::empty,         TileID::dirt,          TileID::mud,
         TileID::jungleGrass,   TileID::stone,         TileID::clay,
         TileID::silt,          TileID::copperOre,     TileID::tinOre,
         TileID::ironOre,       TileID::leadOre,       TileID::silverOre,
         TileID::tungstenOre,   TileID::goldOre,       TileID::platinumOre,
         TileID::cobaltOre,     TileID::palladiumOre,  TileID::mythrilOre,
         TileID::orichalcumOre, TileID::adamantiteOre, TileID::titaniumOre,
         TileID::chlorophyteOre});
    int minX = world.conf.patches
                   ? 350
                   : world.jungleCenter -
                         world.conf.jungleSize * 0.09 * world.getWidth();
    int maxX = world.conf.patches
                   ? world.getWidth() - 350
                   : world.jungleCenter +
                         world.conf.jungleSize * 0.09 * world.getWidth();
    for (int numTries = 0; numTries < 10000; ++numTries) {
        int x = rnd.getInt(minX, maxX);
        int y =
            rnd.getInt(world.getUndergroundLevel(), world.getUnderworldLevel());
        if ((world.conf.patches &&
             !isInBiome(x, y, 10, Biome::jungle, world)) ||
            !world.regionPasses(
                x,
                y,
                shrine.getWidth(),
                std::max(minOpenHeight, 3),
                [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           (tile.wallID == WallID::empty ||
                            listContains(WallVariants::jungle, tile.wallID) ||
                            listContains(WallVariants::hallow, tile.wallID)) &&
                           (tile.liquid == Liquid::none ||
                            tile.liquid == Liquid::water);
                })) {
            continue;
        }
        if (minOpenHeight == shrine.getHeight()) {
            // Floating shrine.
            if (!world.regionPasses(
                    x,
                    y,
                    shrine.getWidth(),
                    shrine.getHeight(),
                    [](Tile &tile) {
                        return !tile.guarded && tile.wallID != WallID::empty;
                    }) ||
                isLocationUsed(x, y, 40, usedLocations)) {
                continue;
            }
            return {x, y};
        }
        std::vector<int> groundLevel;
        for (int i = 0; i < shrine.getWidth(); ++i) {
            for (int j = minOpenHeight; j < 150; ++j) {
                if (world.getTile(x + i, y + j).blockID != TileID::empty) {
                    groundLevel.push_back(y + j);
                    break;
                }
            }
        }
        if (static_cast<int>(groundLevel.size()) != shrine.getWidth()) {
            // Ground out of scan range.
            continue;
        }
        for (; y <= groundLevel[0]; ++y) {
            bool isValid = true;
            for (int i = 0; i < shrine.getWidth(); ++i) {
                int j = groundLevel[i] - y;
                if (j < 0 || j >= shrine.getHeight() ||
                    ((i == 0 || i == shrine.getWidth() - 1) &&
                     shrine.getTile(i, j).blockPaint != Paint::red)) {
                    isValid = false;
                    break;
                }
            }
            if (!isValid) {
                continue;
            }
            if ( // Ground too thin.
                !world.regionPasses(
                    x,
                    y + shrine.getHeight() - 1,
                    shrine.getWidth(),
                    1,
                    [](Tile &tile) { return tile.blockID != TileID::empty; }) ||
                // Near other shrines.
                isLocationUsed(x, y, 40, usedLocations) ||
                // Restricted tiles.
                !world.regionPasses(
                    x,
                    y,
                    shrine.getWidth(),
                    shrine.getHeight(),
                    [&clearableTiles](Tile &tile) {
                        return !tile.guarded &&
                               clearableTiles.contains(tile.blockID) &&
                               (tile.liquid == Liquid::none ||
                                tile.liquid == Liquid::water);
                    })) {
                break;
            }
            return {x, y};
        }
    }
    return {-1, -1};
}

void placeJungleShrines(Random &rnd, World &world)
{
    int shrineCount = world.conf.chests * world.getWidth() * world.getHeight() /
                      rnd.getInt(590700, 677600);
    std::vector<Point> usedLocations;
    std::vector<int> shrines(Data::shrines.begin(), Data::shrines.end());
    std::shuffle(shrines.begin(), shrines.end(), rnd.getPRNG());
    for (int tries = 2 * shrineCount; shrineCount > 0 && tries > 0; --tries) {
        TileBuffer shrine =
            Data::getShrine(rnd.pool(shrines), world.getFramedTiles());
        auto [x, y] = selectShrineLocation(shrine, usedLocations, rnd, world);
        if (x == -1) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        bool isHallow = !world.regionPasses(
            x,
            y,
            shrine.getWidth(),
            shrine.getHeight(),
            [](Tile &tile) { return tile.blockPaint != Paint::cyan; });
        std::vector<Point> chests = world.placeBuffer(x, y, shrine);
        for (int i = 0; i < shrine.getWidth(); ++i) {
            for (int j = 0; j < shrine.getHeight(); ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::mud &&
                    world.isExposed(x + i, y + j)) {
                    tile.blockID = TileID::jungleGrass;
                    tile.blockPaint = isHallow ? Paint::cyan : Paint::none;
                }
            }
        }
        for (auto [chestX, chestY] : chests) {
            Chest &chest = world.placeChest(chestX, chestY, Variant::ivy);
            if (y < world.getCavernLevel()) {
                fillUndergroundIvyChest(chest, rnd, world);
            } else {
                fillCavernIvyChest(chest, rnd, world);
            }
        }
        --shrineCount;
    }
}

bool fuzzyIsSurfaceChest(int x, int y, World &world)
{
    return y < world.getUndergroundLevel() ||
           (y < (2 * world.getUndergroundLevel() + world.getCavernLevel()) /
                    3 &&
            fnv1a32pt(x, y) % 2 == 0);
}

Variant getChestType(int x, int y, World &world)
{
    Tile &probeTile = world.getTile(x, y - 2);
    if (y < 0.45 * world.getUndergroundLevel()) {
        return Variant::skyware;
    } else if (y > world.getUnderworldLevel()) {
        return Variant::shadow;
    } else if (probeTile.wallID == WallID::Unsafe::hive) {
        return Variant::honey;
    } else if (probeTile.liquid == Liquid::water) {
        return (x < 350 || x > world.getWidth() - 350) &&
                       (y < world.getUndergroundLevel() ||
                        !world.regionPasses(
                            x - 2,
                            y,
                            6,
                            6,
                            [](Tile &tile) {
                                return tile.blockID != TileID::coralstone;
                            }))
                   ? Variant::reef
                   : Variant::water;
    } else if (listContains(WallVariants::dungeon, probeTile.wallID)) {
        return Variant::goldLocked;
    } else if (probeTile.wallID == WallID::Unsafe::lihzahrdBrick) {
        return Variant::lihzahrd;
    }
    constexpr auto blockTypes = frozen::make_map<int, Variant>(
        {{TileID::ash, Variant::ashWood},
         {TileID::ashGrass, Variant::ashWood},
         {TileID::obsidian, Variant::ashWood},
         {TileID::crimstone, Variant::flesh},
         {TileID::crimsonGrass, Variant::flesh},
         {TileID::crimsand, Variant::flesh},
         {TileID::crimsonJungleGrass, Variant::flesh},
         {TileID::crimsonIce, Variant::flesh},
         {TileID::crimsandstone, Variant::flesh},
         {TileID::hardenedCrimsand, Variant::flesh},
         {TileID::flesh, Variant::flesh},
         {TileID::snow, Variant::frozen},
         {TileID::ice, Variant::frozen},
         {TileID::thinIce, Variant::frozen},
         {TileID::slush, Variant::frozen},
         {TileID::ebonstone, Variant::lesion},
         {TileID::corruptGrass, Variant::lesion},
         {TileID::ebonsand, Variant::lesion},
         {TileID::corruptJungleGrass, Variant::lesion},
         {TileID::corruptIce, Variant::lesion},
         {TileID::ebonsandstone, Variant::lesion},
         {TileID::hardenedEbonsand, Variant::lesion},
         {TileID::lesion, Variant::lesion},
         {TileID::granite, Variant::granite},
         {TileID::smoothGranite, Variant::granite},
         {TileID::marble, Variant::marble},
         {TileID::smoothMarble, Variant::marble},
         {TileID::heliumMossStone, Variant::meteorite},
         {TileID::aetherium, Variant::meteorite},
         {TileID::mushroomGrass, Variant::mushroom},
         {TileID::hallowedGrass, Variant::pearlwood},
         {TileID::pearlstone, Variant::pearlwood},
         {TileID::hallowedIce, Variant::pearlwood},
         {TileID::pearlsand, Variant::pearlwood},
         {TileID::hardenedPearlsand, Variant::pearlwood},
         {TileID::pearlsandstone, Variant::pearlwood},
         {TileID::crystalBlock, Variant::pearlwood},
         {TileID::jungleGrass, Variant::richMahogany},
         {TileID::livingMahogany, Variant::richMahogany},
         {TileID::mahoganyLeaf, Variant::richMahogany},
         {TileID::sand, Variant::sandstone},
         {TileID::sandstone, Variant::sandstone},
         {TileID::sandstoneBrick, Variant::sandstone},
         {TileID::hardenedSand, Variant::sandstone},
         {TileID::desertFossil, Variant::sandstone}});
    constexpr auto wallTypes = frozen::make_map<int, Variant>(
        {{WallID::Unsafe::ember, Variant::ashWood},
         {WallID::Unsafe::cinder, Variant::ashWood},
         {WallID::Unsafe::magma, Variant::ashWood},
         {WallID::Unsafe::smoulderingStone, Variant::ashWood},
         {WallID::Unsafe::crimsonGrass, Variant::flesh},
         {WallID::Unsafe::crimsandstone, Variant::flesh},
         {WallID::Unsafe::hardenedCrimsand, Variant::flesh},
         {WallID::Unsafe::crimstone, Variant::flesh},
         {WallID::Unsafe::crimsonCrust, Variant::flesh},
         {WallID::Unsafe::crimsonScab, Variant::flesh},
         {WallID::Unsafe::crimsonTeeth, Variant::flesh},
         {WallID::Unsafe::crimsonBlister, Variant::flesh},
         {WallID::Unsafe::snow, Variant::frozen},
         {WallID::Unsafe::ice, Variant::frozen},
         {WallID::Unsafe::corruptGrass, Variant::lesion},
         {WallID::Unsafe::ebonsandstone, Variant::lesion},
         {WallID::Unsafe::hardenedEbonsand, Variant::lesion},
         {WallID::Unsafe::ebonstone, Variant::lesion},
         {WallID::Unsafe::corruptGrowth, Variant::lesion},
         {WallID::Unsafe::corruptMass, Variant::lesion},
         {WallID::Unsafe::corruptPustule, Variant::lesion},
         {WallID::Unsafe::corruptTendril, Variant::lesion},
         {WallID::Unsafe::granite, Variant::granite},
         {WallID::Unsafe::marble, Variant::marble},
         {WallID::Unsafe::mushroom, Variant::mushroom},
         {WallID::Unsafe::hallowedGrass, Variant::pearlwood},
         {WallID::Unsafe::pearlsandstone, Variant::pearlwood},
         {WallID::Unsafe::hardenedPearlsand, Variant::pearlwood},
         {WallID::Unsafe::hallowedPrism, Variant::pearlwood},
         {WallID::Unsafe::hallowedCavern, Variant::pearlwood},
         {WallID::Unsafe::hallowedShard, Variant::pearlwood},
         {WallID::Unsafe::hallowedCrystalline, Variant::pearlwood},
         {WallID::Unsafe::jungle, Variant::richMahogany},
         {WallID::Unsafe::mud, Variant::richMahogany},
         {WallID::Unsafe::lichenStone, Variant::richMahogany},
         {WallID::Unsafe::leafyJungle, Variant::richMahogany},
         {WallID::Unsafe::ivyStone, Variant::richMahogany},
         {WallID::Unsafe::jungleVine, Variant::richMahogany},
         {WallID::Unsafe::sandstone, Variant::sandstone},
         {WallID::Safe::sandstoneBrick, Variant::sandstone},
         {WallID::Unsafe::hardenedSand, Variant::sandstone}});
    std::map<Variant, int> zoneCounts;
    int radius = 5;
    for (int i = -radius; i < radius; ++i) {
        for (int j = -radius; j < radius; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            auto blockItr = blockTypes.find(tile.blockID);
            if (blockItr != blockTypes.end()) {
                zoneCounts[blockItr->second] += 1;
            }
            auto wallItr = wallTypes.find(tile.wallID);
            if (wallItr != wallTypes.end()) {
                zoneCounts[wallItr->second] += 1;
            }
        }
    }
    if (zoneCounts[Variant::meteorite] > 0) {
        return Variant::meteorite;
    }
    for (auto [type, count] : zoneCounts) {
        if (count > radius * 4) {
            if (type == Variant::sandstone &&
                fuzzyIsSurfaceChest(x, y, world)) {
                return Variant::palmWood;
            } else if (type == Variant::pearlwood) {
                for (auto altType :
                     {Variant::mushroom,
                      Variant::marble,
                      Variant::granite,
                      Variant::richMahogany}) {
                    if (zoneCounts[altType] > radius) {
                        return altType;
                    }
                }
            }
            return type;
        }
    }
    for (auto evil : {Variant::flesh, Variant::lesion}) {
        if (zoneCounts[evil] > 2) {
            return evil;
        }
    }
    return fuzzyIsSurfaceChest(x, y, world) ? Variant::none : Variant::gold;
}

void placeStarterChest(Random &rnd, World &world)
{
    int centerX = world.getWidth() / 2;
    for (int iSwap = 0; iSwap < 20; ++iSwap) {
        int x = iSwap % 2 == 0 ? centerX - iSwap / 2 : centerX + iSwap / 2;
        int surfaceLevel = world.getSurfaceLevel(x);
        for (int y = surfaceLevel - 4; y < surfaceLevel + 4; ++y) {
            if (isPlacementCandidate(x, y, world)) {
                Chest &chest =
                    world.placeChest(x, y - 2, getChestType(x, y, world));
                fillStarterChest(world.conf.equipment, chest, rnd, world);
                return;
            }
        }
    }
}

void placeChest(
    int x,
    int y,
    Variant type,
    Variant origType,
    Random &rnd,
    World &world)
{
    Chest &chest = world.placeChest(x, y - 2, type);
    int torchID = ItemID::torch;
    switch (type) {
    case Variant::ashWood:
        fillSurfaceAshWoodChest(chest, rnd, world);
        return;
    case Variant::flesh:
        torchID = ItemID::crimsonTorch;
        break;
    case Variant::frozen:
        if (fuzzyIsSurfaceChest(x, y, world)) {
            fillSurfaceFrozenChest(chest, rnd, world);
        } else if (y < world.getCavernLevel()) {
            fillUndergroundFrozenChest(chest, rnd, world);
        } else {
            fillCavernFrozenChest(chest, rnd, world);
        }
        return;
    case Variant::goldLocked:
        fillDungeonChest(chest, rnd, world);
        return;
    case Variant::granite:
        torchID = ItemID::blueTorch;
        break;
    case Variant::honey:
        if (y < world.getCavernLevel()) {
            fillUndergroundHoneyChest(chest, rnd, world);
        } else {
            fillCavernHoneyChest(chest, rnd, world);
        }
        return;
    case Variant::lesion:
        torchID = ItemID::corruptTorch;
        break;
    case Variant::lihzahrd:
        fillLihzahrdChest(chest, rnd, world);
        return;
    case Variant::marble:
        torchID = ItemID::whiteTorch;
        break;
    case Variant::meteorite:
        torchID = ItemID::aetherTorch;
        break;
    case Variant::mushroom:
        if (y < world.getCavernLevel()) {
            fillUndergroundMushroomChest(chest, rnd, world);
        } else {
            fillCavernMushroomChest(chest, rnd, world);
        }
        return;
    case Variant::palmWood:
        fillSurfacePalmWoodChest(chest, rnd, world);
        return;
    case Variant::pearlwood:
        if (fuzzyIsSurfaceChest(x, y, world)) {
            fillSurfacePearlwoodChest(chest, rnd, world);
        } else if (y < world.getCavernLevel()) {
            fillUndergroundPearlwoodChest(chest, rnd, world);
        } else {
            fillCavernPearlwoodChest(chest, rnd, world);
        }
        return;
    case Variant::reef:
    case Variant::water:
        if (y < world.getUndergroundLevel()) {
            fillSurfaceWaterChest(chest, rnd, world);
        } else if (y < world.getCavernLevel()) {
            fillUndergroundWaterChest(chest, rnd, world);
        } else {
            fillCavernWaterChest(chest, rnd, world);
        }
        return;
    case Variant::richMahogany:
        if (fuzzyIsSurfaceChest(x, y, world)) {
            fillSurfaceRichMahoganyChest(chest, rnd, world);
        } else if (y < world.getCavernLevel()) {
            fillUndergroundRichMahoganyChest(chest, rnd, world);
        } else {
            fillCavernRichMahoganyChest(chest, rnd, world);
        }
        return;
    case Variant::sandstone:
        if (y < world.getCavernLevel()) {
            fillUndergroundSandstoneChest(chest, rnd, world);
        } else {
            fillCavernSandstoneChest(chest, rnd, world);
        }
        return;
    case Variant::shadow:
        fillShadowChest(chest, rnd, world);
        return;
    case Variant::skyware:
        fillSkywareChest(chest, rnd, world);
        return;
    default:
        break;
    }
    bool isTrapped = type == Variant::deadMans;
    if (isTrapped && origType == Variant::richMahogany) {
        torchID = ItemID::jungleTorch;
    }
    if (fuzzyIsSurfaceChest(x, y, world)) {
        fillSurfaceChest(chest, torchID, rnd, world);
    } else if (y < world.getCavernLevel()) {
        fillUndergroundChest(chest, torchID, isTrapped, rnd, world);
    } else {
        fillCavernChest(chest, torchID, isTrapped, rnd, world);
    }
    if (isTrapped) {
        world.queuedTraps.emplace_back([x, y](Random &rnd, World &world) {
            addChestTraps(x, y - 2, rnd, world);
        });
    }
}

void placeChests(int maxBin, LocationBins &locations, Random &rnd, World &world)
{
    int chestCount =
        world.conf.chests * world.getWidth() * world.getHeight() / 41800 -
        world.getChests().size();
    std::vector<Point> usedLocations;
    for (auto &chest : world.getChests()) {
        usedLocations.emplace_back(chest.x, chest.y);
    }
    while (chestCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        int surface = world.getSurfaceLevel(x);
        if (!isPlacementCandidate(x, y, world) ||
            isLocationUsed(
                x,
                y,
                y > surface                              ? 20
                : y < 0.45 * world.getUndergroundLevel() ? 125
                                                         : 50,
                usedLocations)) {
            continue;
        }
        Variant type = getChestType(x, y, world);
        Variant origType = Variant::none;
        if (y < surface + 3 && y > surface - 8 && type != Variant::reef &&
            type != Variant::water && rnd.getDouble(0, 1) < 0.85) {
            continue;
        } else if (
            y < world.getUndergroundLevel() && type == Variant::goldLocked) {
            continue;
        } else if (
            type == Variant::shadow &&
            world.getTile(x, y).blockID == TileID::ash) {
            continue;
        } else if (
            type == Variant::gold || (type == Variant::richMahogany &&
                                      y > std::midpoint(
                                              world.getUndergroundLevel(),
                                              world.getCavernLevel()))) {
            int threshold = std::lerp(0, 220, world.conf.traps);
            if (type == Variant::richMahogany) {
                threshold = std::min(300, threshold);
            }
            if (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 907 <
                threshold) {
                origType = type;
                type = Variant::deadMans;
            }
        }
        usedLocations.emplace_back(x, y);
        if (type == Variant::gold ||
            (origType == Variant::gold && world.conf.traps > 1.8)) {
            maybePlaceCabinForChest(x, y, rnd, world);
        }
        placeChest(x, y, type, origType, rnd, world);
        --chestCount;
    }
}

Variant getPotType(int x, int y, World &world)
{
    switch (getChestType(x, y, world)) {
    case Variant::flesh:
        return Variant::crimson;
    case Variant::frozen:
        return Variant::tundra;
    case Variant::goldLocked:
        return Variant::dungeon;
    case Variant::honey:
    case Variant::richMahogany:
        return Variant::jungle;
    case Variant::lesion:
        return Variant::corruption;
    case Variant::lihzahrd:
        return Variant::lihzahrd;
    case Variant::marble:
        return Variant::marble;
    case Variant::sandstone:
        return Variant::desert;
    case Variant::shadow:
        return Variant::underworld;
    case Variant::spider:
        return Variant::spider;
    default:
        return Variant::forest;
    }
}

void placePots(int maxBin, LocationBins &locations, Random &rnd, World &world)
{
    int potCount =
        world.conf.pots * world.getWidth() * world.getHeight() / 10000;
    while (potCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (y < 0.85 * world.getUndergroundLevel() ||
            !isPlacementCandidate(x, y, world)) {
            continue;
        }
        world.placeFramedTile(x, y - 2, TileID::pot, getPotType(x, y, world));
        --potCount;
    }
}

void placeDirtiestBlocks(Random &rnd, World &world)
{
    int numBlocks = world.getWidth() * world.getHeight() / 1680000;
    while (numBlocks > 0) {
        int x = rnd.getInt(60, world.getWidth() - 60);
        int y = rnd.getInt(60, world.getHeight() - 60);
        Tile &tile = world.getTile(x, y);
        if (tile.blockID != TileID::dirt || world.isExposed(x, y)) {
            continue;
        }
        tile.blockID = TileID::dirtiestBlock;
        --numBlocks;
    }
}

LocationBins genTreasure(Random &rnd, World &world)
{
    std::cout << "Cataloging ground\n";
    std::vector<std::vector<Point>> rawLocations(world.getWidth());
    parallelFor(
        std::views::iota(50, world.getWidth() - 50),
        [&rawLocations, &world](int x) {
            for (int y = 50; y < world.getHeight() - 50; ++y) {
                if (isPlacementCandidate(x, y, world)) {
                    rawLocations[x].emplace_back(y, 0);
                }
                if (testOrbHeartCandidate(x, y, world) != TileID::empty) {
                    rawLocations[x].emplace_back(y, 1);
                }
            }
        });
    LocationBins flatLocations;
    LocationBins orbHeartLocations;
    for (size_t x = 0; x < rawLocations.size(); ++x) {
        for (auto [y, flag] : rawLocations[x]) {
            if (flag == 0) {
                flatLocations[binLocation(x, y, world.getHeight())]
                    .emplace_back(x, y);
            } else {
                orbHeartLocations[binLocation(x, y, world.getHeight())]
                    .emplace_back(x, y);
            }
        }
    }
    std::cout << "Placing treasures\n";
    if (!world.conf.home && world.conf.equipment != 0) {
        placeStarterChest(rnd, world);
    }
    std::shuffle(
        world.queuedTreasures.begin(),
        world.queuedTreasures.end(),
        rnd.getPRNG());
    for (const auto &applyQueuedTreasure : world.queuedTreasures) {
        applyQueuedTreasure(rnd, world);
    }
    int maxBin =
        binLocation(world.getWidth(), world.getHeight(), world.getHeight());
    placeJungleShrines(rnd, world);
    placeLifeCrystals(maxBin, flatLocations, rnd, world);
    placeFallenLogs(maxBin, flatLocations, rnd, world);
    if (world.conf.purity) {
        placePurityAltars(rnd, world);
    } else {
        placeAltars(maxBin, flatLocations, rnd, world);
        placeOrbHearts(maxBin, orbHeartLocations, rnd, world);
    }
    placeManaCrystals(maxBin, flatLocations, rnd, world);
    placeChests(maxBin, flatLocations, rnd, world);
    placePots(maxBin, flatLocations, rnd, world);
    placeGems(rnd, world);
    placeDirtiestBlocks(rnd, world);
    return flatLocations;
}
