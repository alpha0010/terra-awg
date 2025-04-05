#include "structures/Temple.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/Platforms.h"
#include <iostream>
#include <set>

typedef std::pair<int, int> Point;

template <typename Func> void iterateTemple(Point center, World &world, Func f)
{
    int scanX = 0.029 * world.getWidth();
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
    std::set<int> avoidBlocks{
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
    };
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
    int xMin = world.jungleCenter - 0.079 * world.getWidth();
    int xMax = world.jungleCenter + 0.079 * world.getWidth();
    int yMin = (world.getUndergroundLevel() + world.getCavernLevel()) / 2;
    while (true) {
        int x = rnd.getInt(xMin, xMax);
        int y = rnd.getInt(yMin, world.getUnderworldLevel());
        if (testTempleSelection({x, y}, world)) {
            return {x, y};
        }
    }
}

void clearTempleSurface(Point center, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    std::set<int> clearableTiles{
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
    int scanDist = 0.019 * world.getWidth();
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

void genTemple(Random &rnd, World &world)
{
    std::cout << "Training acolytes\n";
    Point center = selectTempleCenter(rnd, world);
    iterateTemple(center, world, [&world](int x, int y) {
        Tile &tile = world.getTile(x, y);
        tile.blockID = TileID::lihzahrdBrick;
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
}
