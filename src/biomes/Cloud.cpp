#include "Cloud.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/SkyBoxes.h"
#include <algorithm>
#include <iostream>

void makeFishingCloud(
    int startX,
    int startY,
    int width,
    int height,
    World &world)
{
    int minX = startX + 0.2 * width;
    int maxX = startX + 0.8 * width;
    int waterLevel = startY;
    while (world.getTile(minX, waterLevel).blockID == TileID::empty ||
           world.getTile(maxX, waterLevel).blockID == TileID::empty) {
        ++waterLevel;
    }
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
    int surfaceLeft = scanWhileEmpty({x, startY}, {0, 1}, world).second + 1;
    int surfaceRight =
        scanWhileEmpty({x + room.getWidth() - 1, startY}, {0, 1}, world)
            .second +
        1;
    int y = std::max(surfaceLeft, surfaceRight) - room.getHeight();
    for (int j = 0; j < room.getHeight(); ++j) {
        int jLeft = surfaceLeft - y - j;
        int jRight = surfaceRight - y - j;
        if (jLeft >= 0 && jRight >= 0 && jLeft < room.getHeight();
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
                        fillSkywareChest(
                            world.registerStorage(x + i, y + j),
                            rnd,
                            world);
                    } else if (
                        tile.blockID == TileID::dresser &&
                        tile.frameX % 54 == 0 && tile.frameY == 0) {
                        fillDresser(world.registerStorage(x + i, y + j), rnd);
                    }
                }
            }
        });
}

void genCloud(Random &rnd, World &world)
{
    std::cout << "Condensing clouds\n";
    rnd.shuffleNoise();
    int numClouds = world.getWidth() / rnd.getInt(600, 1300);
    std::vector<int> rooms(Data::skyBoxes.begin(), Data::skyBoxes.end());
    std::shuffle(rooms.begin(), rooms.end(), rnd.getPRNG());
    auto roomItr = rooms.begin();
    while (numClouds > 0) {
        int width = rnd.getInt(90, 160);
        int height = rnd.getInt(35, 50);
        int x = rnd.getInt(200, world.getWidth() - 200 - width);
        int y = rnd.getInt(100, 0.45 * world.getUndergroundLevel() - height);
        if (!world.regionPasses(
                x - 25,
                y - 25,
                width + 50,
                height + 50,
                [](Tile &tile) { return tile.blockID == TileID::empty; })) {
            continue;
        }
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                double threshold =
                    8 * std::hypot(
                            static_cast<double>(i) / width - 0.5,
                            static_cast<double>(j) / height - 0.5) -
                    3;
                if (rnd.getFineNoise(x + i, y + j) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                tile.blockID = TileID::cloud;
                tile.wallID = WallID::Safe::cloud;
            }
        }
        switch (numClouds % 3) {
        case 1:
            makeFishingCloud(x, y, width, height, world);
            break;
        case 2:
            makeResourceCloud(x, y, width, height, rnd, world);
            addCloudStructure(x, y, width, *roomItr, TileID::dirt, world);
            ++roomItr;
            break;
        default:
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
