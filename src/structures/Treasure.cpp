#include "structures/Treasure.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
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
    for (int numTries = 0; larvaCount > 0 && numTries < 100; ++numTries) {
        auto [x, y] = rnd.select(hiveLocations);
        Tile &tile = world.getTile(x, y - 1);
        if (tile.wallID == WallID::Unsafe::hive &&
            tile.liquid == Liquid::none &&
            isPlacementCandidate(x - 1, y, world) &&
            isPlacementCandidate(x + 2, y, world)) {
            world.placeFramedTile(x, y - 3, TileID::larva);
            --larvaCount;
        }
    }
}

void fillLoot(
    Chest &chest,
    Random &rnd,
    std::initializer_list<std::pair<double, Item>> loot)
{
    int itemIndex = 0;
    for (auto [probability, item] : loot) {
        if (probability > rnd.getDouble(0, 1)) {
            chest.items[itemIndex] = item;
            ++itemIndex;
        }
    }
}

void placeChests(int maxBin, LocationBins &locations, Random &rnd, World &world)
{
    int chestCount = world.getWidth() * world.getHeight() / 50000;
    while (chestCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        if (isPlacementCandidate(x, y, world)) {
            Chest &chest = world.placeChest(x, y - 2);
            fillLoot(
                chest,
                rnd,
                {{1,
                  rnd.select<Item>({
                      {ItemID::spear, rnd.select(PrefixSet::universal), 1},
                      {ItemID::blowpipe, rnd.select(PrefixSet::ranged), 1},
                      {ItemID::woodenBoomerang,
                       rnd.select(PrefixSet::universal),
                       1},
                      {ItemID::aglet, rnd.select(PrefixSet::accessory), 1},
                      {ItemID::climbingClaws,
                       rnd.select(PrefixSet::accessory),
                       1},
                      {ItemID::umbrella, rnd.select(PrefixSet::melee), 1},
                      {ItemID::guideToPlantFiberCordage,
                       rnd.select(PrefixSet::accessory),
                       1},
                      {ItemID::wandOfSparking, rnd.select(PrefixSet::magic), 1},
                      {ItemID::radar, rnd.select(PrefixSet::accessory), 1},
                      {ItemID::stepStool, rnd.select(PrefixSet::accessory), 1},
                  })},
                 {1.0 / 6,
                  {ItemID::glowstick, Prefix::none, rnd.getInt(40, 75)}},
                 {1.0 / 6,
                  {ItemID::throwingKnife, Prefix::none, rnd.getInt(150, 300)}},
                 {1.0 / 6, {ItemID::herbBag, Prefix::none, rnd.getInt(1, 4)}},
                 {1.0 / 6,
                  {ItemID::canOfWorms, Prefix::none, rnd.getInt(1, 4)}},
                 {1.0 / 3, {ItemID::grenade, Prefix::none, rnd.getInt(3, 5)}},
                 {0.5,
                  {rnd.select({ItemID::copperBar, ItemID::tinBar}),
                   Prefix::none,
                   rnd.getInt(3, 10)}},
                 {0.5,
                  {rnd.select({ItemID::ironBar, ItemID::leadBar}),
                   Prefix::none,
                   rnd.getInt(3, 10)}},
                 {0.5, {ItemID::rope, Prefix::none, rnd.getInt(50, 100)}},
                 {2.0 / 3,
                  {rnd.select({ItemID::woodenArrow, ItemID::shuriken}),
                   Prefix::none,
                   rnd.getInt(25, 50)}},
                 {0.5,
                  {ItemID::lesserHealingPotion,
                   Prefix::none,
                   rnd.getInt(3, 5)}},
                 {2.0 / 3,
                  {ItemID::recallPotion, Prefix::none, rnd.getInt(3, 5)}},
                 {2.0 / 3,
                  {rnd.select(
                       {ItemID::ironskinPotion,
                        ItemID::shinePotion,
                        ItemID::nightOwlPotion,
                        ItemID::swiftnessPotion,
                        ItemID::miningPotion,
                        ItemID::builderPotion}),
                   Prefix::none,
                   rnd.getInt(1, 2)}},
                 {0.5,
                  {rnd.select({ItemID::torch, ItemID::bottle}),
                   Prefix::none,
                   rnd.getInt(10, 20)}},
                 {0.5, {ItemID::silverCoin, Prefix::none, rnd.getInt(10, 29)}},
                 {0.5, {ItemID::wood, Prefix::none, rnd.getInt(50, 99)}}});
            --chestCount;
        }
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
