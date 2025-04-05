#include "structures/Treasure.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/ItemID.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Plants.h"
#include "structures/data/Shrines.h"
#include <iostream>
#include <set>

template <typename T, typename U>
bool listContains(const T &list, const U &value)
{
    return std::find(std::begin(list), std::end(list), value) != std::end(list);
}

inline const std::set<int> nonSolidTiles{
    TileID::empty,
    TileID::alchemyTable,
    TileID::bench,
    TileID::bewitchingTable,
    TileID::boneWelder,
    TileID::book,
    TileID::bottle,
    TileID::bubble,
    TileID::chest,
    TileID::chestGroup2,
    TileID::lihzahrdAltar,
    TileID::painting3x3,
    TileID::rope,
    TileID::silverCoin,
    TileID::statue,
    TileID::thinIce,
    TileID::waterCandle,
};

bool isPlacementCandidate(int x, int y, World &world)
{
    if (nonSolidTiles.contains(world.getTile(x, y).blockID) ||
        nonSolidTiles.contains(world.getTile(x + 1, y).blockID)) {
        return false;
    }
    return world.regionPasses(x, y - 3, 2, 3, [](Tile &tile) {
        return tile.blockID == TileID::empty && tile.liquid != Liquid::lava &&
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
    std::set<int> allowedTiles{
        TileID::clay,
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
        TileID::flesh};
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

int binLocation(int x, int y, int maxY)
{
    int factor = 128;
    x /= factor;
    y /= factor;
    maxY /= factor;
    return x * maxY + y;
}

void placeLifeCrystals(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int lifeCrystalCount = world.getWidth() * world.getHeight() / 50000;
    int maxDungeonPlacements = 3;
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
            } else if (probeWall == WallID::Unsafe::lihzahrdBrick) {
                continue;
            } else {
                world.placeFramedTile(x, y - 2, TileID::lifeCrystal);
            }
            --lifeCrystalCount;
        }
    }
}

void placeAltars(int maxBin, LocationBins &locations, Random &rnd, World &world)
{
    int altarCount = std::max(8, world.getWidth() / 200);
    std::set<int> corruptTiles{
        TileID::ebonstone,
        TileID::corruptGrass,
        TileID::demonite,
        TileID::ebonsand,
        TileID::corruptJungleGrass,
        TileID::corruptIce,
        TileID::ebonsandstone,
        TileID::hardenedEbonsand,
        TileID::lesion};
    std::set<int> crimsonTiles{
        TileID::crimstone,
        TileID::crimsonGrass,
        TileID::crimtane,
        TileID::crimsand,
        TileID::crimsonJungleGrass,
        TileID::crimsonIce,
        TileID::crimsandstone,
        TileID::hardenedCrimsand,
        TileID::flesh};
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

void placeOrbHearts(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int orbHeartCount = world.getWidth() * world.getHeight() / 240000;
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

void placeLarvae(LocationBins &locations, Random &rnd, World &world)
{
    std::vector<Point> hiveLocations;
    for (const auto &locBin : locations) {
        for (auto [x, y] : locBin.second) {
            if (world.getTile(x, y - 1).wallID == WallID::Unsafe::hive) {
                hiveLocations.emplace_back(x, y);
            }
        }
    }
    int larvaCount = rnd.getInt(3, 6);
    int lastX = -1;
    int lastY = -1;
    for (int numTries = 0; larvaCount > 0 && numTries < 100; ++numTries) {
        auto [x, y] = rnd.select(hiveLocations);
        if (std::hypot(lastX - x, lastY - y) < 75) {
            continue;
        }
        Tile &tile = world.getTile(x, y - 1);
        if (tile.wallID == WallID::Unsafe::hive &&
            tile.liquid == Liquid::none &&
            isPlacementCandidate(x - 1, y, world) &&
            isPlacementCandidate(x + 2, y, world)) {
            world.placeFramedTile(x, y - 3, TileID::larva);
            lastX = x;
            lastY = y;
            --larvaCount;
        }
    }
}

void placeManaCrystals(
    int maxBin,
    LocationBins &locations,
    Random &rnd,
    World &world)
{
    int manaCrystalCount = world.getWidth() * world.getHeight() / 250000;
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

Point selectShrineLocation(TileBuffer &shrine, Random &rnd, World &world)
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
    std::set<int> clearableTiles{
        TileID::empty,
        TileID::dirt,
        TileID::mud,
        TileID::jungleGrass,
        TileID::stone,
        TileID::clay,
        TileID::silt,
        TileID::copperOre,
        TileID::tinOre,
        TileID::ironOre,
        TileID::leadOre,
        TileID::silverOre,
        TileID::tungstenOre,
        TileID::goldOre,
        TileID::platinumOre};
    while (true) {
        int x = rnd.getInt(
            world.jungleCenter - 0.09 * world.getWidth(),
            world.jungleCenter + 0.09 * world.getWidth());
        int y =
            rnd.getInt(world.getUndergroundLevel(), world.getUnderworldLevel());
        if (!world.regionPasses(
                x,
                y,
                shrine.getWidth(),
                std::max(minOpenHeight, 3),
                [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           (tile.wallID == WallID::empty ||
                            listContains(WallVariants::jungle, tile.wallID));
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
                    })) {
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
                // Restricted tiles.
                !world.regionPasses(
                    x,
                    y,
                    shrine.getWidth(),
                    shrine.getHeight(),
                    [&clearableTiles](Tile &tile) {
                        return !tile.guarded &&
                               clearableTiles.contains(tile.blockID);
                    })) {
                break;
            }
            return {x, y};
        }
    }
}

void placeJungleShrines(Random &rnd, World &world)
{
    int shrineCount = world.getWidth() * world.getHeight() / 600000;
    while (shrineCount > 0) {
        TileBuffer shrine =
            Data::getShrine(rnd.select(Data::shrines), world.getFramedTiles());
        auto [x, y] = selectShrineLocation(shrine, rnd, world);
        std::set<Point> chests;
        for (int i = 0; i < shrine.getWidth(); ++i) {
            for (int j = 0; j < shrine.getHeight(); ++j) {
                Tile &shrineTile = shrine.getTile(i, j);
                if (shrineTile.blockID == TileID::cloud) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                if (shrineTile.blockID == TileID::chest &&
                    !chests.contains({i - 1, j}) &&
                    !chests.contains({i, j - 1}) &&
                    !chests.contains({i - 1, j - 1})) {
                    chests.emplace(i, j);
                }
                if (shrineTile.wallID == WallID::empty) {
                    shrineTile.wallID = tile.wallID;
                }
                tile = shrineTile;
                tile.guarded = true;
            }
        }
        for (int i = 0; i < shrine.getWidth(); ++i) {
            for (int j = 0; j < shrine.getHeight(); ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::mud &&
                    world.isExposed(x + i, y + j)) {
                    tile.blockID = TileID::jungleGrass;
                }
            }
        }
        for (auto [i, j] : chests) {
            Chest &chest = world.placeChest(x + i, y + j, Variant::ivy);
            if (y < world.getCavernLevel()) {
                fillUndergroundIvyChest(chest, rnd, world);
            } else {
                fillCavernIvyChest(chest, rnd, world);
            }
        }
        --shrineCount;
    }
}

Variant getChestType(int x, int y, World &world)
{
    Tile &probeTile = world.getTile(x, y - 2);
    if (y > world.getUnderworldLevel()) {
        return Variant::shadow;
    } else if (probeTile.wallID == WallID::Unsafe::hive) {
        return Variant::honey;
    } else if (probeTile.liquid == Liquid::water) {
        return y < world.getUndergroundLevel() &&
                       (x < 350 || x > world.getWidth() - 350)
                   ? Variant::reef
                   : Variant::water;
    } else if (listContains(WallVariants::dungeon, probeTile.wallID)) {
        return Variant::goldLocked;
    } else if (probeTile.wallID == WallID::Unsafe::lihzahrdBrick) {
        return Variant::lihzahrd;
    }
    std::map<int, Variant> blockTypes{
        {TileID::crimstone, Variant::flesh},
        {TileID::crimsonGrass, Variant::flesh},
        {TileID::crimsand, Variant::flesh},
        {TileID::crimsonJungleGrass, Variant::flesh},
        {TileID::crimsonIce, Variant::flesh},
        {TileID::crimsandstone, Variant::flesh},
        {TileID::hardenedCrimsand, Variant::flesh},
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
        {TileID::jungleGrass, Variant::richMahogany},
        {TileID::sand, Variant::sandstone},
        {TileID::sandstone, Variant::sandstone},
        {TileID::hardenedSand, Variant::sandstone},
        {TileID::desertFossil, Variant::sandstone}};
    std::map<int, Variant> wallTypes{
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
        {WallID::Unsafe::jungle, Variant::richMahogany},
        {WallID::Unsafe::mud, Variant::richMahogany},
        {WallID::Unsafe::lichenStone, Variant::richMahogany},
        {WallID::Unsafe::leafyJungle, Variant::richMahogany},
        {WallID::Unsafe::ivyStone, Variant::richMahogany},
        {WallID::Unsafe::jungleVine, Variant::richMahogany},
        {WallID::Unsafe::sandstone, Variant::sandstone},
        {WallID::Unsafe::hardenedSand, Variant::sandstone}};
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
            if (type == Variant::sandstone && y < world.getUndergroundLevel()) {
                return Variant::palmWood;
            }
            return type;
        }
    }
    for (auto evil : {Variant::flesh, Variant::lesion}) {
        if (zoneCounts[evil] > 2) {
            return evil;
        }
    }
    return y < world.getUndergroundLevel() ? Variant::none : Variant::gold;
}

void placeChest(int x, int y, Variant type, Random &rnd, World &world)
{
    Chest &chest = world.placeChest(x, y - 2, type);
    int torchID = ItemID::torch;
    switch (type) {
    case Variant::flesh:
        torchID = ItemID::crimsonTorch;
        break;
    case Variant::frozen:
        if (y < world.getUndergroundLevel()) {
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
        fillLihzahrdChest(chest, rnd);
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
        if (y < world.getUndergroundLevel()) {
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
    default:
        break;
    }
    if (y < world.getUndergroundLevel()) {
        fillSurfaceChest(chest, torchID, rnd, world);
    } else if (y < world.getCavernLevel()) {
        fillUndergroundChest(chest, torchID, rnd, world);
    } else {
        fillCavernChest(chest, torchID, rnd, world);
    }
}

void placeChests(int maxBin, LocationBins &locations, Random &rnd, World &world)
{
    int chestCount = world.getWidth() * world.getHeight() / 50000;
    LocationBins usedLocations;
    for (auto &chest : world.getChests()) {
        usedLocations[binLocation(chest.x, chest.y, world.getHeight())]
            .emplace_back(chest.x, chest.y);
    }
    while (chestCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (!isPlacementCandidate(x, y, world)) {
            continue;
        }
        bool isNearOtherChests = false;
        for (auto [usedX, usedY] : usedLocations[binId]) {
            if (std::hypot(usedX - x, usedY - y) < 20) {
                isNearOtherChests = true;
                break;
            }
        }
        if (isNearOtherChests) {
            continue;
        }
        usedLocations[binId].emplace_back(x, y);
        Variant type = getChestType(x, y, world);
        placeChest(x, y, type, rnd, world);
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
    int potCount = world.getWidth() * world.getHeight() / 10000;
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

void genTreasure(Random &rnd, World &world)
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
    int maxBin =
        binLocation(world.getWidth(), world.getHeight(), world.getHeight());
    placeJungleShrines(rnd, world);
    placeLarvae(flatLocations, rnd, world);
    placeLifeCrystals(maxBin, flatLocations, rnd, world);
    placeAltars(maxBin, flatLocations, rnd, world);
    placeOrbHearts(maxBin, orbHeartLocations, rnd, world);
    placeManaCrystals(maxBin, flatLocations, rnd, world);
    placeChests(maxBin, flatLocations, rnd, world);
    placePots(maxBin, flatLocations, rnd, world);
    genPlants(flatLocations, rnd, world);
}
