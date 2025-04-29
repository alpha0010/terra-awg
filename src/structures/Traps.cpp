#include "structures/Traps.h"

#include "Random.h"
#include "World.h"
#include "structures/StructureUtil.h"
#include <algorithm>
#include <iostream>
#include <set>

inline const std::set<int> trappableTiles{
    TileID::empty,
    TileID::dirt,
    TileID::stone,
    TileID::grass,
    TileID::corruptGrass,
    TileID::ebonstone,
    TileID::clay,
    TileID::mud,
    TileID::jungleGrass,
    TileID::mushroomGrass,
    TileID::snow,
    TileID::ice,
    TileID::corruptIce,
    TileID::slime,
    TileID::crimsonGrass,
    TileID::crimsonIce,
    TileID::crimstone,
    TileID::smoothMarble,
    TileID::marble,
    TileID::granite,
    TileID::smoothGranite,
    TileID::lavaMossStone,
    TileID::sandstone,
    TileID::hardenedSand,
    TileID::hardenedEbonsand,
    TileID::hardenedCrimsand,
    TileID::ebonsandstone,
    TileID::crimsandstone,
    TileID::kryptonMossStone,
    TileID::xenonMossStone,
    TileID::argonMossStone,
    TileID::neonMossStone,
    TileID::corruptJungleGrass,
    TileID::crimsonJungleGrass,
};

bool isTrappable(Tile &tile)
{
    return !tile.guarded && trappableTiles.contains(tile.blockID);
}

void placeSandTraps(Random &rnd, World &world)
{
    double scanDist = 0.065 * world.getWidth();
    LocationBins locations;
    for (int x = world.desertCenter - scanDist;
         x < world.desertCenter + scanDist;
         ++x) {
        int fallingCount = 0;
        for (int y = world.getUndergroundLevel();
             y < world.getUnderworldLevel();
             ++y) {
            Tile &tile = world.getTile(x, y);
            switch (tile.blockID) {
            case TileID::sand:
            case TileID::ebonsand:
            case TileID::crimsand:
                ++fallingCount;
                break;
            case TileID::empty:
                if (fallingCount > 3) {
                    locations[binLocation(x, y, world.getUnderworldLevel())]
                        .emplace_back(x, y);
                }
                [[fallthrough]];
            default:
                fallingCount = 0;
            }
        }
    }
    int minBin = binLocation(
        world.desertCenter - scanDist,
        world.getUndergroundLevel(),
        world.getUnderworldLevel());
    int maxBin = binLocation(
        world.desertCenter + scanDist,
        world.getUnderworldLevel(),
        world.getUnderworldLevel());
    int numSandTraps =
        world.getWidth() * world.getHeight() / rnd.getInt(240000, 360000);
    std::set<int> validFloors{
        TileID::sand,
        TileID::hardenedSand,
        TileID::sandstone,
        TileID::ebonsand,
        TileID::hardenedEbonsand,
        TileID::ebonsandstone,
        TileID::crimsand,
        TileID::hardenedCrimsand,
        TileID::crimsandstone};
    std::set<int> looseBlocks{TileID::sand, TileID::ebonsand, TileID::crimsand};
    while (numSandTraps > 0) {
        int binId = rnd.getInt(minBin, maxBin);
        if (locations[binId].empty()) {
            continue;
        }
        auto [x, y] = rnd.select(locations[binId]);
        int trapFloor = scanWhileEmpty({x, y}, {0, 1}, world).second;
        if (trapFloor - y < 4 ||
            !validFloors.contains(world.getTile(x, trapFloor + 1).blockID)) {
            continue;
        }
        world.getTile(x, trapFloor).blockID = TileID::pressurePlate;
        world.getTile(x, trapFloor).frameY = 126; // TODO
        placeWire({x, trapFloor}, {x, y - 1}, Wire::red, world);
        Point prevActuator{-1, -1};
        for (int i = -5; i < 5; ++i) {
            int trapCeiling =
                scanWhileEmpty({x + i, (y + trapFloor) / 2}, {0, -1}, world)
                    .second -
                1;
            Tile &tile = world.getTile(x + i, trapCeiling);
            if (looseBlocks.contains(tile.blockID) ||
                (validFloors.contains(tile.blockID) &&
                 looseBlocks.contains(
                     world.getTile(x + i, trapCeiling - 1).blockID))) {
                tile.actuator = true;
                if (prevActuator.first != -1) {
                    placeWire(
                        prevActuator,
                        {x + i, trapCeiling},
                        Wire::red,
                        world);
                }
                prevActuator = {x + i, trapCeiling};
            }
        }
        --numSandTraps;
    }
}

bool isValidBoulderPlacement(int x, int y, World &world)
{
    std::set<int> validTiles{
        TileID::crimstone,
        TileID::ebonstone,
        TileID::granite,
        TileID::sandstone,
        TileID::slime,
        TileID::stone};
    return world.regionPasses(x, y, 6, 6, [&validTiles](Tile &tile) {
        return !tile.guarded && validTiles.contains(tile.blockID);
    }) && world.regionPasses(x + 2, y + 6, 2, 1, [](Tile &tile) {
        return tile.blockID != TileID::empty;
    });
}

Point selectBoulderLocation(Random &rnd, World &world)
{
    while (true) {
        int x = rnd.getInt(100, world.getWidth() - 100);
        int y = rnd.getInt(
            0.85 * world.getUndergroundLevel(),
            world.getUnderworldLevel() - 100);
        if (isValidBoulderPlacement(x, y, world)) {
            while (isValidBoulderPlacement(x, y + 1, world)) {
                ++y;
            }
            return {x + 2, y + 2};
        }
    }
}

void placeBoulderTraps(Random &rnd, World &world)
{
    int numBoulders =
        world.getWidth() * world.getHeight() / rnd.getInt(57600, 64000);
    std::vector<Point> usedLocations;
    while (numBoulders > 0) {
        auto [x, y] = selectBoulderLocation(rnd, world);
        int trapFloor = y + 4;
        while (!world.regionPasses(x, trapFloor, 2, 3, [](Tile &tile) {
            return tile.blockID == TileID::empty;
        })) {
            ++trapFloor;
        }
        int trapX = rnd.select({x, x + 1});
        trapFloor = scanWhileEmpty({trapX, trapFloor}, {0, 1}, world).second;
        if (trapFloor > world.getUnderworldLevel() || trapFloor - y > 25 ||
            !world.regionPasses(
                x,
                y,
                2,
                trapFloor - y + 2,
                [](Tile &tile) {
                    return isTrappable(tile) && tile.liquid == Liquid::none;
                }) ||
            isLocationUsed(x, y, 20, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        int probeTileId = world.getTile(x, y).blockID;
        world.placeFramedTile(
            x,
            y,
            probeTileId == TileID::sandstone ? TileID::rollingCactus
            : probeTileId == TileID::slime   ? TileID::bouncyBoulder
                                             : TileID::boulder);
        for (int i = -2; i < 4; ++i) {
            for (int j = -2; j < 4; ++j) {
                world.getTile(x + i, y + j).guarded = true;
            }
        }
        for (int i = 0; i < 2; ++i) {
            for (int j = 2; j < trapFloor - y; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID != TileID::empty) {
                    tile.actuator = true;
                    tile.wireRed = true;
                } else if (x + i == trapX) {
                    tile.wireRed = true;
                }
            }
        }
        Tile &pressureTile = world.getTile(trapX, trapFloor);
        pressureTile.blockID = TileID::pressurePlate;
        pressureTile.frameY = 126;
        pressureTile.wireRed = true;
        --numBoulders;
    }
}

void placeLavaTraps(Random &rnd, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    std::vector<Point> locations;
    for (int x = 100; x < world.getWidth() - 100; ++x) {
        int lavaCount = 0;
        for (int y = lavaLevel; y < world.getUnderworldLevel() - 10; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.liquid == Liquid::lava) {
                ++lavaCount;
            } else {
                if (lavaCount > 5) {
                    locations.emplace_back(x, y);
                }
                lavaCount = 0;
            }
        }
    }
    std::vector<Point> usedLocations;
    double numLavaTraps =
        world.getWidth() * world.getHeight() / rnd.getInt(164000, 230400);
    while (numLavaTraps > 0) {
        auto [x, y] = rnd.select(locations);
        if (isLocationUsed(x, y, 15, usedLocations)) {
            numLavaTraps -= 0.1;
            continue;
        }
        int gapJ = 0;
        while (gapJ < 50 &&
               !world.regionPasses(x - 1, y + gapJ, 3, 3, [](Tile &tile) {
                   return tile.blockID == TileID::empty &&
                          tile.liquid == Liquid::none;
               })) {
            ++gapJ;
        }
        if (gapJ == 50) {
            continue;
        }
        int trapFloor = scanWhileEmpty({x, y + gapJ}, {0, 1}, world).second;
        if (trapFloor > world.getUnderworldLevel() ||
            !world.regionPasses(x, y, 1, trapFloor - y, isTrappable)) {
            continue;
        }
        std::vector<Point> plateLocs;
        for (int plateX = x - 4; plateX < x + 4; ++plateX) {
            for (int plateY = y + gapJ - 2; plateY < trapFloor + 4; ++plateY) {
                Tile &baseTile = world.getTile(plateX, plateY + 1);
                if (baseTile.blockID != TileID::empty &&
                    isTrappable(baseTile) &&
                    world
                        .regionPasses(plateX, plateY - 2, 1, 3, [](Tile &tile) {
                            return tile.blockID == TileID::empty &&
                                   tile.liquid == Liquid::none;
                        })) {
                    plateLocs.emplace_back(plateX, plateY);
                }
            }
        }
        if (plateLocs.empty()) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        for (int j = 0; j < gapJ; ++j) {
            Tile &tile = world.getTile(x, y + j);
            tile.wireRed = true;
            if (tile.blockID != TileID::empty) {
                tile.actuator = true;
                if (j < 2) {
                    tile.guarded = true;
                }
            }
        }
        auto [plateX, plateY] = rnd.select(plateLocs);
        Tile &pressureTile = world.getTile(plateX, plateY);
        pressureTile.blockID = TileID::pressurePlate;
        pressureTile.frameY = 126;
        placeWire({x, y + gapJ}, {plateX, plateY}, Wire::red, world);
        --numLavaTraps;
    }
}

void genTraps(Random &rnd, World &world)
{
    std::cout << "Arming traps\n";
    placeSandTraps(rnd, world);
    placeBoulderTraps(rnd, world);
    placeLavaTraps(rnd, world);
    for (const auto &applyQueuedTrap : world.queuedTraps) {
        applyQueuedTrap(rnd, world);
    }
}

bool addChestBoulderTraps(int x, int y, Random &rnd, World &world)
{
    std::vector<Point> traps;
    for (int i = rnd.getInt(-10, -7); i < 10; i += 4) {
        Point pos = scanWhileNotSolid({x + i, y}, {0, -1}, world);
        int dist = y - pos.second;
        if (dist < 5 || dist > 30) {
            continue;
        }
        pos = addPts(pos, {0, -5});
        if (world.regionPasses(
                pos.first - 2,
                pos.second - 2,
                6,
                6,
                [](Tile &tile) {
                    return tile.blockID != TileID::empty && isTrappable(tile);
                }) &&
            world.regionPasses(
                pos.first,
                pos.second,
                2,
                pos.second - y - 3,
                isTrappable)) {
            traps.push_back(pos);
        }
    }
    if (traps.empty()) {
        return false;
    }
    for (auto trap : traps) {
        placeWire({trap.first, trap.second + 2}, {x, y}, Wire::red, world);
        for (int i = 0; i < 2; ++i) {
            for (int wireY = trap.second + 2; wireY < y; ++wireY) {
                Tile &tile = world.getTile(trap.first + i, wireY);
                if (tile.blockID != TileID::empty) {
                    tile.actuator = true;
                    placeWire(
                        {trap.first, trap.second + 2},
                        {trap.first + i, wireY},
                        Wire::red,
                        world);
                }
            }
        }
        world.placeFramedTile(trap.first, trap.second, TileID::boulder);
    }
    return true;
}

bool addChestDartTraps(int x, int y, Random &rnd, World &world)
{
    std::vector<Point> traps;
    for (auto delta : {Point{-1, 0}, {1, 0}}) {
        for (int j = -4; j < 2; ++j) {
            Point pos = scanWhileNotSolid({x, y + j}, delta, world);
            int dist = std::abs(pos.first - x);
            if (dist < 5 || dist > 30) {
                continue;
            }
            if (!isTrappable(world.getTile(pos)) ||
                !isSolidBlock(world.getTile(addPts(pos, {0, 1})).blockID)) {
                pos = addPts(pos, delta);
                if (!isTrappable(world.getTile(pos)) ||
                    !isSolidBlock(world.getTile(addPts(pos, {0, 1})).blockID)) {
                    continue;
                }
            }
            traps.push_back(pos);
        }
    }
    if (traps.empty()) {
        return false;
    }
    std::shuffle(traps.begin(), traps.end(), rnd.getPRNG());
    traps.resize(rnd.getInt(1, std::min<int>(traps.size(), 4)));
    for (auto trap : traps) {
        placeWire(trap, {x, y}, Wire::red, world);
        world.placeFramedTile(
            trap.first,
            trap.second,
            TileID::trap,
            trap.first > x ? Variant::dartLeft : Variant::dartRight);
    }
    return true;
}

bool addChestExplosiveTraps(int x, int y, Random &rnd, World &world)
{
    double numExplosives = rnd.getDouble(0, 3);
    bool didTrap = false;
    while (numExplosives > 0) {
        int i = rnd.getInt(-9, 9);
        int j = rnd.getInt(-7, 4);
        if (!world.regionPasses(x + i - 1, y + j - 1, 3, 3, [](Tile &tile) {
                return tile.blockID != TileID::empty && isTrappable(tile);
            })) {
            numExplosives -= 0.1;
            continue;
        }
        world.getTile(x + i, y + j).blockID = TileID::explosives;
        placeWire({x + i, y + j}, {x, y}, Wire::red, world);
        didTrap = true;
        --numExplosives;
    }
    return didTrap;
}

bool addChestLavaTrap(int x, int y, Random &rnd, World &world)
{
    if (y < std::midpoint(world.getCavernLevel(), world.getUnderworldLevel())) {
        return false;
    }
    Point pos = scanWhileNotSolid({x, y}, {0, -1}, world);
    int dist = y - pos.second;
    if (dist < 5 || dist > 30) {
        return false;
    }
    pos = addPts(pos, {0, -5});
    std::set<int> oreTiles{
        TileID::ironOre,
        TileID::copperOre,
        TileID::goldOre,
        TileID::silverOre,
        TileID::tinOre,
        TileID::leadOre,
        TileID::tungstenOre,
        TileID::platinumOre};
    if (!world.regionPasses(
            pos.first - 10,
            pos.second - 10,
            20,
            20,
            [&oreTiles](Tile &tile) {
                return !tile.guarded &&
                       (trappableTiles.contains(tile.blockID) ||
                        oreTiles.contains(tile.blockID));
            }) ||
        !world.regionPasses(
            pos.first - 9,
            pos.second - 18,
            18,
            14,
            [&oreTiles](Tile &tile) {
                return !tile.guarded && tile.blockID != TileID::empty &&
                       (trappableTiles.contains(tile.blockID) ||
                        oreTiles.contains(tile.blockID));
            })) {
        return false;
    }
    for (int i = -7; i < 7; ++i) {
        for (int j = -7; j < 7; ++j) {
            double threshold = 4 * std::hypot(i, j) / 7 - 3;
            if (rnd.getFineNoise(pos.first + i, pos.second + j - 11) >
                threshold) {
                Tile &tile = world.getTile(pos.first + i, pos.second + j - 11);
                tile.blockID = TileID::empty;
                tile.liquid = Liquid::lava;
                tile.guarded = true;
            }
        }
    }
    world.getTile(pos).blockID = TileID::explosives;
    placeWire(pos, {x, y}, Wire::red, world);
    return true;
}

void addChestTraps(int x, int y, Random &rnd, World &world)
{
    if (addChestLavaTrap(x, y, rnd, world)) {
        return;
    }
    std::array trapAdders{
        addChestBoulderTraps,
        addChestDartTraps,
        addChestExplosiveTraps};
    std::shuffle(trapAdders.begin(), trapAdders.end(), rnd.getPRNG());
    int numTypes = 0;
    for (const auto &trapAdder : trapAdders) {
        if (trapAdder(x, y, rnd, world)) {
            ++numTypes;
            if (numTypes == 2) {
                break;
            }
        }
    }
}
