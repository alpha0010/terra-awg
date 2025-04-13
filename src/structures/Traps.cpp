#include "structures/Traps.h"

#include "Random.h"
#include "World.h"
#include "structures/StructureUtil.h"
#include <iostream>
#include <set>

void placeWire(Point from, Point to, World &world)
{
    world.getTile(from.first, from.second).wireRed = true;
    while (from != to) {
        if (from.first < to.first) {
            ++from.first;
        } else if (from.first > to.first) {
            --from.first;
        }
        world.getTile(from.first, from.second).wireRed = true;
        if (from.second < to.second) {
            ++from.second;
        } else if (from.second > to.second) {
            --from.second;
        }
        world.getTile(from.first, from.second).wireRed = true;
    }
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
        world.getWidth() * world.getHeight() / rnd.getInt(384000, 480000);
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
        placeWire({x, trapFloor}, {x, y - 1}, world);
        Point prevActuator{-1, -1};
        for (int i = -5; i < 5; ++i) {
            int trapCeiling =
                scanWhileEmpty({x + i, (y + trapFloor) / 2}, {0, -1}, world)
                    .second -
                1;
            Tile &tile = world.getTile(x + i, trapCeiling);
            if (looseBlocks.contains(tile.blockID)) {
                tile.actuator = true;
                if (prevActuator.first != -1) {
                    placeWire(prevActuator, {x + i, trapCeiling}, world);
                }
                prevActuator = {x + i, trapCeiling};
            }
        }
        --numSandTraps;
    }
}

bool isValidBoulderPlacement(int x, int y, World &world)
{
    return world.regionPasses(x, y, 6, 6, [](Tile &tile) {
        return !tile.guarded && tile.blockID == TileID::stone;
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
    std::set<int> allowedTiles{
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
        TileID::kryptonMossStone,
        TileID::xenonMossStone,
        TileID::argonMossStone,
        TileID::neonMossStone,
        TileID::corruptJungleGrass,
        TileID::crimsonJungleGrass};
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
                [&allowedTiles](Tile &tile) {
                    return !tile.guarded && tile.liquid == Liquid::none &&
                           allowedTiles.contains(tile.blockID);
                })) {
            continue;
        }
        world.placeFramedTile(x, y, TileID::boulder);
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
        --numBoulders;
    }
}

void genTraps(Random &rnd, World &world)
{
    std::cout << "Arming traps\n";
    placeSandTraps(rnd, world);
    placeBoulderTraps(rnd, world);
}
