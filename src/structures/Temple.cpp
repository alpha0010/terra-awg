#include "structures/Temple.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>
#include <set>

template <typename Func> void iterateTemple(Point center, World &world, Func f)
{
    int scanX =
        std::max<int>(world.conf.templeSize * 0.029 * world.getWidth(), 128);
    int scanY = 0.45 * scanX;
    int startScanY = scanY;
    int stepCtrl = -1;
    for (int i = -scanX; i < scanX; ++i) {
        if (stepCtrl < 0) {
            if (stepCtrl < -3) {
                stepCtrl = i > -30 && i < -15 ? -i * 2 - 1 : 15;
            }
            if (i < 0) {
                startScanY -= 4;
            } else {
                startScanY += 4;
            }
        }
        --stepCtrl;
        int endScanY = scanY - (((i + scanX) / 3) % 2);
        for (int j = startScanY; j < endScanY; ++j) {
            if (!f(center.first + i, center.second + j)) {
                return;
            }
        }
    }
}

bool testTempleSelection(Point center, World &world)
{
    constexpr auto avoidBlocks = frozen::make_set<int>({
        TileID::aetherium,
        TileID::ash,
        TileID::blueBrick,
        TileID::corruptJungleGrass,
        TileID::crimsonJungleGrass,
        TileID::granite,
        TileID::greenBrick,
        TileID::hive,
        TileID::marble,
        TileID::mushroomGrass,
        TileID::pinkBrick,
    });
    bool isValid = true;
    iterateTemple(
        center,
        world,
        [&avoidBlocks, &isValid, &world](int x, int y) {
            if (avoidBlocks.contains(world.getTile(x, y).blockID)) {
                isValid = false;
                return false;
            }
            return true;
        });
    return isValid;
}

Point selectTempleCenter(Random &rnd, World &world)
{
    int minX = world.conf.patches
                   ? 350
                   : std::max<int>(
                         world.jungleCenter -
                             world.conf.jungleSize * 0.079 * world.getWidth(),
                         350);
    int maxX = world.conf.patches
                   ? world.getWidth() - 350
                   : std::min<int>(
                         world.jungleCenter +
                             world.conf.jungleSize * 0.079 * world.getWidth(),
                         world.getWidth() - 350);
    int minY = (world.getUndergroundLevel() + world.getCavernLevel()) / 2;
    for (int numTries = 0; numTries < 1000; ++numTries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, world.getUnderworldLevel());
        if ((!world.conf.patches ||
             isInBiome(x, y, 200 - 0.19 * numTries, Biome::jungle, world)) &&
            testTempleSelection({x, y}, world)) {
            return {x, y};
        }
    }
    return {
        world.jungleCenter,
        std::midpoint(minY, world.getUnderworldLevel())};
}

void clearTempleSurface(Point center, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::dirt,          TileID::mud,         TileID::jungleGrass,
         TileID::stone,         TileID::clay,        TileID::silt,
         TileID::copperOre,     TileID::tinOre,      TileID::ironOre,
         TileID::leadOre,       TileID::silverOre,   TileID::tungstenOre,
         TileID::goldOre,       TileID::platinumOre, TileID::cobaltOre,
         TileID::palladiumOre,  TileID::mythrilOre,  TileID::orichalcumOre,
         TileID::adamantiteOre, TileID::titaniumOre, TileID::chlorophyteOre});
    int scanDist =
        std::max<double>(world.conf.templeSize * 0.019 * world.getWidth(), 82);
    for (int x = center.first - scanDist; x < center.first + scanDist; ++x) {
        for (int y = center.second - scanDist; y < center.second + scanDist;
             ++y) {
            double threshold =
                3 * std::hypot(x - center.first, y - center.second) / scanDist -
                2;
            Tile &tile = world.getTile(x, y);
            if (rnd.getFineNoise(x, y) > threshold) {
                if (tile.blockID == TileID::lihzahrdBrick) {
                    break;
                } else if (clearableTiles.contains(tile.blockID)) {
                    tile.blockID = TileID::empty;
                    Tile &leftTile = world.getTile(x - 1, y);
                    if (leftTile.blockID == TileID::mud) {
                        leftTile.blockID = TileID::jungleGrass;
                    }
                }
            } else if (tile.blockID == TileID::mud && world.isExposed(x, y)) {
                tile.blockID = TileID::jungleGrass;
            }
        }
    }
}

Point avgPoints(Point a, Point b)
{
    return {(a.first + b.first) / 2, (a.second + b.second) / 2};
}

void applyRoomConnection(Point from, Point to, int roomSize, World &world)
{
    if (from.first == to.first) {
        int minY = std::min(from.second, to.second);
        int maxY = std::max(from.second, to.second);
        for (int x = from.first; x < from.first + roomSize; ++x) {
            for (int y = minY + roomSize; y < maxY; ++y) {
                world.getTile(x, y).blockID = TileID::empty;
            }
        }
    } else {
        int minX = std::min(from.first, to.first);
        int maxX = std::max(from.first, to.first);
        for (int x = minX + roomSize; x < maxX; ++x) {
            for (int y = from.second; y < from.second + roomSize; ++y) {
                world.getTile(x, y).blockID = TileID::empty;
            }
        }
    }
}

void addTempleEntry(int centerX, int minY, World &world)
{
    int doorOffset = 29;
    int x = centerX;
    while (world.getTile(x - 1, minY + doorOffset).wallID ==
           WallID::Unsafe::lihzahrdBrick) {
        --x;
    }
    world
        .placeFramedTile(x, minY + doorOffset, TileID::door, Variant::lihzahrd);
    world.placeFramedTile(
        x - 2,
        minY + doorOffset,
        TileID::lamp,
        Variant::lihzahrd);
    world.placeFramedTile(
        x + 2,
        minY + doorOffset + 1,
        TileID::pot,
        Variant::lihzahrd);
    int clearedGap = 0;
    for (++x; x < centerX; ++x) {
        for (int i = 0; i < 3; ++i) {
            Tile &tile = world.getTile(x, minY + doorOffset + i);
            if (tile.blockID == TileID::lihzahrdBrick) {
                tile.blockID = TileID::empty;
            }
        }
        if (world.getTile(x, minY + doorOffset - 1).blockID == TileID::empty ||
            world.getTile(x, minY + doorOffset + 3).blockID == TileID::empty) {
            ++clearedGap;
            if (clearedGap > 1) {
                break;
            }
        }
    }
    x = centerX;
    while (world.getTile(x + 1, minY + doorOffset).wallID ==
           WallID::Unsafe::lihzahrdBrick) {
        ++x;
    }
    world
        .placeFramedTile(x, minY + doorOffset, TileID::door, Variant::lihzahrd);
    world.placeFramedTile(
        x + 2,
        minY + doorOffset,
        TileID::lamp,
        Variant::lihzahrd);
    world.placeFramedTile(
        x - 3,
        minY + doorOffset + 1,
        TileID::pot,
        Variant::lihzahrd);
    clearedGap = 0;
    for (--x; x > centerX; --x) {
        for (int i = 0; i < 3; ++i) {
            Tile &tile = world.getTile(x, minY + doorOffset + i);
            if (tile.blockID == TileID::lihzahrdBrick) {
                tile.blockID = TileID::empty;
            }
        }
        if (world.getTile(x, minY + doorOffset - 1).blockID == TileID::empty ||
            world.getTile(x, minY + doorOffset + 3).blockID == TileID::empty) {
            ++clearedGap;
            if (clearedGap > 1) {
                break;
            }
        }
    }
}

void addDeadEndPlatforms(
    int roomSize,
    const std::set<Point> &rooms,
    World &world)
{
    for (auto [x, y] : rooms) {
        bool isVertDeadEnd = true;
        for (auto [i, j] :
             {std::pair{-1, 0}, {0, -1}, {roomSize - 1, -1}, {roomSize, 0}}) {
            if (world.getTile(x + i, y + j).blockID != TileID::lihzahrdBrick) {
                isVertDeadEnd = false;
                break;
            }
        }
        if (!isVertDeadEnd) {
            continue;
        }
        for (int i = 0; i < roomSize; ++i) {
            placePlatform(x + i, y + roomSize - 1, Platform::lihzahrd, world);
        }
    }
}

bool canPlaceTempleTreasureAt(int x, int y, World &world)
{
    constexpr auto baseTiles =
        frozen::make_set<int>({TileID::lihzahrdBrick, TileID::platform});
    return baseTiles.contains(world.getTile(x, y).blockID) &&
           baseTiles.contains(world.getTile(x + 1, y).blockID) &&
           world.regionPasses(x, y - 4, 2, 4, [](Tile &tile) {
               return tile.blockID == TileID::empty &&
                      tile.wallID == WallID::Unsafe::lihzahrdBrick;
           });
}

std::vector<Point>
addTempleTreasures(Point center, int numRooms, Random &rnd, World &world)
{
    std::vector<Point> locations;
    iterateTemple(center, world, [&](int x, int y) {
        if (canPlaceTempleTreasureAt(x, y, world)) {
            locations.emplace_back(x, y);
            if (world.getTile(x, y).blockID == TileID::platform) {
                for (int i = 0; i < 14; ++i) {
                    locations.emplace_back(x, y);
                }
            }
        }
        return true;
    });
    int numChests = world.conf.chests * numRooms / 17.5;
    std::vector<Point> usedLocations;
    while (numChests > 0) {
        auto [x, y] = rnd.select(locations);
        if (!canPlaceTempleTreasureAt(x, y, world) ||
            isLocationUsed(x, y, 12, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        Chest &chest = world.placeChest(x, y - 2, Variant::lihzahrd);
        fillLihzahrdChest(chest, rnd, world);
        --numChests;
    }
    int numPots = numRooms / 14;
    while (numPots > 0) {
        auto [x, y] = rnd.select(locations);
        if (!canPlaceTempleTreasureAt(x, y, world)) {
            continue;
        }
        world.placeFramedTile(x, y - 2, TileID::pot, Variant::lihzahrd);
        --numPots;
    }
    int numStatues = numRooms / 12;
    while (numStatues > 0) {
        auto [x, y] = rnd.select(locations);
        if (!canPlaceTempleTreasureAt(x - 1, y, world) ||
            !canPlaceTempleTreasureAt(x + 1, y, world)) {
            continue;
        }
        world.placeFramedTile(x, y - 3, TileID::statue, Variant::lihzahrd);
        --numStatues;
    }
    return locations;
}

void addWallTrap(
    Point pos,
    Variant trapLeft,
    Variant trapRight,
    Random &rnd,
    World &world)
{
    if (!canPlaceTempleTreasureAt(pos.first, pos.second, world)) {
        return;
    }
    auto [x, y] = pos;
    --y;
    std::vector<Point> traps;
    for (int j = 0; j < 4; ++j) {
        for (int dir : {-1, 1}) {
            Point trap = scanWhileNotSolid({x, y - j}, {dir, 0}, world);
            double dist = std::hypot(x - trap.first, y - trap.second);
            if (dist > 1.5 && dist < 20 &&
                world.getTile(trap.first + dir, trap.second).blockID ==
                    TileID::lihzahrdBrick) {
                traps.push_back({trap.first + dir, trap.second});
            }
        }
    }
    if (traps.empty()) {
        return;
    }
    std::shuffle(traps.begin(), traps.end(), rnd.getPRNG());
    traps.resize(rnd.getInt(1, std::min<int>(traps.size(), 3)));
    for (auto trap : traps) {
        placeWire(trap, {x, y}, static_cast<Wire>((x + y) % 4), world);
        world.placeFramedTile(
            trap.first,
            trap.second,
            TileID::trap,
            trap.first > x ? trapLeft : trapRight);
    }
    world.placeFramedTile(x, y, TileID::pressurePlate, Variant::lihzahrd);
}

void addCeilingTrap(Point pos, Variant trapType, World &world)
{
    if (!canPlaceTempleTreasureAt(pos.first, pos.second, world)) {
        return;
    }
    auto [x, y] = pos;
    --y;
    int trapCeiling = scanWhileEmpty({x, y}, {0, -1}, world).second - 1;
    if (world.getTile(x, trapCeiling).blockID != TileID::lihzahrdBrick ||
        (trapType == Variant::spear && y - trapCeiling > 18)) {
        return;
    }
    for (int i = -3; i < 4; ++i) {
        if (world.getTile(x + i, trapCeiling).blockID ==
                TileID::lihzahrdBrick &&
            world.getTile(x + i, trapCeiling + 1).blockID == TileID::empty) {
            placeWire(
                {x + i, trapCeiling},
                {x, y},
                static_cast<Wire>((x + y) % 4),
                world);
            world.placeFramedTile(x + i, trapCeiling, TileID::trap, trapType);
        }
    }
    world.placeFramedTile(x, y, TileID::pressurePlate, Variant::lihzahrd);
}

void addTraps(std::vector<Point> locations, Random &rnd, World &world)
{
    for (auto [x, y] : locations) {
        switch (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 75) {
        case 0:
            if (canPlaceTempleTreasureAt(x, y, world)) {
                world.placeFramedTile(x, y - 2, TileID::TNTBarrel);
            }
            break;
        case 1:
        case 2:
            addWallTrap(
                {x, y},
                Variant::flameLeft,
                Variant::flameRight,
                rnd,
                world);
            break;
        case 3:
        case 4:
            addWallTrap(
                {x, y},
                Variant::superDartLeft,
                Variant::superDartRight,
                rnd,
                world);
            break;
        case 5:
        case 6:
            addCeilingTrap({x, y}, Variant::spear, world);
            break;
        case 7:
        case 8:
            addCeilingTrap({x, y}, Variant::spikyBall, world);
            break;
        }
    }
}

void addSpikesAt(Point pos, Random &rnd, World &world)
{
    if (static_cast<int>(
            99999 * (1 + rnd.getFineNoise(pos.first, pos.second))) %
            150 !=
        0) {
        return;
    }
    Point delta = rnd.select({std::pair{1, 0}, {-1, 0}, {0, 1}, {0, -1}});
    Point wall = scanWhileNotSolid(pos, delta, world);
    Point incr = delta.first == 0 ? std::pair{1, 0} : std::pair{0, 1};
    for (int t = 0; t < 4; ++t) {
        wall = subPts(wall, incr);
    }
    for (int t = 0; t < 9; ++t, wall = addPts(wall, incr)) {
        if (world.getTile(wall).blockID != TileID::empty ||
            world.getTile(addPts(wall, delta)).blockID !=
                TileID::lihzahrdBrick) {
            continue;
        }
        world.getTile(wall).blockID = TileID::woodenSpike;
        if ((wall.first + wall.second) % 2 == 0) {
            world.getTile(addPts(wall, delta)).blockID = TileID::woodenSpike;
        } else {
            Tile &tile = world.getTile(subPts(wall, delta));
            if (tile.blockID == TileID::empty) {
                tile.blockID = TileID::woodenSpike;
            }
        }
    }
}

void addSpikes(Point center, Random &rnd, World &world)
{
    iterateTemple(center, world, [&](int x, int y) {
        if (world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            addSpikesAt({x, y}, rnd, world);
        }
        return true;
    });
}

void paintTemple(Point center, World &world)
{
    iterateTemple(center, world, [&world](int x, int y) {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::lihzahrdBrick ||
            tile.blockID == TileID::platform ||
            tile.blockID == TileID::pressurePlate ||
            tile.blockID == TileID::trap) {
            tile.blockPaint = Paint::deepLime;
        }
        if (tile.wallID == WallID::Unsafe::lihzahrdBrick) {
            tile.wallPaint = Paint::deepLime;
        }
        return true;
    });
}

void genTemple(Random &rnd, World &world)
{
    std::cout << "Training acolytes\n";
    Point center = selectTempleCenter(rnd, world);
    if (center.first < 100) {
        return;
    }
    iterateTemple(center, world, [&world](int x, int y) {
        Tile &tile = world.getTile(x, y);
        tile.blockID = TileID::lihzahrdBrick;
        tile.blockPaint = Paint::none;
        tile.wallID = WallID::Unsafe::lihzahrdBrick;
        return true;
    });
    clearTempleSurface(center, rnd, world);
    int wallThickness = 4;
    int roomSize = 7;
    int roomStep = roomSize + wallThickness;
    std::set<Point> rooms;
    int modTargetX = center.first % roomStep;
    int modTargetY = center.second % roomStep;
    int centerRoomX = 0;
    int maxRoomY = 0;
    int minY = center.second;
    iterateTemple(center, world, [&](int x, int y) {
        minY = std::min(y, minY);
        if (x % roomStep == modTargetX && y % roomStep == modTargetY &&
            world.regionPasses(
                x - wallThickness,
                y - wallThickness,
                roomStep + wallThickness,
                roomStep + wallThickness,
                [](Tile &tile) {
                    return tile.wallID == WallID::Unsafe::lihzahrdBrick;
                })) {
            if (std::abs(x - center.first) <
                std::abs(centerRoomX - center.first)) {
                centerRoomX = x;
            }
            maxRoomY = std::max(y, maxRoomY);
            rooms.emplace(x, y);
        }
        return true;
    });
    for (int i = -2; i < 3; ++i) {
        for (int j = -2; j < 1; ++j) {
            rooms.erase({centerRoomX + i * roomStep, maxRoomY + j * roomStep});
        }
    }
    rooms.emplace(
        rnd.select({centerRoomX - 2 * roomStep, centerRoomX + 2 * roomStep}),
        maxRoomY);
    Point agent = *rooms.begin();
    std::set<Point> connectedRooms{agent};
    std::set<Point> connections;
    std::vector<Point> junctions;
    while (connectedRooms.size() < rooms.size()) {
        std::vector<Point> choices;
        for (auto [i, j] :
             {std::pair{-roomStep, 0},
              {roomStep, 0},
              {0, -roomStep},
              {0, roomStep}}) {
            Point candidate = {agent.first + i, agent.second + j};
            if (rooms.contains(candidate) &&
                (!connectedRooms.contains(candidate) ||
                 connections.contains(avgPoints(agent, candidate)))) {
                choices.push_back(candidate);
            }
        }
        if (choices.size() == 1 && connectedRooms.contains(choices[0])) {
            agent = junctions.back();
            junctions.pop_back();
        } else {
            for (Point choice : choices) {
                if (!connectedRooms.contains(choice)) {
                    junctions.push_back(agent);
                    break;
                }
            }
            Point next = rnd.select(choices);
            if (!connectedRooms.contains(next)) {
                connections.insert(avgPoints(agent, next));
                connectedRooms.insert(next);
                applyRoomConnection(agent, next, roomSize, world);
            }
            agent = next;
        }
    }
    for (auto [x, y] : rooms) {
        for (int i = 0; i < roomSize; ++i) {
            for (int j = 0; j < roomSize; ++j) {
                world.getTile(x + i, y + j).blockID = TileID::empty;
            }
        }
    }
    for (int x = centerRoomX - 2 * roomStep;
         x < centerRoomX + 2 * roomStep + roomSize;
         ++x) {
        for (int y = maxRoomY - 2 * roomStep; y < maxRoomY + roomSize; ++y) {
            world.getTile(x, y).blockID = TileID::empty;
        }
    }
    addTempleEntry(center.first, minY, world);
    addDeadEndPlatforms(roomSize, rooms, world);
    world.placeFramedTile(
        centerRoomX + roomSize / 2 - 1,
        maxRoomY + roomSize - 2,
        TileID::lihzahrdAltar);
    std::vector<Point> flatLocations =
        addTempleTreasures(center, rooms.size(), rnd, world);
    std::erase_if(flatLocations, [&world](Point &pt) {
        return world.getTile(pt).blockID == TileID::platform;
    });
    std::shuffle(flatLocations.begin(), flatLocations.end(), rnd.getPRNG());
    addTraps(flatLocations, rnd, world);
    addSpikes(center, rnd, world);
    if (world.conf.doubleTrouble) {
        paintTemple(center, world);
    }
}
