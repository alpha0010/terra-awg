#include "structures/SpiderHall.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/Buildings.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

void convertRainbowBricks(int x, int y, Tile &tile)
{
    if (tile.blockID == TileID::stoneSlab) {
        tile.blockID = TileID::rainbowBrick;
    } else if (
        tile.blockID == TileID::stone || tile.blockID == TileID::grayBrick) {
        tile.blockPaint = getDeepRainbowPaint(x, y);
    }
    constexpr auto paintWalls = frozen::make_set<int>(
        {WallID::Unsafe::craggyStone,
         WallID::Safe::grayBrick,
         WallID::Unsafe::spider});
    if (tile.wallID == WallID::Safe::stoneSlab) {
        tile.wallID = WallID::Safe::rainbowBrick;
    } else if (paintWalls.contains(tile.wallID)) {
        tile.wallPaint = getDeepRainbowPaint(x, y);
    }
}

bool canPlaceSpiderDecoAt(int x, int y, int width, int height, World &world)
{
    return world.regionPasses(x, y, width, height, [](Tile &tile) {
        return tile.blockID == TileID::empty;
    }) && world.regionPasses(x, y + height, width, 1, [](Tile &tile) {
        return tile.slope == Slope::none && !tile.actuated &&
               isSolidBlock(tile.blockID);
    }) && world.getTile(x, y).wallID == WallID::Unsafe::spider;
}

void placeSpiderDeco(
    int x,
    int y,
    int width,
    int height,
    Random &rnd,
    World &world)
{
    std::vector<Point> locations;
    for (int i = 0; i < width; ++i) {
        bool prevIsSolid = false;
        for (int j = 0; j < height; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            bool curIsSolid = tile.guarded && tile.slope == Slope::none &&
                              (tile.blockID == TileID::stoneSlab ||
                               tile.blockID == TileID::stone);
            if (prevIsSolid && !curIsSolid) {
                locations.emplace_back(x + i, y + j);
            }
            prevIsSolid = curIsSolid;
            switch (
                static_cast<int>(99999 * (1 + rnd.getFineNoise(x + i, y + j))) %
                11) {
            case 0:
                if (canPlaceSpiderDecoAt(x + i, y + j, 2, 2, world)) {
                    world.placeFramedTile(
                        x + i,
                        y + j,
                        TileID::pot,
                        Variant::spider);
                }
                break;
            case 1:
                if (canPlaceSpiderDecoAt(x + i, y + j, 2, 1, world)) {
                    world.placeFramedTile(
                        x + i,
                        y + j,
                        TileID::smallPile,
                        Variant::spider);
                }
                break;
            case 2:
                if (canPlaceSpiderDecoAt(x + i, y + j, 3, 2, world)) {
                    world.placeFramedTile(
                        x + i,
                        y + j,
                        TileID::largePileGroup2,
                        Variant::spider);
                }
                break;
            }
        }
    }
    std::array paintings{
        Painting::gloriousNight,
        Painting::happyLittleTree,
        Painting::land,
        Painting::strangeGrowth,
        Painting::auroraBorealis,
        Painting::heartlands,
        Painting::vikingVoyage,
        Painting::wildflowers,
        Painting::discover,
        Painting::morningHunt,
        Painting::oldMiner,
        Painting::sunflowers,
        Painting::theMerchant};
    std::shuffle(paintings.begin(), paintings.end(), rnd.getPRNG());
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    auto paintingItr = paintings.begin();
    int numPaintings = rnd.getInt(4, 5);
    std::vector<Point> usedLocations;
    for (auto [pX, pY] : locations) {
        auto [pWidth, pHeight] = world.getPaintingDims(*paintingItr);
        if (!world.regionPasses(
                pX - 2,
                pY,
                pWidth + 2,
                pHeight + 2,
                [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           tile.wallID != WallID::Safe::grayBrick;
                }) ||
            isLocationUsed(pX, pY, 10, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(pX, pY);
        world.placePainting(pX - 1, pY + 1, *paintingItr);
        --numPaintings;
        if (numPaintings <= 0) {
            break;
        }
        ++paintingItr;
    }
}

void placeCobwebs(
    int centerX,
    int centerY,
    int width,
    int height,
    Random &rnd,
    World &world)
{
    for (int i = -width; i < width; ++i) {
        for (int j = -height; j < height; ++j) {
            double threshold = std::max(
                0.1,
                2 * std::hypot(
                        static_cast<double>(i) / width,
                        static_cast<double>(j) / height) -
                    1);
            if (rnd.getFineNoise(centerX + 4 * i, centerY + 4 * j) <
                threshold) {
                continue;
            }
            Tile &tile = world.getTile(centerX + i, centerY + j);
            if (tile.blockID == TileID::empty && tile.wallID != WallID::empty) {
                tile.blockID = TileID::cobweb;
            }
        }
    }
}

Point selectSpiderHallLocation(
    int minX,
    int maxX,
    TileBuffer &hall,
    Random &rnd,
    World &world)
{
    constexpr auto clearableTiles = frozen::make_set<int>({
        TileID::empty,         TileID::dirt,        TileID::stone,
        TileID::clay,          TileID::sand,        TileID::mud,
        TileID::copperOre,     TileID::tinOre,      TileID::ironOre,
        TileID::leadOre,       TileID::silverOre,   TileID::tungstenOre,
        TileID::goldOre,       TileID::platinumOre, TileID::cobaltOre,
        TileID::palladiumOre,  TileID::mythrilOre,  TileID::orichalcumOre,
        TileID::adamantiteOre, TileID::titaniumOre,
    });
    int minY = (world.getUndergroundLevel() + 2 * world.getCavernLevel()) / 3;
    int maxY = (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3 -
               hall.getHeight() / 2;
    int maxFoundationEmpty = 0.4 * hall.getWidth();
    for (int tries = 0; tries < 8000; ++tries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        int numEmpty = 0;
        int numFilled = 0;
        int maxEntryFilled = tries / 250;
        if (world.regionPasses(
                x - 3,
                y + hall.getHeight() - 9,
                4,
                4,
                [maxEntryFilled, &numFilled](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < maxEntryFilled;
                }) &&
            world.regionPasses(
                x + hall.getWidth() - 1,
                y + hall.getHeight() - 9,
                4,
                4,
                [maxEntryFilled, &numFilled](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < maxEntryFilled;
                }) &&
            world.regionPasses(
                x - 20,
                y - 20,
                hall.getWidth() + 40,
                hall.getHeight() + 40,
                [&clearableTiles](Tile &tile) {
                    return !tile.guarded && tile.liquid != Liquid::shimmer &&
                           (clearableTiles.contains(tile.blockID) ||
                            (tile.flag == Flag::border &&
                             tile.blockID == TileID::hive));
                }) &&
            world.regionPasses(
                x,
                y + hall.getHeight() - 2,
                hall.getWidth(),
                4,
                [maxFoundationEmpty, &numEmpty](Tile &tile) {
                    if (tile.blockID == TileID::empty) {
                        ++numEmpty;
                    }
                    return numEmpty < maxFoundationEmpty;
                })) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void genSpiderHall(Random &rnd, World &world)
{
    std::cout << "Spinning webs\n";
    TileBuffer hall = Data::getBuilding(
        rnd.select({Data::Building::spiderHall1, Data::Building::spiderHall2}),
        world.getFramedTiles());
    std::vector<Point> bounds;
    if (world.getWidth() < 8000) {
        bounds.emplace_back(200, world.getWidth() - hall.getWidth() - 200);
    } else {
        bounds.emplace_back(200, 0.35 * world.getWidth() - hall.getWidth());
        bounds.emplace_back(
            0.65 * world.getWidth(),
            world.getWidth() - hall.getWidth() - 200);
    }
    for (auto [minX, maxX] : bounds) {
        auto [x, y] = selectSpiderHallLocation(minX, maxX, hall, rnd, world);
        if (x == -1) {
            continue;
        }
        for (int i = 0; i < hall.getWidth(); ++i) {
            for (int j = 0; j < hall.getHeight(); ++j) {
                Tile &hallTile = hall.getTile(i, j);
                if (hallTile.blockID == TileID::empty &&
                    hallTile.wallID == WallID::empty) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                bool shouldConvertWall =
                    hallTile.wallID != WallID::Safe::grayBrick &&
                    hallTile.wallID != WallID::empty;
                if (static_cast<int>(
                        99999 * (1 + rnd.getFineNoise(x + i, y + j))) %
                        5 ==
                    0) {
                    if (hallTile.blockID == TileID::grayBrick ||
                        hallTile.blockID == TileID::stoneSlab) {
                        hallTile.blockID = TileID::stone;
                    }
                    if (hallTile.wallID == WallID::Safe::grayBrick ||
                        hallTile.wallID == WallID::Safe::stoneSlab) {
                        hallTile.wallID = WallID::Unsafe::craggyStone;
                    }
                }
                if (shouldConvertWall &&
                    std::abs(rnd.getFineNoise(x + 3 * i, y + 3 * j)) < 0.45) {
                    hallTile.wallID = WallID::Unsafe::spider;
                } else if (hallTile.wallID == WallID::empty) {
                    hallTile.wallID = tile.wallID;
                }
                if (world.conf.celebration) {
                    convertRainbowBricks(x + i, y + j, hallTile);
                }
                tile = hallTile;
                tile.guarded = true;
                if (tile.blockID == TileID::chest && tile.frameX % 36 == 0 &&
                    tile.frameY == 0) {
                    fillWebCoveredChest(
                        world.registerStorage(x + i, y + j),
                        rnd,
                        world);
                }
            }
        }
        placeSpiderDeco(x, y, hall.getWidth(), hall.getHeight(), rnd, world);
        placeCobwebs(
            x + hall.getWidth() / 2,
            y + hall.getHeight() / 2,
            hall.getWidth(),
            hall.getHeight(),
            rnd,
            world);
        hall = Data::getBuilding(
            rnd.select(
                {Data::Building::spiderHall1, Data::Building::spiderHall2}),
            world.getFramedTiles());
    }
}
