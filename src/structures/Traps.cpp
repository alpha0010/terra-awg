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

void genTraps(Random &rnd, World &world)
{
    std::cout << "Arming traps\n";
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
