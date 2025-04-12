#include "structures/Pyramid.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/data/Rooms.h"
#include <iostream>
#include <map>
#include <set>

typedef std::pair<int, int> Point;

Point makeHall(int x, int y, int steps, Point delta, World &world)
{
    std::map<int, int> accent1{
        {WallID::Safe::sandstoneBrick, WallID::Safe::smoothSandstone},
        {WallID::Safe::ebonstoneBrick, WallID::Unsafe::ebonsandstone},
        {WallID::Safe::crimstoneBrick, WallID::Unsafe::crimsandstone}};
    std::map<int, int> accent2{
        {WallID::Safe::sandstoneBrick, WallID::Safe::goldBrick},
        {WallID::Safe::ebonstoneBrick, WallID::Safe::demoniteBrick},
        {WallID::Safe::crimstoneBrick, WallID::Safe::crimtaneBrick}};
    std::set<int> validBlocks{
        TileID::sandstoneBrick,
        TileID::ebonstoneBrick,
        TileID::crimstoneBrick};
    for (; steps > 0; --steps, x += delta.first, y += delta.second) {
        for (int j = 0; j < 7; ++j) {
            Tile &tile = world.getTile(x, y + j);
            if (validBlocks.contains(tile.blockID)) {
                tile.blockID = TileID::empty;
                if (j == 0) {
                    tile.wallID = accent1[tile.wallID];
                } else if (j == 1) {
                    tile.wallID = accent2[tile.wallID];
                }
            }
        }
    }
    return {x, y};
}

Point fillTreasureRoom(int x, int y, Random &rnd, World &world)
{
    TileBuffer treasureRoom =
        Data::getRoom(rnd.select(Data::pyramidRooms), world.getFramedTiles());
    x -= treasureRoom.getWidth() - 1;
    int align = 0;
    for (int j = 0; j < treasureRoom.getHeight(); ++j) {
        if (treasureRoom.getTile(0, j).wallID ==
            WallID::Safe::smoothSandstone) {
            align = -j;
            break;
        }
    }
    std::set<Point> chests;
    for (int i = 0; i < treasureRoom.getWidth(); ++i) {
        for (int j = 0; j < treasureRoom.getHeight(); ++j) {
            Tile &roomTile = treasureRoom.getTile(i, j);
            if (roomTile.blockID == TileID::chest &&
                !chests.contains({i - 1, j}) && !chests.contains({i, j - 1}) &&
                !chests.contains({i - 1, j - 1})) {
                chests.emplace(i, j);
            }
            Tile &tile = world.getTile(x + i, y + j + align);
            std::map<int, int> blockMap{{roomTile.blockID, roomTile.blockID}};
            std::map<int, int> wallMap{{roomTile.wallID, roomTile.wallID}};
            if (tile.blockID == TileID::ebonstoneBrick) {
                blockMap[TileID::sand] = TileID::ebonsand;
                blockMap[TileID::sandstoneBrick] = TileID::ebonstoneBrick;
                wallMap[WallID::Safe::smoothSandstone] =
                    WallID::Unsafe::ebonsandstone;
                wallMap[WallID::Safe::goldBrick] = WallID::Safe::demoniteBrick;
                wallMap[WallID::Safe::sandstoneBrick] =
                    WallID::Safe::ebonstoneBrick;
            } else if (tile.blockID == TileID::crimstoneBrick) {
                blockMap[TileID::sand] = TileID::crimsand;
                blockMap[TileID::sandstoneBrick] = TileID::crimstoneBrick;
                wallMap[WallID::Safe::smoothSandstone] =
                    WallID::Unsafe::crimsandstone;
                wallMap[WallID::Safe::goldBrick] = WallID::Safe::crimtaneBrick;
                wallMap[WallID::Safe::sandstoneBrick] =
                    WallID::Safe::crimstoneBrick;
            }
            roomTile.blockID = blockMap[roomTile.blockID];
            roomTile.wallID = wallMap[roomTile.wallID];
            tile = roomTile;
        }
    }
    for (auto [i, j] : chests) {
        Chest &chest = world.placeChest(x + i, y + j + align, Variant::gold);
        fillPyramidChest(chest, rnd, world);
    }
    for (int i = 0; i < treasureRoom.getWidth(); ++i) {
        if (world.regionPasses(
                x + i,
                y + 5,
                2,
                2,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            rnd.getDouble(0, 1) < 0.9) {
            if (rnd.getDouble(0, 1) < 0.1) {
                world.placeFramedTile(
                    x + i,
                    y + 6,
                    TileID::smallPile,
                    Variant::gold);
            } else {
                world.placeFramedTile(
                    x + i,
                    y + 5,
                    TileID::pot,
                    Variant::pyramid);
            }
        }
    }
    for (int i : {1, 3, 10, 12, 23, 25, 32, 34}) {
        if (world.regionPasses(x + i, y, 1, 3, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            world.placeFramedTile(
                x + i,
                y,
                TileID::banner,
                rnd.select({Variant::ankh, Variant::omega, Variant::snake}));
        }
    }
    return {x - 1, y};
}

void applyGravity(int x, int y, int width, int height, World &world)
{
    std::set<int> unstableBlocks{
        TileID::sand,
        TileID::ebonsand,
        TileID::crimsand};
    for (int i = 0; i < width; ++i) {
        int lastGap = -1;
        for (int j = height; j > -1; --j) {
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.blockID == TileID::empty) {
                if (lastGap == -1) {
                    lastGap = j;
                }
            } else if (unstableBlocks.contains(tile.blockID)) {
                if (lastGap != -1) {
                    world.getTile(x + i, y + lastGap).blockID = tile.blockID;
                    tile.blockID = TileID::empty;
                    --lastGap;
                }
            } else {
                lastGap = -1;
            }
        }
    }
}

Point findLinkingCave(int x, int y, World &world)
{
    for (int j = 0; j < 200; ++j) {
        for (int iSwap = 0; iSwap < j; ++iSwap) {
            int i = iSwap / 2;
            if (iSwap % 2 == 0) {
                i = -i;
            }
            if (world.regionPasses(x + i - 2, y + j, 5, 4, [](Tile &tile) {
                    return tile.blockID == TileID::empty;
                })) {
                return {x + i, y + j};
            }
        }
    }
    return {-1, -1};
}

void genPyramid(Random &rnd, World &world)
{
    std::cout << "Building monuments\n";
    int size = 80;
    double scanDist = 0.061 * world.getWidth() - size;
    int x = world.surfaceEvilCenter;
    while (std::abs(x - world.surfaceEvilCenter) < 1.5 * size ||
           std::abs(x - world.getWidth() / 2) < 2 * size) {
        x = rnd.getInt(
            world.desertCenter - scanDist,
            world.desertCenter + scanDist);
    }
    int y = 0.6 * world.getUndergroundLevel();
    std::set<int> ignoreBlocks{TileID::empty, TileID::lesion, TileID::flesh};
    while (ignoreBlocks.contains(world.getTile(x, y).blockID) &&
           y < 0.85 * world.getUndergroundLevel()) {
        ++y;
    }
    x -= size;
    y -= 5;
    std::map<int, int> convertTiles{
        {TileID::ebonstone, TileID::ebonstoneBrick},
        {TileID::ebonsand, TileID::ebonstoneBrick},
        {TileID::ebonsandstone, TileID::ebonstoneBrick},
        {TileID::hardenedEbonsand, TileID::ebonstoneBrick},
        {TileID::crimstone, TileID::crimstoneBrick},
        {TileID::crimsand, TileID::crimstoneBrick},
        {TileID::crimsandstone, TileID::crimstoneBrick},
        {TileID::hardenedCrimsand, TileID::crimstoneBrick}};
    std::set<int> skipTiles{
        TileID::demonite,
        TileID::lesion,
        TileID::crimtane,
        TileID::flesh};
    for (int i = 0; i < 2 * size; ++i) {
        for (int j = std::abs(i - size); j < size; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            if (skipTiles.contains(tile.blockID)) {
                continue;
            }
            auto itr = convertTiles.find(tile.blockID);
            if (itr == convertTiles.end()) {
                tile.blockID = TileID::sandstoneBrick;
                tile.wallID = WallID::Safe::sandstoneBrick;
            } else {
                tile.blockID = itr->second;
                tile.wallID = itr->second == TileID::ebonstoneBrick
                                  ? WallID::Safe::ebonstoneBrick
                                  : WallID::Safe::crimstoneBrick;
            }
        }
    }
    std::tie(x, y) = makeHall(x + size - 16, y + 10, 10, {1, 0}, world);
    applyGravity(x - 12, y - 20, 12, 30, world);
    std::tie(x, y) = makeHall(x, y, 25, {1, 1}, world);
    std::tie(x, y) = makeHall(x, y, 12, {1, 0}, world);
    std::tie(x, y) = makeHall(x, y, 14, {-1, 1}, world);
    std::tie(x, y) = makeHall(x, y, 5, {-1, 0}, world);
    std::tie(x, y) = fillTreasureRoom(x, y, rnd, world);
    std::tie(x, y) = makeHall(x, y, 18, {-1, 0}, world);
    std::tie(x, y) = makeHall(x, y, 30, {1, 1}, world);
    x -= 3;
    ++y;
    Point cave = findLinkingCave(x, y, world);
    if (cave.first != -1 && cave.second > y + 2) {
        std::set<int> clearTiles{
            TileID::sand,
            TileID::hardenedSand,
            TileID::sandstone,
            TileID::ebonsand,
            TileID::crimsand};
        double aspect = static_cast<double>(cave.first - x) / (cave.second - y);
        for (int j = 0; y + j < cave.second; ++j) {
            int minI = aspect * j - 3 + 4 * rnd.getFineNoise(0, j);
            int maxI = aspect * j + 3 + 4 * rnd.getFineNoise(x, j);
            for (int i = minI; i < maxI; ++i) {
                Tile &tile = world.getTile(x + i, y + j);
                if (clearTiles.contains(tile.blockID)) {
                    tile.blockID = TileID::empty;
                }
            }
        }
    }
}
