#include "structures/Pyramid.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/data/Rooms.h"
#include "vendor/frozen/map.h"
#include "vendor/frozen/set.h"
#include <iostream>
#include <map>
#include <set>

typedef std::pair<int, int> Point;

template <typename T> constexpr int getOrKey(const T &data, int key)
{
    auto itr = data.find(key);
    return itr == data.end() ? key : itr->second;
}

Point makeHall(int x, int y, int steps, Point delta, World &world)
{
    constexpr auto accent1 = frozen::make_map<int, int>(
        {{WallID::Safe::sandstoneBrick, WallID::Safe::smoothSandstone},
         {WallID::Safe::ebonstoneBrick, WallID::Unsafe::ebonsandstone},
         {WallID::Safe::crimstoneBrick, WallID::Unsafe::crimsandstone},
         {WallID::Safe::pearlstoneBrick, WallID::Unsafe::pearlsandstone}});
    constexpr auto accent2 = frozen::make_map<int, int>(
        {{WallID::Safe::sandstoneBrick, WallID::Safe::goldBrick},
         {WallID::Safe::ebonstoneBrick, WallID::Safe::demoniteBrick},
         {WallID::Safe::crimstoneBrick, WallID::Safe::crimtaneBrick},
         {WallID::Safe::pearlstoneBrick, WallID::Safe::crystalBlock}});
    constexpr auto validBlocks = frozen::make_set<int>(
        {TileID::sandstoneBrick,
         TileID::ebonstoneBrick,
         TileID::crimstoneBrick,
         TileID::pearlstoneBrick});
    for (; steps > 0; --steps, x += delta.first, y += delta.second) {
        for (int j = 0; j < 7; ++j) {
            Tile &tile = world.getTile(x, y + j);
            if (validBlocks.contains(tile.blockID)) {
                tile.blockID = TileID::empty;
                if (j == 0) {
                    tile.wallID = getOrKey(accent1, tile.wallID);
                } else if (j == 1) {
                    tile.wallID = getOrKey(accent2, tile.wallID);
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
    for (int i = 0; i < treasureRoom.getWidth(); ++i) {
        for (int j = 0; j < treasureRoom.getHeight(); ++j) {
            Tile &roomTile = treasureRoom.getTile(i, j);
            if (roomTile.blockID == TileID::chest &&
                roomTile.frameX % 36 == 0 && roomTile.frameY == 0) {
                fillPyramidChest(
                    world.registerStorage(x + i, y + j + align),
                    rnd,
                    world);
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
            } else if (tile.blockID == TileID::pearlstoneBrick) {
                blockMap[TileID::sand] = TileID::pearlsand;
                blockMap[TileID::sandstoneBrick] = TileID::pearlstoneBrick;
                wallMap[WallID::Safe::smoothSandstone] =
                    WallID::Unsafe::pearlsandstone;
                wallMap[WallID::Safe::goldBrick] = WallID::Safe::crystalBlock;
                wallMap[WallID::Safe::sandstoneBrick] =
                    WallID::Safe::pearlstoneBrick;
            }
            roomTile.blockID = blockMap[roomTile.blockID];
            roomTile.wallID = wallMap[roomTile.wallID];
            tile = roomTile;
        }
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
    constexpr auto unstableBlocks = frozen::make_set<int>(
        {TileID::sand, TileID::ebonsand, TileID::crimsand});
    for (int i = 0; i < width; ++i) {
        int lastGap = -1;
        std::set<int> fallenTiles;
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
                    fallenTiles.insert(j);
                    --lastGap;
                }
            } else {
                lastGap = -1;
            }
        }
        int cloneWall = world.getTile(x + i, y).wallID;
        for (int j = 0; j < height; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.blockID != TileID::empty) {
                break;
            }
            if (fallenTiles.contains(j)) {
                tile.wallID = cloneWall;
            }
        }
    }
}

Point findLinkingCave(int x, int y, World &world)
{
    for (int j = 30; j < 200; ++j) {
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

std::vector<int> getDesertSurfaceCols(int size, World &world)
{
    std::vector<int> vals;
    if (world.conf.patches) {
        for (int x = 350; x < world.getWidth() - 350; ++x) {
            if (world.getBiome(x, world.getSurfaceLevel(x)).desert > 0.99) {
                vals.push_back(x);
            }
        }
    } else {
        double scanDist =
            world.conf.desertSize * 0.061 * world.getWidth() - size;
        for (int x = world.desertCenter - scanDist;
             x < world.desertCenter + scanDist;
             ++x) {
            vals.push_back(x);
        }
    }
    return vals;
}

void genPyramid(Random &rnd, World &world)
{
    std::cout << "Building monuments\n";
    int size = 80;
    int x = world.surfaceEvilCenter;
    int numTries = 0;
    auto desertSurface = getDesertSurfaceCols(size, world);
    if (desertSurface.empty()) {
        return;
    }
    constexpr auto avoidTiles = frozen::make_set<int>(
        {TileID::blueBrick, TileID::greenBrick, TileID::pinkBrick});
    while (std::abs(x - world.surfaceEvilCenter) < 1.5 * size ||
           std::abs(x - world.getWidth() / 2) < 2 * size ||
           !world.regionPasses(
               x - size,
               world.getSurfaceLevel(x),
               2 * size,
               size,
               [&avoidTiles](Tile &tile) {
                   return !avoidTiles.contains(tile.blockID);
               })) {
        x = rnd.select(desertSurface);
        ++numTries;
        if (numTries > 1000) {
            return;
        }
    }
    int y = world.getSurfaceLevel(x);
    constexpr auto ignoreBlocks = frozen::make_set<int>(
        {TileID::empty, TileID::lesion, TileID::flesh, TileID::crystalBlock});
    while (ignoreBlocks.contains(world.getTile(x, y).blockID) &&
           y < 0.85 * world.getUndergroundLevel()) {
        ++y;
    }
    x -= size;
    y -= 5;
    constexpr auto convertTiles = frozen::make_map<int, int>(
        {{TileID::ebonstone, TileID::ebonstoneBrick},
         {TileID::ebonsand, TileID::ebonstoneBrick},
         {TileID::ebonsandstone, TileID::ebonstoneBrick},
         {TileID::hardenedEbonsand, TileID::ebonstoneBrick},
         {TileID::crimstone, TileID::crimstoneBrick},
         {TileID::crimsand, TileID::crimstoneBrick},
         {TileID::crimsandstone, TileID::crimstoneBrick},
         {TileID::hardenedCrimsand, TileID::crimstoneBrick},
         {TileID::pearlstone, TileID::pearlstoneBrick},
         {TileID::pearlsand, TileID::pearlstoneBrick},
         {TileID::pearlsandstone, TileID::pearlstoneBrick},
         {TileID::hardenedPearlsand, TileID::pearlstoneBrick}});
    constexpr auto skipTiles = frozen::make_set<int>(
        {TileID::demonite,
         TileID::lesion,
         TileID::crimtane,
         TileID::flesh,
         TileID::crystalBlock});
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
                              : itr->second == TileID::pearlstoneBrick
                                  ? WallID::Safe::pearlstoneBrick
                                  : WallID::Safe::crimstoneBrick;
            }
        }
    }
    std::tie(x, y) = makeHall(x + size - 16, y + 10, 10, {1, 0}, world);
    applyGravity(x - 12, y - 20, 12, 30, world);
    world.queuedTreasures.emplace_back([x, y](Random &, World &world) {
        for (int i = -12; i < 12; ++i) {
            for (int j = -20; j < 10; ++j) {
                world.getTile(x + i, y + j).liquid = Liquid::none;
            }
        }
    });
    std::tie(x, y) = makeHall(x, y, 25, {1, 1}, world);
    std::tie(x, y) = makeHall(x, y, 12, {1, 0}, world);
    std::tie(x, y) = makeHall(x, y, 14, {-1, 1}, world);
    std::tie(x, y) = makeHall(x, y, 5, {-1, 0}, world);
    if (rnd.getBool()) {
        world.placePainting(x + 11, y - 13, Painting::ancientTablet);
    }
    std::tie(x, y) = fillTreasureRoom(x, y, rnd, world);
    std::tie(x, y) = makeHall(x, y, 18, {-1, 0}, world);
    std::tie(x, y) = makeHall(x, y, 30, {1, 1}, world);
    x -= 3;
    ++y;
    Point cave = findLinkingCave(x, y, world);
    if (cave.first != -1 && cave.second > y + 2) {
        constexpr auto clearTiles = frozen::make_set<int>(
            {TileID::sand,
             TileID::hardenedSand,
             TileID::sandstone,
             TileID::ebonsand,
             TileID::hardenedEbonsand,
             TileID::ebonsandstone,
             TileID::crimsand,
             TileID::hardenedCrimsand,
             TileID::crimsandstone,
             TileID::pearlsand,
             TileID::hardenedPearlsand,
             TileID::pearlsandstone});
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
