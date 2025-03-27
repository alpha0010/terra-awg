#include "structures/Treasure.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/ItemID.h"
#include "ids/Prefix.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include <iostream>
#include <map>
#include <set>
#include <vector>

typedef std::map<int, std::vector<std::pair<int, int>>> LocationBins;

bool isSolid(int tileId)
{
    return tileId != TileID::empty && tileId != TileID::thinIce &&
           tileId != TileID::bubble;
}

bool isPlacementCandidate(int x, int y, World &world)
{
    if (!isSolid(world.getTile(x, y).blockID) ||
        !isSolid(world.getTile(x + 1, y).blockID)) {
        return false;
    }
    for (int i = 0; i < 2; ++i) {
        for (int j = -3; j < 0; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.blockID != TileID::empty || tile.liquid == Liquid::lava ||
                tile.liquid == Liquid::shimmer) {
                return false;
            }
        }
    }
    return true;
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
    while (lifeCrystalCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (y > world.getUndergroundLevel() && y < world.getUnderworldLevel() &&
            isPlacementCandidate(x, y, world)) {
            world.placeFramedTile(x, y - 2, TileID::lifeCrystal);
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
    std::vector<std::pair<int, int>> hiveLocations;
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

Variant getChestType(int x, int y, World &world)
{
    if (y > world.getUnderworldLevel()) {
        return Variant::shadow;
    } else if (world.getTile(x, y - 1).wallID == WallID::Unsafe::hive) {
        return Variant::honey;
    } else if (world.getTile(x, y - 2).liquid == Liquid::water) {
        return y < world.getUndergroundLevel() &&
                       (x < 350 || x > world.getWidth() - 350)
                   ? Variant::reef
                   : Variant::water;
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
    for (auto [type, count] : zoneCounts) {
        if (count > radius * 4) {
            if (type == Variant::sandstone && y < world.getUndergroundLevel()) {
                return Variant::palmWood;
            }
            return type;
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
    case Variant::jungle:
        fillDungeonBiomeChest(
            chest,
            rnd,
            {ItemID::piranhaGun, rnd.select(PrefixSet::ranged), 1});
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

void genTreasure(Random &rnd, World &world)
{
    std::cout << "Cataloging ground\n";
    std::vector<std::vector<std::pair<int, int>>> rawLocations(
        world.getWidth());
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
    placeLarvae(flatLocations, rnd, world);
    placeLifeCrystals(maxBin, flatLocations, rnd, world);
    placeAltars(maxBin, flatLocations, rnd, world);
    placeOrbHearts(maxBin, orbHeartLocations, rnd, world);
    placeChests(maxBin, flatLocations, rnd, world);
}
