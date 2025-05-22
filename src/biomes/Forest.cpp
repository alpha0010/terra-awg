#include "Forest.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/Rooms.h"
#include "structures/data/SwordShrines.h"
#include <algorithm>
#include <iostream>
#include <numbers>
#include <set>

typedef std::pair<double, double> Pointf;

void drawRect(
    Pointf topLeft,
    Pointf bottomRight,
    double skewX,
    double skewY,
    World &world)
{
    int width = bottomRight.first - topLeft.first;
    int height = bottomRight.second - topLeft.second;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            Tile &tile = world.getTile(
                topLeft.first + i + skewX * j,
                topLeft.second + j + skewY * i);
            tile.blockID = TileID::livingWood;
            tile.wallID = WallID::Unsafe::livingWood;
        }
    }
}

void drawLine(Pointf from, Pointf to, double width, World &world)
{
    double deltaX = std::abs(to.first - from.first);
    double deltaY = std::abs(to.second - from.second);
    if (deltaY > deltaX) {
        if (from.second > to.second) {
            std::swap(from, to);
        }
        drawRect(
            {std::floor(from.first - width), std::floor(from.second)},
            {std::ceil(from.first + width), std::ceil(to.second)},
            (to.first - from.first) / deltaY,
            0,
            world);
    } else {
        if (from.first > to.first) {
            std::swap(from, to);
        }
        drawRect(
            {std::floor(from.first), std::floor(from.second - width)},
            {std::ceil(to.first), std::ceil(from.second + width)},
            0,
            (to.second - from.second) / deltaX,
            world);
    }
}

void growLeaves(Pointf from, Pointf to, double leafSpan, World &world)
{
    int minX = std::floor(std::min(from.first, to.first) - leafSpan);
    int maxX = std::ceil(std::max(from.first, to.first) + leafSpan);
    int minY = std::floor(std::min(from.second, to.second) - leafSpan);
    int maxY = std::ceil(std::max(from.second, to.second) + leafSpan);
    for (int x = minX; x < maxX; ++x) {
        for (int y = minY; y < maxY; ++y) {
            if (std::hypot(x - from.first, y - from.second) +
                    std::hypot(x - to.first, y - to.second) <
                2 * leafSpan) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::empty) {
                    tile.blockID = TileID::leaf;
                    tile.wallID = WallID::Safe::livingLeaf;
                }
            }
        }
    }
}

void growBranch(
    Pointf from,
    double weight,
    double stretch,
    double angle,
    Random &rnd,
    World &world)
{
    if (weight < 0.5) {
        return;
    }
    angle = (3 * angle - std::numbers::pi / 2) / 4;
    Pointf to{
        from.first + stretch * weight * std::cos(angle),
        from.second + stretch * weight * std::sin(angle)};
    drawLine(from, to, weight / 2, world);
    if (weight < 2.3) {
        growLeaves(from, to, std::max(5.5, 4 * weight), world);
    }
    double threshold = rnd.getDouble(0, 1);
    if (threshold < 0.45) {
        growBranch(
            to,
            rnd.getDouble(0.75, 0.88) * weight,
            stretch,
            angle + rnd.getDouble(-std::numbers::pi / 8, std::numbers::pi / 8),
            rnd,
            world);
        if (threshold < 0.32) {
            growBranch(
                to,
                rnd.getDouble(0.3, 0.5) * weight,
                stretch,
                angle + (threshold < 0.16 ? std::numbers::pi / 2
                                          : -std::numbers::pi / 2),
                rnd,
                world);
        }
    } else {
        double branchDistr = rnd.getDouble(0.55, 0.75);
        growBranch(
            to,
            branchDistr * weight,
            stretch,
            angle + rnd.getDouble(-std::numbers::pi / 2, -std::numbers::pi / 4),
            rnd,
            world);
        growBranch(
            to,
            (1.3 - branchDistr) * weight,
            stretch,
            angle + rnd.getDouble(std::numbers::pi / 4, std::numbers::pi / 2),
            rnd,
            world);
    }
}

void growRoot(
    Pointf from,
    double weight,
    double angle,
    Random &rnd,
    World &world)
{
    if (weight < 0.5) {
        return;
    }
    angle = (4 * angle + std::numbers::pi / 2) / 5;
    Pointf to{
        from.first + 1.8 * weight * std::cos(angle),
        from.second + 1.8 * weight * std::sin(angle)};
    drawLine(from, to, weight / 2, world);
    if (rnd.getDouble(0, 1) < 0.6) {
        growRoot(
            to,
            rnd.getDouble(0.9, 0.99) * weight,
            angle + rnd.getDouble(-std::numbers::pi / 8, std::numbers::pi / 8),
            rnd,
            world);
    } else {
        double rootDistr = rnd.getDouble(0.5, 0.8);
        growRoot(
            to,
            rootDistr * weight,
            angle + rnd.getDouble(-std::numbers::pi / 2, -std::numbers::pi / 4),
            rnd,
            world);
        growRoot(
            to,
            (1.3 - rootDistr) * weight,
            angle + rnd.getDouble(std::numbers::pi / 4, std::numbers::pi / 2),
            rnd,
            world);
    }
}

Point findDoor(TileBuffer &room)
{
    for (int i = 0; i < room.getWidth(); ++i) {
        for (int j = 0; j < room.getHeight(); ++j) {
            if (room.getTile(i, j).blockID == TileID::door) {
                return {i, j};
            }
        }
    }
    return {0, 0};
}

void growTapRoot(double x, double y, int roomId, Random &rnd, World &world)
{
    if (world.regionPasses(x - 3, y - 5, 6, 5, [](Tile &tile) {
            return tile.blockID == TileID::livingWood;
        })) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (j == 0 && (i == 0 || i == 3)) {
                    continue;
                }
                Tile &tile = world.getTile(x - 2 + i, y - 5 + j);
                tile.blockID = TileID::empty;
            }
        }
    }
    int tunnelDepth = 150;
    while (world.getTile(x, y + tunnelDepth).blockID != TileID::empty) {
        ++tunnelDepth;
    }
    int anchorX;
    int anchorY;
    for (int numTries = 0; numTries < 100; ++numTries) {
        anchorY = y + tunnelDepth * rnd.getDouble(0.4, 0.7);
        anchorX = x + 2.8 * rnd.getFineNoise(x, anchorY - y);
        int scanX = numTries < 50 ? 12 : 8;
        if (world.regionPasses(
                anchorX - scanX,
                anchorY,
                2 * scanX,
                numTries < 50 ? 8 : 6,
                [](Tile &tile) { return tile.blockID != TileID::empty; })) {
            break;
        }
    }
    for (int j = 0; j < tunnelDepth; ++j) {
        int iMin = 2 * rnd.getFineNoise(x + 100, j) - 4;
        int iMax = 2 * rnd.getFineNoise(x + 200, j) + 5;
        for (int i = iMin; i < iMax; ++i) {
            Tile &tile =
                world.getTile(x + i + 2.8 * rnd.getFineNoise(x, j), y + j);
            if (tile.blockID == TileID::livingWood ||
                tile.blockID == TileID::empty) {
                continue;
            }
            tile.wallID = WallID::Unsafe::livingWood;
            if (i < iMin + 2 || i > iMax - 3) {
                tile.blockID = tile.blockID == TileID::grass ||
                                       tile.blockID == TileID::jungleGrass
                                   ? TileID::leaf
                                   : TileID::livingWood;
            } else {
                tile.blockID = TileID::empty;
            }
        }
    }
    world.queuedTreasures.emplace_back([anchorX,
                                        anchorY,
                                        roomId](Random &rnd, World &world) {
        if (world.getTile(anchorX, anchorY).wallID !=
            WallID::Unsafe::livingWood) {
            return;
        }
        TileBuffer room = Data::getRoom(roomId, world.getFramedTiles());
        auto [doorI, doorJ] = findDoor(room);
        bool placeOnRight = doorI < room.getWidth() / 2;
        int x = anchorX;
        while (world.getTile(x, anchorY).wallID == WallID::Unsafe::livingWood) {
            if (placeOnRight) {
                ++x;
            } else {
                --x;
            }
        }
        if (!placeOnRight) {
            x -= room.getWidth();
        }
        std::set<int> clearableTiles{
            TileID::dirt,
            TileID::grass,
            TileID::stone,
            TileID::livingWood,
            TileID::leaf,
            TileID::corruptGrass,
            TileID::corruptJungleGrass,
            TileID::crimsonGrass,
            TileID::crimsonJungleGrass,
            TileID::mud,
            TileID::jungleGrass,
            TileID::clay,
            TileID::sand,
            TileID::sandstone,
            TileID::hardenedSand,
            TileID::snow,
            TileID::ice,
            TileID::copperOre,
            TileID::tinOre,
            TileID::ironOre,
            TileID::leadOre};
        if (!world.regionPasses(
                x,
                anchorY,
                room.getWidth(),
                room.getHeight(),
                [&clearableTiles](Tile &tile) {
                    return !tile.guarded &&
                           clearableTiles.contains(tile.blockID);
                })) {
            return;
        }
        for (int hallX = std::min(x + doorI, anchorX);
             hallX < std::max(x + doorI, anchorX);
             ++hallX) {
            for (int hallY = anchorY + doorJ; hallY < anchorY + doorJ + 3;
                 ++hallY) {
                Tile &tile = world.getTile(hallX, hallY);
                tile.blockID = TileID::empty;
                tile.wallID = WallID::Unsafe::livingWood;
            }
        }
        std::set<Point> chests;
        for (int i = 0; i < room.getWidth(); ++i) {
            for (int j = 0; j < room.getHeight(); ++j) {
                Tile &roomTile = room.getTile(i, j);
                if (roomTile.blockID == TileID::cloud) {
                    continue;
                }
                if (roomTile.blockID == TileID::chest &&
                    !chests.contains({i - 1, j}) &&
                    !chests.contains({i, j - 1}) &&
                    !chests.contains({i - 1, j - 1})) {
                    chests.emplace(i, j);
                }
                roomTile.guarded = true;
                world.getTile(x + i, anchorY + j) = roomTile;
            }
        }
        for (auto [i, j] : chests) {
            Chest &chest =
                world.placeChest(x + i, anchorY + j, Variant::livingWood);
            fillSurfaceLivingWoodChest(chest, rnd, world);
        }
    });
}

void growLivingTree(double x, double y, int roomId, Random &rnd, World &world)
{
    double weight = rnd.getDouble(5, 10);
    growBranch(
        {x, y},
        weight,
        rnd.getDouble(2.1, 2.7),
        rnd.getDouble(-std::numbers::pi / 8, std::numbers::pi / 8) -
            std::numbers::pi / 2,
        rnd,
        world);
    growRoot(
        {x - 0.18 * weight, y},
        0.58 * weight,
        rnd.getDouble(std::numbers::pi / 8, std::numbers::pi / 4) +
            std::numbers::pi / 2,
        rnd,
        world);
    growRoot(
        {x + 0.18 * weight, y},
        0.58 * weight,
        rnd.getDouble(-std::numbers::pi / 4, -std::numbers::pi / 8) +
            std::numbers::pi / 2,
        rnd,
        world);
    growTapRoot(x, y, roomId, rnd, world);
}

void growLivingTrees(Random &rnd, World &world)
{
    auto partitions =
        rnd.partitionRange(world.getWidth() / 1280, world.getWidth());
    std::vector<int> rooms(Data::treeRooms.begin(), Data::treeRooms.end());
    std::shuffle(rooms.begin(), rooms.end(), rnd.getPRNG());
    for (int partition : partitions) {
        int numTrees = rnd.getInt(3, 6);
        for (int x = partition - 25 * numTrees; numTrees > 0;
             x += rnd.getInt(45, 55), --numTrees) {
            if (std::abs(x - world.getWidth() / 2) < 20) {
                continue;
            }
            int y = world.getSurfaceLevel(x);
            while (world.getTile(x, y).blockID == TileID::empty &&
                   y < world.getUndergroundLevel()) {
                ++y;
            }
            if (world.getTile(x, y).blockID == TileID::grass) {
                growLivingTree(x, y, rnd.pool(rooms), rnd, world);
            }
        }
    }
}

void buryEnchantedSwords(Random &rnd, World &world)
{
    double numSwords = world.getWidth() / rnd.getInt(1800, 3800);
    std::vector<Point> usedLocations;
    while (numSwords > 0) {
        int x = rnd.getInt(400, 0.36 * world.getWidth());
        if (rnd.getBool()) {
            x = world.getWidth() - x;
        }
        int y = world.getSurfaceLevel(x) + rnd.getInt(25, 45);
        if (!world.regionPasses(
                x - 16,
                y,
                32,
                20,
                [](Tile &tile) {
                    return tile.blockID != TileID::empty &&
                           tile.blockID != TileID::snow &&
                           tile.blockID != TileID::sand &&
                           tile.blockID != TileID::jungleGrass;
                }) ||
            isLocationUsed(x, y, 100, usedLocations)) {
            numSwords -= 0.002;
            continue;
        }
        usedLocations.emplace_back(x, y);
        for (int i = -15; i < 15; ++i) {
            for (int j = 3; j < 17; ++j) {
                if (std::hypot(i, 17 - j) + 2 * rnd.getFineNoise(x + i, y + j) <
                    13.5) {
                    Tile &tile = world.getTile(x + i, y + j);
                    tile.blockID = TileID::empty;
                    if (i == 0) {
                        tile.wallID = WallID::empty;
                    } else {
                        tile.wallID = WallID::Unsafe::flower;
                    }
                }
            }
        }
        for (int i = -15; i < 15; ++i) {
            for (int j = 2; j < 18; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::dirt &&
                    world.isExposed(x + i, y + j)) {
                    tile.blockID = TileID::grass;
                }
            }
        }
        world.queuedTreasures.emplace_back([x, y](Random &, World &world) {
            TileBuffer shrine = Data::getSwordShrine(world.getFramedTiles());
            if (!world.regionPasses(
                    x - shrine.getWidth() / 2,
                    y + 13,
                    shrine.getWidth(),
                    shrine.getHeight(),
                    [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
                !world.regionPasses(
                    x - shrine.getWidth() / 2,
                    y + 13 + shrine.getHeight(),
                    shrine.getWidth(),
                    1,
                    [](Tile &tile) { return tile.blockID != TileID::empty; })) {
                return;
            }
            for (int i = 0; i < shrine.getWidth(); ++i) {
                for (int j = 0; j < shrine.getHeight(); ++j) {
                    Tile &shrineTile = shrine.getTile(i, j);
                    if (shrineTile.blockID == TileID::empty) {
                        continue;
                    }
                    Tile &tile = world.getTile(
                        x - shrine.getWidth() / 2 + i,
                        y + 13 + j);
                    shrineTile.wallID = tile.wallID;
                    tile = shrineTile;
                    tile.guarded = true;
                }
                Tile &base = world.getTile(
                    x - shrine.getWidth() / 2 + i,
                    y + 13 + shrine.getHeight());
                if (base.blockID == TileID::grass) {
                    base.blockID = TileID::dirt;
                }
            }
        });
        --numSwords;
    }
}

void genForest(Random &rnd, World &world)
{
    std::cout << "Nurturing forests\n";
    rnd.shuffleNoise();
    // Grow grass.
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getUndergroundLevel(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::dirt) {
                if (world.isExposed(x, y) ||
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            100 ==
                        0) {
                    tile.blockID = TileID::grass;
                }
            } else if (
                tile.blockID == TileID::empty &&
                tile.wallID == WallID::Unsafe::dirt) {
                tile.wallID = rnd.getFineNoise(x, y) > 0
                                  ? WallID::Unsafe::grass
                                  : WallID::Unsafe::flower;
            }
        }
    }
    // Add living tree clumps.
    growLivingTrees(rnd, world);
    buryEnchantedSwords(rnd, world);
}
