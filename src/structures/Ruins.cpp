#include "structures/Ruins.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include "structures/data/Furniture.h"
#include "structures/data/RoomWindows.h"
#include <algorithm>
#include <iostream>
#include <set>

inline std::array const roofCurve{1, 2, 3, 5, 8, 13, 21, 26, 29, 31, 32, 33};

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
                if ((tile.blockID != TileID::obsidianBrick &&
                     tile.blockID != TileID::hellstoneBrick) ||
                    tile.actuated) {
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

class Ruins
{
private:
    Random &rnd;
    World &world;
    std::set<Point> queuedPlatforms;
    std::vector<Point> queuedDoors;
    std::vector<Point> locations;
    std::vector<Point> structLocs;

    void buildRoof(int &minX, int &maxX, int &roofLevel)
    {
        int center = std::midpoint(minX, maxX);
        for (size_t j = 0; j < roofCurve.size(); ++j) {
            for (int i = -roofCurve[j]; i < roofCurve[j]; ++i) {
                Tile &tile = world.getTile(center + i, roofLevel + j);
                if (tile.blockID == TileID::lesion ||
                    tile.blockID == TileID::flesh ||
                    tile.blockID == TileID::crystalBlock) {
                    continue;
                }
                tile.blockID = TileID::obsidianBrick;
                if (i == -roofCurve[j]) {
                    tile.slope = j == 0 || j > 2 ? Slope::half : Slope::topLeft;
                } else if (i == roofCurve[j] - 1) {
                    tile.slope =
                        j == 0 || j > 2 ? Slope::half : Slope::topRight;
                }
                if (world.getTile(center + i, roofLevel + j - 1).blockID ==
                    TileID::empty) {
                    tile.wallID = WallID::empty;
                } else {
                    tile.wallID = WallID::Unsafe::obsidianBrick;
                }
                structLocs.emplace_back(center + i, roofLevel + j);
            }
            if (center - roofCurve[j] <= minX) {
                roofLevel += j + 1;
                minX = std::max(minX, center - roofCurve[j] + 1);
                maxX = std::min(maxX, center + roofCurve[j] - 1);
                return;
            }
        }
    }

    void partitionRooms(int minX, int maxX, int minY, int maxY)
    {
        if (maxY - minY > 11) {
            int center = std::midpoint(minY, maxY) + rnd.getInt(-1, 1);
            partitionRooms(minX, maxX, minY, center);
            partitionRooms(minX, maxX, center, maxY);
            return;
        }
        auto isRoomWall = [maxY, this](int x) {
            return static_cast<int>(99999 * (1 + rnd.getFineNoise(x, maxY))) %
                       23 ==
                   0;
        };
        for (int x = minX; x < maxX; ++x) {
            for (int y = minY; y < maxY; ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::lesion ||
                    tile.blockID == TileID::flesh ||
                    tile.blockID == TileID::crystalBlock) {
                    continue;
                }
                tile.blockID =
                    y == minY || x == minX || x == maxX - 1 ||
                            (x > minX + 1 && x < maxX - 2 && isRoomWall(x) &&
                             !isRoomWall(x - 1) && !isRoomWall(x - 2))
                        ? TileID::obsidianBrick
                        : TileID::empty;
                tile.wallID = WallID::Unsafe::obsidianBrick;
                tile.liquid = Liquid::none;
                locations.emplace_back(x, y);
                if (y == maxY - 3 && tile.blockID == TileID::obsidianBrick &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            3 ==
                        0) {
                    queuedDoors.emplace_back(x, y);
                }
            }
        }
        int range = maxX - minX;
        for (auto partition : rnd.partitionRange(
                 std::max(rnd.getInt(range / 18, range / 10), 2),
                 range)) {
            for (int i = -2; i < 2; ++i) {
                queuedPlatforms.emplace(minX + partition + i, minY);
            }
        }
    }

    void buildRuin(int minX, int maxX, int cityBase)
    {
        if (minX == -1) {
            return;
        }
        if (maxX - minX > 66) {
            int center = std::midpoint(minX, maxX);
            buildRuin(minX, center - rnd.getInt(2, 5), cityBase);
            buildRuin(center + rnd.getInt(2, 5), maxX, cityBase);
            return;
        }
        cityBase += rnd.getInt(-2, 8);
        int roofLevel = std::max<int>(
            cityBase - 5 - rnd.getDouble(0.7, 1.3) * (maxX - minX),
            world.getUnderworldLevel() + 50);
        if (cityBase - roofLevel < 12) {
            return;
        }
        buildRoof(minX, maxX, roofLevel);
        partitionRooms(minX, maxX, roofLevel, cityBase);
        for (int x = minX; x < maxX; ++x) {
            Tile &tile = world.getTile(x, cityBase);
            if (tile.blockID == TileID::lesion ||
                tile.blockID == TileID::flesh ||
                tile.blockID == TileID::crystalBlock) {
                continue;
            }
            // Base.
            tile.blockID = TileID::obsidianBrick;
            structLocs.emplace_back(x, cityBase);
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
        }
    }

public:
    Ruins(Random &r, World &w) : rnd(r), world(w) {}

    std::vector<Point> gen()
    {
        int step = 8;
        int underworldHeight = world.getHeight() - world.getUnderworldLevel();
        int cityBase = world.getUnderworldLevel() + 0.47 * underworldHeight;
        int minX = makeCongruent(
            world.conf.dontDigUp ? 100 : 0.15 * world.getWidth(),
            step);
        int maxX = makeCongruent(
            world.conf.dontDigUp ? world.getWidth() - 100
                                 : 0.85 * world.getWidth(),
            step);
        int skipFrom = world.conf.dontDigUp
                           ? makeCongruent(0.39 * world.getWidth(), step)
                           : -1;
        int skipTo = world.conf.dontDigUp
                         ? makeCongruent(0.61 * world.getWidth(), step)
                         : -1;
        int ruinStartX = -1;
        for (int x = minX; x < maxX; ++x) {
            if (x == skipFrom) {
                x = skipTo;
                ruinStartX = -1;
            }
            if (x % step == 0 &&
                (rnd.getCoarseNoise(x, 0) < 0.15 ||
                 !world.regionPasses(
                     x,
                     world.getUnderworldLevel(),
                     step,
                     cityBase - world.getUnderworldLevel(),
                     [](Tile &tile) { return !tile.guarded; }))) {
                buildRuin(ruinStartX, x, cityBase);
                ruinStartX = -1;
                x += step - 1;
            } else if (rnd.getFineNoise(makeCongruent(x, step), 0) < -0.2) {
                buildRuin(ruinStartX, x, cityBase);
                ruinStartX = -1;
            } else if (ruinStartX == -1) {
                ruinStartX = x;
            }
        }
        return locations;
    }

    void applyDamage()
    {
        locations.insert(locations.end(), structLocs.begin(), structLocs.end());
        std::vector<Point> noiseShuffles;
        for (int iter = 0; iter < 4; ++iter) {
            noiseShuffles.emplace_back(
                rnd.getInt(0, world.getWidth()),
                rnd.getInt(0, world.getHeight()));
        }
        auto getCompsiteNoise = [noiseShuffles, this](int x, int y) {
            double noise = 0;
            for (auto [shuffleX, shuffleY] : noiseShuffles) {
                noise = std::max(
                    std::abs(rnd.getFineNoise(x + shuffleX, y + shuffleY)),
                    noise);
            }
            return noise;
        };
        for (auto [x, y] : locations) {
            double noise = getCompsiteNoise(x, y);
            if (noise > 0.65) {
                world.getTile(x, y) = {};
            } else if (noise > 0.55) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::obsidianBrick) {
                    tile.blockID = TileID::hellstoneBrick;
                }
                if (tile.wallID == WallID::Unsafe::obsidianBrick) {
                    tile.wallID = WallID::Unsafe::hellstoneBrick;
                }
            }
        }
        for (auto [x, y] : queuedDoors) {
            if (isSolidBlock(world.getTile(x, y - 1).blockID) &&
                isSolidBlock(world.getTile(x, y + 3).blockID)) {
                world.placeFramedTile(x, y, TileID::door, Variant::obsidian);
            }
        }
        std::erase_if(queuedPlatforms, [this](const auto &pt) {
            return world.getTile(pt.x, pt.y - 1).blockID != TileID::empty ||
                   world.getTile(pt).blockID == TileID::empty ||
                   world.getTile(pt.x, pt.y + 1).blockID != TileID::empty;
        });
        for (auto [x, y] : queuedPlatforms) {
            if (queuedPlatforms.contains({x - 1, y}) ||
                queuedPlatforms.contains({x + 1, y})) {
                placePlatform(x, y, Platform::obsidian, world);
            }
        }
    }
};

void igniteRuins(World &world)
{
    parallelFor(std::views::iota(0, world.getWidth()), [&world](int x) {
        for (int y = world.getUnderworldLevel(); y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            switch (tile.blockID) {
            case TileID::hellstoneBrick:
                tile.blockID = TileID::obsidianBrick;
                break;
            case TileID::obsidianBrick:
                tile.blockID = TileID::hellstoneBrick;
                break;
            }
            switch (tile.wallID) {
            case WallID::Unsafe::hellstoneBrick:
                tile.wallID = WallID::Unsafe::obsidianBrick;
                break;
            case WallID::Unsafe::obsidianBrick:
                tile.wallID = WallID::Unsafe::hellstoneBrick;
                break;
            }
        }
    });
}

void genRuins(Random &rnd, World &world)
{
    std::cout << "Abandoning cities\n";
    Ruins structure(rnd, world);
    std::vector<Point> locations = structure.gen();
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    // Windows.
    int numPlacements = locations.size() / 500;
    auto locItr = locations.begin();
    Data::Window windowStyle = Data::Window::tall;
    TileBuffer data = Data::getWindow(
        windowStyle,
        TileID::obsidianBrick,
        WallID::Safe::redStainedGlass,
        world.getFramedTiles());
    std::vector<Point> usedLocations;
    for (; locItr != locations.end(); ++locItr) {
        auto [x, y] = *locItr;
        y = scanWhileEmpty({x, y}, {0, -1}, world).y;
        if (windowStyle == Data::Window::square) {
            ++y;
        }
        if (!world.regionPasses(
                x - 1,
                windowStyle == Data::Window::tall ? y : y - 1,
                data.getWidth() + 2,
                data.getHeight() + (windowStyle == Data::Window::tall ? 1 : 2),
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            isLocationUsed(x, y, 10, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        for (int i = 0; i < data.getWidth(); ++i) {
            for (int j = 0; j < data.getHeight(); ++j) {
                Tile &dataTile = data.getTile(i, j);
                Tile &tile = world.getTile(x + i, y + j);
                if (dataTile.blockID != TileID::empty) {
                    tile.blockID = dataTile.blockID;
                    tile.slope = dataTile.slope;
                    tile.actuated = dataTile.actuated;
                    tile.guarded = true;
                }
                if (dataTile.wallID != WallID::empty) {
                    tile.wallID = dataTile.wallID;
                }
            }
        }
        --numPlacements;
        if (numPlacements < 0) {
            break;
        }
        windowStyle = rnd.select({Data::Window::square, Data::Window::tall});
        data = Data::getWindow(
            windowStyle,
            TileID::obsidianBrick,
            rnd.select(
                {WallID::Safe::redStainedGlass,
                 WallID::Safe::multicoloredStainedGlass,
                 WallID::Safe::leadFence}),
            world.getFramedTiles());
    }
    structure.applyDamage();
    // Furniture.
    int tries = 0;
    numPlacements = locations.size() / 450;
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
        for (auto [chestX, chestY] :
             world.placeBuffer(x, y, data, Blend::blockOnly)) {
            fillDresser(world.registerStorage(chestX, chestY), rnd);
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
    numPlacements = locations.size() / 650;
    usedLocations.clear();
    for (; locItr != locations.end(); ++locItr) {
        auto [x, y] = *locItr;
        auto [width, height] = world.getPaintingDims(*paintingItr);
        y = scanWhileNotSolid({x, y}, {0, -1}, world).y + 1;
        if (!world.regionPasses(
                x,
                y,
                width,
                height,
                [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           (tile.wallID == WallID::Unsafe::obsidianBrick ||
                            tile.wallID == WallID::Unsafe::hellstoneBrick);
                }) ||
            world.getTile(x, y - 2).actuated ||
            isLocationUsed(x, y, 30, usedLocations)) {
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
    numPlacements = world.conf.chests * locations.size() / 640;
    usedLocations.clear();
    for (; locItr != locations.end() && numPlacements > 0; ++locItr) {
        auto [x, y] = *locItr;
        y = scanWhileNotSolid({x, y}, {0, 1}, world).y - 1;
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
                [](Tile &tile) {
                    return isSolidBlock(tile.blockID) && !tile.actuated;
                }) ||
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
    for (auto rItr = locations.rbegin();
         rItr != locations.rend() && numPlacements > 0;
         ++rItr) {
        auto [x, y] = *rItr;
        y = scanWhileNotSolid({x, y}, {0, 1}, world).y - 1;
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
                [](Tile &tile) {
                    return isSolidBlock(tile.blockID) && !tile.actuated;
                }) ||
            isLocationUsed(x, y, 25, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        world.placeFramedTile(x, y, TileID::hellforge);
        --numPlacements;
    }
    // Pots.
    numPlacements = locations.size() / 600;
    usedLocations.clear();
    for (; locItr != locations.end() && numPlacements > 0; ++locItr) {
        auto [x, y] = *locItr;
        y = scanWhileNotSolid({x, y}, {0, 1}, world).y - 1;
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
                [](Tile &tile) {
                    return isSolidBlock(tile.blockID) && !tile.actuated;
                }) ||
            isLocationUsed(x, y, 5, usedLocations)) {
            continue;
        }
        usedLocations.emplace_back(x, y);
        world.placeFramedTile(x, y, TileID::pot, Variant::underworld);
        --numPlacements;
    }
    if (world.conf.forTheWorthy) {
        igniteRuins(world);
    }
}
