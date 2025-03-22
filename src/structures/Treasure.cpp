#include "structures/Treasure.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include <iostream>
#include <map>
#include <set>
#include <vector>

bool isPlacementCandidate(int x, int y, World &world)
{
    if (world.getTile(x, y).blockID == TileID::empty ||
        world.getTile(x + 1, y).blockID == TileID::empty) {
        return false;
    }
    for (int i = 0; i < 2; ++i) {
        for (int j = -3; j < 0; ++j) {
            if (world.getTile(x + i, y + j).blockID != TileID::empty) {
                return false;
            }
        }
    }
    return true;
}

int binLocation(int x, int y, int maxY)
{
    int factor = 128;
    x /= factor;
    y /= factor;
    maxY /= factor;
    return x * maxY + y;
}

void genTreasure(Random &rnd, World &world)
{
    std::cout << "Cataloging ground\n";
    std::vector<std::vector<int>> rawLocations(world.getWidth());
    parallelFor(
        std::views::iota(50, world.getWidth() - 50),
        [&rawLocations, &world](int x) {
            for (int y = 50; y < world.getHeight() - 50; ++y) {
                if (isPlacementCandidate(x, y, world)) {
                    rawLocations[x].push_back(y);
                }
            }
        });
    std::map<int, std::vector<std::pair<int, int>>> locations;
    for (size_t x = 0; x < rawLocations.size(); ++x) {
        for (int y : rawLocations[x]) {
            locations[binLocation(x, y, world.getHeight())].emplace_back(x, y);
        }
    }
    std::cout << "Placing treasures\n";
    int lifeCrystalCount = world.getWidth() * world.getHeight() / 50000;
    int maxBin =
        binLocation(world.getWidth(), world.getHeight(), world.getHeight());
    while (lifeCrystalCount > 0) {
        int binId = rnd.getInt(0, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] =
            rnd.select(locations[binId].begin(), locations[binId].end());
        if (y > world.getUndergroundLevel() && y < world.getUnderworldLevel() &&
            isPlacementCandidate(x, y, world)) {
            world.placeFramedTile(x, y - 2, TileID::lifeCrystal);
            --lifeCrystalCount;
        }
    }
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
        auto [x, y] =
            rnd.select(locations[binId].begin(), locations[binId].end());
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
