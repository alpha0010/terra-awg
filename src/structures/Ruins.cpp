#include "structures/Ruins.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include "structures/data/Furniture.h"
#include <algorithm>
#include <iostream>
#include <set>

int makeCongruent(int val, int mod)
{
    return mod * (val / mod);
}

bool canPlaceFurniture(int x, int y, TileBuffer &data, World &world)
{
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.wallID != WallID::Unsafe::obsidianBrick && i != 0 &&
                j != 0 && i != data.getWidth() - 1 &&
                j != data.getHeight() - 1) {
                return false;
            }
            switch (data.getTile(i, j).blockID) {
            case TileID::empty:
                if (isSolidBlock(tile.blockID)) {
                    return false;
                }
                break;
            case TileID::cloud:
                if (tile.blockID != TileID::obsidianBrick) {
                    return false;
                }
                break;
            default:
                if (tile.blockID != TileID::empty) {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}

void genRuins(Random &rnd, World &world)
{
    std::cout << "Abandoning cities\n";
    int step = 8;
    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    int cityBase = makeCongruent(
        world.getUnderworldLevel() + 0.49 * underworldHeight,
        step);
    auto isInBuilding = [cityBase, step, underworldHeight, &rnd](int x, int y) {
        double threshold =
            1.6 * (cityBase - makeCongruent(y, step)) / underworldHeight - 0.2;
        return rnd.getFineNoise(
                   makeCongruent(x, step),
                   makeCongruent(y, step)) > threshold;
    };
    auto isRoomWall = [step, &rnd](int x, int y) {
        return static_cast<int>(
                   99999 * (1 + rnd.getFineNoise(x, makeCongruent(y, step)))) %
                   23 ==
               0;
    };
    auto isPlatform = [step, &rnd](int x, int y) {
        return static_cast<int>(
                   99999 * (1 + rnd.getFineNoise(
                                    makeCongruent(x, step / 2) + step / 4,
                                    y))) %
                   11 ==
               0;
    };
    std::set<Point> queuedPlatforms;
    std::vector<Point> locations;
    int minX = makeCongruent(0.15 * world.getWidth(), step);
    int maxX = makeCongruent(0.85 * world.getWidth(), step);
    for (int x = minX; x < maxX; ++x) {
        if (x % step == 0 && (rnd.getCoarseNoise(x, 0) < 0.15 ||
                              !world.regionPasses(
                                  x,
                                  world.getUnderworldLevel(),
                                  step,
                                  cityBase - world.getUnderworldLevel(),
                                  [](Tile &tile) { return !tile.guarded; }))) {
            x += step - 1;
            continue;
        } else if (isInBuilding(x, cityBase - 1)) {
            // Base.
            Tile &tile = world.getTile(x, cityBase);
            if (tile.blockID != TileID::lesion &&
                tile.blockID != TileID::flesh) {
                tile.blockID = TileID::obsidianBrick;
                tile.liquid = Liquid::none;
            }
            for (int j = 1; j < 20; ++j) {
                double noise = rnd.getFineNoise(x, cityBase + j);
                if (noise > j / 10.0 - 1) {
                    Tile &baseTile = world.getTile(x, cityBase + j);
                    if (baseTile.blockID == TileID::empty) {
                        baseTile.blockID =
                            static_cast<int>(99999 * (1 + noise)) % 5 == 0
                                ? TileID::obsidianBrick
                                : TileID::ash;
                        baseTile.liquid = Liquid::none;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            continue;
        }
        for (int y = cityBase - 1; y > world.getUnderworldLevel(); --y) {
            if (!isInBuilding(x, y)) {
                break;
            }
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::lesion ||
                tile.blockID == TileID::flesh) {
                continue;
            }
            locations.emplace_back(x, y);
            tile.liquid = Liquid::none;
            if (!isInBuilding(x - 1, y) ||
                (isInBuilding(x, y - 1) && !isInBuilding(x - 1, y - 1)) ||
                !isInBuilding(x + 1, y) ||
                (isInBuilding(x, y - 1) && !isInBuilding(x + 1, y - 1))) {
                // Outer wall.
                if (step - y % step == 3 &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            7 ==
                        0) {
                    world
                        .placeFramedTile(x, y, TileID::door, Variant::obsidian);
                } else {
                    tile.blockID = TileID::obsidianBrick;
                }
            } else if (
                isRoomWall(x, y) && !isRoomWall(x + 1, y) &&
                !isRoomWall(x + 2, y)) {
                // Room wall.
                if (step - y % step == 3 &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            2 ==
                        0) {
                    world
                        .placeFramedTile(x, y, TileID::door, Variant::obsidian);
                } else {
                    tile.blockID = TileID::obsidianBrick;
                }
                tile.wallID = WallID::Unsafe::obsidianBrick;
            } else if (!isInBuilding(x, y - 1)) {
                // Roof.
                tile.blockID = TileID::obsidianBrick;
                if (isPlatform(x, y)) {
                    queuedPlatforms.emplace(x, y);
                }
            } else if (y % step == 0) {
                // Room floor.
                tile.blockID = TileID::obsidianBrick;
                tile.wallID = WallID::Unsafe::obsidianBrick;
                if (isPlatform(x, y)) {
                    queuedPlatforms.emplace(x, y);
                }
            } else {
                // Room.
                tile.blockID = TileID::empty;
                tile.wallID = WallID::Unsafe::obsidianBrick;
            }
        }
    }
    std::erase_if(queuedPlatforms, [&world](const auto &pt) {
        return world.getTile(pt.first, pt.second - 1).blockID !=
                   TileID::empty ||
               world.getTile(pt.first, pt.second + 1).blockID != TileID::empty;
    });
    for (auto [x, y] : queuedPlatforms) {
        if (queuedPlatforms.contains({x - 1, y}) ||
            queuedPlatforms.contains({x + 1, y})) {
            placePlatform(x, y, Platform::obsidian, world);
        }
    }
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    TileBuffer data;
    int tries = 0;
    // Furniture.
    int numPlacements = locations.size() / 250;
    auto locItr = locations.begin();
    for (; locItr != locations.end(); ++locItr) {
        auto [x, y] = *locItr;
        if (tries > 30) {
            tries = 0;
        }
        if (tries == 0) {
            data = getFurniture(
                rnd.select(Data::furnitureLayouts),
                Data::Variant::obsidian,
                world.getFramedTiles());
        }
        ++tries;
        if (!canPlaceFurniture(x, y, data, world)) {
            continue;
        }
        for (int i = 0; i < data.getWidth(); ++i) {
            for (int j = 0; j < data.getHeight(); ++j) {
                Tile &dataTile = data.getTile(i, j);
                if (dataTile.blockID == TileID::empty ||
                    dataTile.blockID == TileID::cloud) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                tile.blockID = dataTile.blockID;
                tile.frameX = dataTile.frameX;
                tile.frameY = dataTile.frameY;
                if (((tile.blockID == TileID::dresser &&
                      tile.frameX % 54 == 0) ||
                     (tile.blockID == TileID::chest &&
                      tile.frameX % 36 == 0)) &&
                    tile.frameY == 0) {
                    fillDresser(world.registerStorage(x + i, y + j), rnd);
                }
            }
        }
        tries = 0;
        --numPlacements;
        if (numPlacements < 0) {
            break;
        }
    }
    std::array underworldPaintings{
        Painting::darkness,
        Painting::darkSoulReaper,
        Painting::trappedGhost,
        Painting::demonsEye,
        Painting::livingGore,
        Painting::flowingMagma,
        Painting::handEarth,
        Painting::impFace,
        Painting::ominousPresence,
        Painting::shiningMoon,
        Painting::skelehead,
        Painting::lakeOfFire};
    std::shuffle(
        underworldPaintings.begin(),
        underworldPaintings.end(),
        rnd.getPRNG());
    auto paintingItr = underworldPaintings.begin();
    // Paintings.
    numPlacements = locations.size() / 320;
    std::vector<Point> usedLocations;
    for (; locItr != locations.end(); ++locItr) {
        auto [x, y] = *locItr;
        auto [width, height] = world.getPaintingDims(*paintingItr);
        if (!world.regionPasses(
                x,
                y,
                width,
                height,
                [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           tile.wallID == WallID::Unsafe::obsidianBrick;
                }) ||
            isLocationUsed(x, y, 40, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        world.placePainting(x, y, *paintingItr);
        --numPlacements;
        if (numPlacements < 0) {
            break;
        }
        ++paintingItr;
        if (paintingItr == underworldPaintings.end()) {
            paintingItr = underworldPaintings.begin();
        }
    }
    // Chests.
    numPlacements = locations.size() / 640;
    usedLocations.clear();
    for (; locItr != locations.end() && numPlacements > 0; ++locItr) {
        auto [x, y] = *locItr;
        if (!world.regionPasses(
                x,
                y,
                2,
                2,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(
                x,
                y + 2,
                2,
                1,
                [](Tile &tile) { return isSolidBlock(tile.blockID); }) ||
            isLocationUsed(x, y, 25, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        Chest &chest = world.placeChest(x, y, Variant::shadow);
        fillShadowChest(chest, rnd, world);
        --numPlacements;
    }
    // Hellforges.
    numPlacements = locations.size() / 600;
    usedLocations.clear();
    for (; locItr != locations.end() && numPlacements > 0; ++locItr) {
        auto [x, y] = *locItr;
        if (!world.regionPasses(
                x,
                y,
                3,
                2,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(
                x,
                y + 2,
                3,
                1,
                [](Tile &tile) { return isSolidBlock(tile.blockID); }) ||
            isLocationUsed(x, y, 25, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        world.placeFramedTile(x, y, TileID::hellforge);
        --numPlacements;
    }
}
