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
            if (!f(center.x + i, center.y + j)) {
                return;
            }
        }
    }
}

bool testTempleSelection(Point center, World &world)
{
    std::set<int> avoidBlocks{
        TileID::aetherium,
        TileID::ash,
        TileID::blueBrick,
        TileID::granite,
        TileID::greenBrick,
        TileID::hive,
        TileID::livingMahogany,
        TileID::livingWood,
        TileID::marble,
        TileID::mushroomGrass,
        TileID::pinkBrick,
    };
    if (!world.conf.dontDigUp) {
        avoidBlocks.insert(
            {TileID::corruptJungleGrass, TileID::crimsonJungleGrass});
    }
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

Point selectTempleCenter(
    std::function<bool(Point, World &)> isValid,
    Random &rnd,
    World &world)
{
    int minX = world.conf.biomes == BiomeLayout::columns
                   ? std::max<int>(
                         world.jungleCenter -
                             world.conf.jungleSize * 0.079 * world.getWidth(),
                         350)
                   : 350;
    int maxX = world.conf.biomes == BiomeLayout::columns
                   ? std::min<int>(
                         world.jungleCenter +
                             world.conf.jungleSize * 0.079 * world.getWidth(),
                         world.getWidth() - 350)
                   : world.getWidth() - 350;
    if (maxX < minX) {
        minX = std::midpoint(minX, maxX);
        maxX = minX + 1;
    }
    int minY = world.conf.dontDigUp
                   ? world.getSurfaceLevel(world.jungleCenter) +
                         std::max(
                             world.conf.templeSize * 0.009 * world.getWidth(),
                             40.0) -
                         20
                   : std::midpoint(
                         world.getUndergroundLevel(),
                         world.getCavernLevel());
    int maxY = world.conf.dontDigUp ? minY + 30 : world.getUnderworldLevel();
    for (int numTries = 0; numTries < 1000; ++numTries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        if ((world.conf.biomes == BiomeLayout::columns ||
             isInBiome(x, y, 200 - 0.19 * numTries, Biome::jungle, world)) &&
            isValid({x, y}, world)) {
            return {x, y};
        }
    }
    return {world.jungleCenter, std::midpoint(minY, maxY)};
}

void clearTempleSurface(Point center, int grassTile, Random &rnd, World &world)
{
    clearTempleSurface(
        center,
        std::max<double>(world.conf.templeSize * 0.019 * world.getWidth(), 82),
        grassTile,
        rnd,
        world);
}

void clearTempleSurface(
    Point center,
    int scanDist,
    int grassTile,
    Random &rnd,
    World &world)
{
    rnd.shuffleNoise();
    constexpr auto clearableTiles = frozen::make_set<int>(
        {TileID::dirt,          TileID::mud,         TileID::jungleGrass,
         TileID::ebonstone,     TileID::demonite,    TileID::corruptJungleGrass,
         TileID::crimstone,     TileID::crimtane,    TileID::crimsonJungleGrass,
         TileID::stone,         TileID::clay,        TileID::silt,
         TileID::copperOre,     TileID::tinOre,      TileID::ironOre,
         TileID::leadOre,       TileID::silverOre,   TileID::tungstenOre,
         TileID::goldOre,       TileID::platinumOre, TileID::cobaltOre,
         TileID::palladiumOre,  TileID::mythrilOre,  TileID::orichalcumOre,
         TileID::adamantiteOre, TileID::titaniumOre, TileID::chlorophyteOre});
    for (int x = center.x - scanDist; x < center.x + scanDist; ++x) {
        for (int y = center.y - scanDist; y < center.y + scanDist; ++y) {
            double threshold = 3 * hypot(center, {x, y}) / scanDist - 2;
            Tile &tile = world.getTile(x, y);
            if (rnd.getFineNoise(x, y) > threshold) {
                if (tile.blockID == TileID::lihzahrdBrick) {
                    break;
                } else if (clearableTiles.contains(tile.blockID)) {
                    tile.blockID = TileID::empty;
                    Tile &leftTile = world.getTile(x - 1, y);
                    if (leftTile.blockID == TileID::mud) {
                        leftTile.blockID = grassTile;
                    }
                }
            } else if (tile.blockID == TileID::mud && world.isExposed(x, y)) {
                tile.blockID = grassTile;
            }
        }
    }
}

void applyRoomConnection(Point from, Point to, int roomSize, World &world)
{
    if (from.x == to.x) {
        int minY = std::min(from.y, to.y);
        int maxY = std::max(from.y, to.y);
        for (int x = from.x; x < from.x + roomSize; ++x) {
            for (int y = minY + roomSize; y < maxY; ++y) {
                world.getTile(x, y).blockID = TileID::empty;
            }
        }
    } else {
        int minX = std::min(from.x, to.x);
        int maxX = std::max(from.x, to.x);
        for (int x = minX + roomSize; x < maxX; ++x) {
            for (int y = from.y; y < from.y + roomSize; ++y) {
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
    addTempleTreasures(locations, numRooms, rnd, world);
    return locations;
}

void addTempleTreasures(
    std::vector<Point> &locations,
    int numRooms,
    Random &rnd,
    World &world)
{
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
}

void addWallTrap(
    Point pos,
    Variant trapLeft,
    Variant trapRight,
    Random &rnd,
    World &world)
{
    if (!canPlaceTempleTreasureAt(pos.x, pos.y, world)) {
        return;
    }
    auto [x, y] = pos;
    --y;
    std::vector<Point> traps;
    for (int j = 0; j < 4; ++j) {
        for (int dir : {-1, 1}) {
            Point trap = scanWhileNotSolid({x, y - j}, {dir, 0}, world);
            double dist = hypot(trap, {x, y});
            if (dist > 1.5 && dist < 20 &&
                world.getTile(trap.x + dir, trap.y).blockID ==
                    TileID::lihzahrdBrick) {
                traps.push_back({trap.x + dir, trap.y});
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
            trap.x,
            trap.y,
            TileID::trap,
            trap.x > x ? trapLeft : trapRight);
    }
    world.placeFramedTile(x, y, TileID::pressurePlate, Variant::lihzahrd);
}

void addCeilingTrap(Point pos, Variant trapType, World &world)
{
    if (!canPlaceTempleTreasureAt(pos.x, pos.y, world)) {
        return;
    }
    auto [x, y] = pos;
    --y;
    int trapCeiling = scanWhileEmpty({x, y}, {0, -1}, world).y - 1;
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

void addTempleTraps(
    std::vector<Point> &locations,
    int freqCtrl,
    Random &rnd,
    World &world)
{
    for (auto [x, y] : locations) {
        switch (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                freqCtrl) {
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

void addTempleSpikesAt(Point pos, Random &rnd, World &world)
{
    if (static_cast<int>(99999 * (1 + rnd.getFineNoise(pos.x, pos.y))) % 150 !=
        0) {
        return;
    }
    Point delta = rnd.select({Point{1, 0}, {-1, 0}, {0, 1}, {0, -1}});
    Point wall = scanWhileNotSolid(pos, delta, world);
    Point incr = delta.x == 0 ? Point{1, 0} : Point{0, 1};
    for (int t = 0; t < 4; ++t) {
        wall -= incr;
    }
    for (int t = 0; t < 9; ++t, wall += incr) {
        if (world.getTile(wall).blockID != TileID::empty ||
            world.getTile(wall + delta).blockID != TileID::lihzahrdBrick) {
            continue;
        }
        world.getTile(wall).blockID = TileID::woodenSpike;
        if ((wall.x + wall.y) % 2 == 0) {
            world.getTile(wall + delta).blockID = TileID::woodenSpike;
        } else {
            Tile &tile = world.getTile(wall - delta);
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
            addTempleSpikesAt({x, y}, rnd, world);
        }
        return true;
    });
}

void paintTemple(Point center, int blockPaint, int wallPaint, World &world)
{
    iterateTemple(center, world, [blockPaint, wallPaint, &world](int x, int y) {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::lihzahrdBrick ||
            tile.blockID == TileID::platform ||
            tile.blockID == TileID::pressurePlate ||
            tile.blockID == TileID::trap) {
            tile.blockPaint = blockPaint;
        }
        if (tile.wallID == WallID::Unsafe::lihzahrdBrick) {
            tile.wallPaint = wallPaint;
        }
        return true;
    });
}

void genTemple(Random &rnd, World &world)
{
    if (world.conf.templeSize < 0.01) {
        return;
    }
    std::cout << "Training acolytes\n";
    Point center = selectTempleCenter(testTempleSelection, rnd, world);
    if (center.x < 100) {
        return;
    }
    int numCorrupt = 0;
    int numCrimson = 0;
    iterateTemple(
        center,
        world,
        [&numCorrupt, &numCrimson, &world](int x, int y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::corruptJungleGrass) {
                ++numCorrupt;
            } else if (tile.blockID == TileID::crimsonJungleGrass) {
                ++numCrimson;
            }
            tile.blockID = TileID::lihzahrdBrick;
            tile.blockPaint = Paint::none;
            tile.wallID = WallID::Unsafe::lihzahrdBrick;
            return true;
        });
    clearTempleSurface(
        center,
        numCrimson > numCorrupt ? TileID::crimsonJungleGrass
        : numCorrupt == 0       ? TileID::jungleGrass
                                : TileID::corruptJungleGrass,
        rnd,
        world);
    int wallThickness = 4;
    int roomSize = 7;
    int roomStep = roomSize + wallThickness;
    std::set<Point> rooms;
    int modTargetX = center.x % roomStep;
    int modTargetY = center.y % roomStep;
    int centerRoomX = 0;
    int maxRoomY = 0;
    int minY = center.y;
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
            if (std::abs(x - center.x) < std::abs(centerRoomX - center.x)) {
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
        for (Point step :
             {Point{-roomStep, 0},
              {roomStep, 0},
              {0, -roomStep},
              {0, roomStep}}) {
            Point candidate = agent + step;
            if (rooms.contains(candidate) &&
                (!connectedRooms.contains(candidate) ||
                 connections.contains(agent + candidate))) {
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
                connections.insert(agent + next);
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
    addTempleEntry(center.x, minY, world);
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
    addTempleTraps(flatLocations, 75, rnd, world);
    addSpikes(center, rnd, world);
    if (!world.conf.unpainted) {
        if (world.conf.forTheWorthy) {
            paintTemple(center, Paint::deepGreen, Paint::deepGreen, world);
        } else if (world.conf.dontDigUp) {
            paintTemple(
                center,
                numCrimson > numCorrupt ? Paint::deepRed : Paint::deepPurple,
                Paint::black,
                world);
        } else if (world.conf.celebration) {
            paintTemple(center, Paint::purple, Paint::cyan, world);
        } else if (world.conf.doubleTrouble) {
            paintTemple(center, Paint::deepLime, Paint::deepLime, world);
        }
    }
}
