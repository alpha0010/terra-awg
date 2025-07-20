#include "Cloud.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/SkyBoxes.h"
#include <algorithm>
#include <iostream>
#include <numbers>

bool isRowZoneEmpty(int x, int y, World &world)
{
    return world.regionPasses(x - 2, y, 5, 1, [](Tile &tile) {
        return tile.blockID == TileID::empty;
    });
}

void makeFishingCloud(
    int startX,
    int startY,
    int width,
    int height,
    Random &rnd,
    World &world)
{
    int minX = startX + 0.2 * width;
    int maxX = startX + 0.8 * width;
    int waterLevel = startY;
    while (isRowZoneEmpty(minX, waterLevel, world) ||
           isRowZoneEmpty(maxX, waterLevel, world)) {
        ++waterLevel;
    }
    ++waterLevel;
    for (int x = minX; x < maxX; ++x) {
        int depth = 0.45 * height *
                    std::sin(std::numbers::pi * (x - minX) / (0.6 * width));
        int y = startY;
        while (depth > 0) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::empty) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                if (y > waterLevel) {
                    tile.liquid = Liquid::water;
                }
                --depth;
            }
            ++y;
        }
    }
    embedWaterfalls(
        {startX, startY},
        {startX + width, startY + height},
        {TileID::cloud,
         TileID::rainCloud,
         TileID::snowCloud,
         TileID::lesion,
         TileID::flesh},
        Liquid::water,
        14,
        rnd,
        world);
}

void makeResourceCloud(
    int startX,
    int startY,
    int width,
    int height,
    Random &rnd,
    World &world)
{
    int minX = startX + 0.2 * width;
    int maxX = startX + 0.8 * width;
    for (int x = minX; x < maxX; ++x) {
        int depth = 0.45 * height *
                    std::sin(std::numbers::pi * (x - minX) / (0.6 * width));
        int y = startY;
        bool prevWasEmpty = true;
        while (depth > 0) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::empty) {
                tile.blockID = TileID::dirt;
                if (prevWasEmpty) {
                    tile.wallID = WallID::empty;
                    prevWasEmpty = false;
                } else {
                    tile.wallID = WallID::Unsafe::dirt;
                }
                --depth;
            }
            ++y;
        }
    }
    for (int x = startX; x < startX + width; ++x) {
        for (int y = startY; y < startY + height; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::empty &&
                (std::abs(rnd.getFineNoise(x, y + startY)) > 0.65 ||
                 std::abs(rnd.getFineNoise(x + startX, y)) > 0.65)) {
                tile.blockID = world.goldVariant;
            }
        }
    }
}

void makeGraveCloud(
    int startX,
    int startY,
    int width,
    int height,
    Random &rnd,
    World &world)
{
    int radius = width / 2;
    int centerX = startX + radius;
    int centerY = startY + height / 2;
    radius = 2 + 0.9 * radius;
    double delta =
        2 * std::numbers::pi / std::ceil(0.1 * std::numbers::pi * radius);
    double offset = rnd.getDouble(0, delta);
    std::vector<Point> locations;
    for (double t = 0.0001; t < 2 * std::numbers::pi; t += delta) {
        locations.emplace_back(
            centerX + radius * std::cos(t + offset),
            centerY + radius * std::sin(t + offset));
    }
    world.queuedDeco.emplace_back([locations](Random &, World &world) {
        for (auto [x, y] : locations) {
            if (world.regionPasses(x - 2, y - 2, 6, 6, [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           tile.wallID == WallID::empty;
                })) {
                world.placeFramedTile(x, y, TileID::tombstone);
                for (int i = 0; i < 2; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        world.getTile(x + i, y + j).echoCoatBlock = true;
                    }
                }
            }
        }
    });
}

void addCloudStructure(
    int startX,
    int startY,
    int width,
    int roomId,
    int fillTile,
    World &world)
{
    TileBuffer room = Data::getSkyBox(roomId, world.getFramedTiles());
    int x = startX + (width - room.getWidth()) / 2;
    int surfaceLeft = scanWhileEmpty({x, startY}, {0, 1}, world).y + 1;
    int surfaceRight =
        scanWhileEmpty({x + room.getWidth() - 1, startY}, {0, 1}, world).y + 1;
    int y = std::max(surfaceLeft, surfaceRight) - room.getHeight();
    for (int j = 0; j < room.getHeight(); ++j) {
        int jLeft = surfaceLeft - y - j;
        int jRight = surfaceRight - y - j;
        if (jLeft >= 0 && jRight >= 0 && jLeft < room.getHeight() &&
            jRight < room.getHeight() &&
            room.getTile(0, jLeft).blockPaint == Paint::red &&
            room.getTile(room.getWidth() - 1, jRight).blockPaint ==
                Paint::red) {
            y += j;
            break;
        }
    }
    for (int i = 0; i < room.getWidth(); ++i) {
        for (int j = 0; j < room.getHeight(); ++j) {
            Tile &roomTile = room.getTile(i, j);
            if (roomTile.blockID == TileID::cloud) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            if (roomTile.blockID != TileID::empty) {
                tile.blockID = TileID::cloud;
            }
        }
    }
    for (int i = 1; i < room.getWidth() - 1; ++i) {
        for (int j = -1; j < 4; ++j) {
            Tile &tile = world.getTile(x + i, y + room.getHeight() + j);
            if (tile.blockID == TileID::empty) {
                tile.blockID = fillTile;
            }
        }
    }
    world.queuedTreasures.emplace_back(
        [x, y, roomId](Random &rnd, World &world) {
            TileBuffer room = Data::getSkyBox(roomId, world.getFramedTiles());
            std::vector<Point> chests;
            for (int i = 0; i < room.getWidth(); ++i) {
                for (int j = 0; j < room.getHeight(); ++j) {
                    Tile &roomTile = room.getTile(i, j);
                    if (roomTile.blockID == TileID::cloud) {
                        continue;
                    }
                    roomTile.guarded = roomTile.blockID != TileID::empty ||
                                       roomTile.wallID != WallID::empty;
                    Tile &tile = world.getTile(x + i, y + j);
                    if (!roomTile.guarded &&
                        (tile.blockID == TileID::livingMahogany ||
                         tile.blockID == TileID::mahoganyLeaf)) {
                        roomTile.blockID = tile.blockID;
                    }
                    tile = roomTile;
                    if (tile.blockID == TileID::chest &&
                        tile.frameX % 36 == 0 && tile.frameY == 0) {
                        chests.emplace_back(x + i, y + j);
                    } else if (
                        tile.blockID == TileID::dresser &&
                        tile.frameX % 54 == 0 && tile.frameY == 0) {
                        fillDresser(world.registerStorage(x + i, y + j), rnd);
                    }
                }
            }
            for (auto [chestX, chestY] : chests) {
                fillSkywareChest(
                    world.conf.forTheWorthy
                        ? world.placeChest(chestX, chestY, Variant::goldLocked)
                        : world.registerStorage(chestX, chestY),
                    rnd,
                    world);
            }
        });
}

void genCloud(Random &rnd, World &world)
{
    std::cout << "Condensing clouds\n";
    rnd.shuffleNoise();
    int numClouds =
        world.conf.clouds * world.getWidth() / rnd.getInt(600, 1300);
    std::vector<int> rooms(Data::skyBoxes.begin(), Data::skyBoxes.end());
    std::shuffle(rooms.begin(), rooms.end(), rnd.getPRNG());
    auto roomItr = rooms.begin();
    double cloudScale = std::min(0.1 + world.getHeight() / 1400.0, 1.0);
    int hSpacingBuffer = world.conf.hiveQueen ? -10 : 50;
    for (int tries = 500 * numClouds; numClouds > 0 && tries > 0; --tries) {
        int width = cloudScale * rnd.getInt(90, 160);
        int height = cloudScale * rnd.getInt(35, 50);
        int x = rnd.getInt(
            cloudScale * 200,
            world.getWidth() - cloudScale * 200 - width);
        int maxY =
            std::max<int>(0.45 * world.getUndergroundLevel() - height, 50);
        int y = rnd.getInt(
            std::min<int>(
                numClouds == 3 ? std::midpoint<int>(cloudScale * 100, maxY)
                               : cloudScale * 100,
                maxY - 1),
            maxY);
        if (!world.regionPasses(
                x - hSpacingBuffer / 2,
                y - 25,
                width + hSpacingBuffer,
                height + 50,
                [](Tile &tile) { return tile.blockID == TileID::empty; })) {
            continue;
        }
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                auto [hI, hJ] = world.conf.hiveQueen ? getHexCentroid(i, j, 8)
                                                     : Point{i, j};
                double threshold =
                    8 * std::hypot(
                            static_cast<double>(hI) / width - 0.5,
                            static_cast<double>(hJ) / height - 0.5) -
                    3;
                if (rnd.getFineNoise(x + hI, y + hJ) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                tile.blockID = TileID::cloud;
                tile.wallID = WallID::Safe::cloud;
            }
        }
        switch (numClouds % 3) {
        case 1:
            makeFishingCloud(x, y, width, height, rnd, world);
            break;
        case 2:
            makeResourceCloud(x, y, width, height, rnd, world);
            addCloudStructure(x, y, width, *roomItr, TileID::dirt, world);
            ++roomItr;
            break;
        default:
            if (numClouds == 3) {
                makeGraveCloud(x, y, width, height, rnd, world);
            }
            addCloudStructure(x, y, width, *roomItr, TileID::cloud, world);
            ++roomItr;
            break;
        }
        --numClouds;
        if (roomItr == rooms.end()) {
            roomItr = rooms.begin();
        }
    }
}
