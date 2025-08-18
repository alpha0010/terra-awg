#include "structures/Dungeon.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/ItemID.h"
#include "ids/Paint.h"
#include "ids/Prefix.h"
#include "ids/WallID.h"
#include "map/DiegeticColor.h"
#include "structures/LootRules.h"
#include "structures/Platforms.h"
#include "structures/StructureUtil.h"
#include "structures/data/DungeonRooms.h"
#include "structures/data/Furniture.h"
#include "vendor/frozen/map.h"
#include <algorithm>
#include <iostream>
#include <set>

struct DungeonTheme {
    int brick;
    int crackedBrick;
    int brickWall;
    int slabWall;
    int tiledWall;
    int paint;
    int altPaint;
    Data::Variant furniture;

    void apply(int themeBrick, Random &rnd)
    {
        brick = themeBrick;
        if (themeBrick == TileID::blueBrick) {
            crackedBrick = TileID::crackedBlueBrick;
            brickWall = WallID::Unsafe::blueBrick;
            slabWall = WallID::Unsafe::blueSlab;
            tiledWall = WallID::Unsafe::blueTiled;
            paint = rnd.select(
                {Paint::deepCyan,
                 Paint::deepSkyBlue,
                 Paint::deepBlue,
                 Paint::deepPurple});
            furniture = Data::Variant::blueDungeon;
        } else if (themeBrick == TileID::greenBrick) {
            crackedBrick = TileID::crackedGreenBrick;
            brickWall = WallID::Unsafe::greenBrick;
            slabWall = WallID::Unsafe::greenSlab;
            tiledWall = WallID::Unsafe::greenTiled;
            paint = rnd.select(
                {Paint::deepYellow,
                 Paint::deepLime,
                 Paint::deepGreen,
                 Paint::deepTeal});
            furniture = Data::Variant::greenDungeon;
        } else {
            crackedBrick = TileID::crackedPinkBrick;
            brickWall = WallID::Unsafe::pinkBrick;
            slabWall = WallID::Unsafe::pinkSlab;
            tiledWall = WallID::Unsafe::pinkTiled;
            paint = rnd.select(
                {Paint::deepOrange,
                 Paint::deepRed,
                 Paint::deepPink,
                 Paint::deepViolet});
            furniture = Data::Variant::pinkDungeon;
        }
        altPaint = paint;
    }
};

class Dungeon
{
private:
    Random &rnd;
    World &world;
    int roomSize;
    int hallSize;
    int wallThickness;
    DungeonTheme theme;
    std::vector<Point> requestedDoors;

    Point selectSize(int dungeonCenter)
    {
        double dungeonSize = world.conf.dungeonSize;
        double initialW = rnd.getDouble(0.029, 0.034) * world.getWidth();
        double initialH = rnd.getDouble(0.35, 0.4) * world.getHeight();
        double targetSize = dungeonSize * initialW * initialH;
        int dungeonHeight = std::max(
            std::min(
                (dungeonSize < 1 ? std::lerp(
                                       std::sqrt(dungeonSize),
                                       dungeonSize,
                                       dungeonSize)
                                 : dungeonSize) *
                    initialH,
                0.4 * world.getHeight()),
            2.5 * roomSize);
        int maxWidth =
            std::min(dungeonCenter, world.getWidth() - dungeonCenter) - 100;
        return {
            std::min<int>(targetSize / dungeonHeight, maxWidth),
            dungeonHeight};
    }

    void sortInitialZones(
        int dungeonCenter,
        int dungeonWidth,
        std::vector<Point> &zones)
    {
        int thresholdY = zones.front().y + roomSize;
        auto endItr = zones.begin();
        while (endItr != zones.end() && endItr->y <= thresholdY) {
            ++endItr;
        }
        std::sort(zones.begin(), endItr);
        int thresholdX = dungeonCenter - dungeonWidth / 5;
        endItr = zones.begin();
        while (endItr != zones.end() && endItr->x < thresholdX &&
               endItr->y <= thresholdY) {
            ++endItr;
        }
        if (endItr != zones.begin()) {
            std::reverse(zones.begin(), endItr);
        }
    }

    void drawRect(
        Point topLeft,
        Point bottomRight,
        double skewX,
        double skewY,
        bool filled)
    {
        auto [width, height] = bottomRight - topLeft;
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                Tile &tile = world.getTile(
                    topLeft.x + i + skewX * j,
                    topLeft.y + j + skewY * i);
                tile.blockPaint = Paint::none;
                if (i < wallThickness || j < wallThickness ||
                    width - i <= wallThickness || height - j <= wallThickness) {
                    if (tile.wallID != theme.brickWall) {
                        tile.blockID = theme.brick;
                    }
                } else {
                    if (filled && (tile.wallID != theme.brickWall ||
                                   tile.blockID == theme.brick ||
                                   tile.blockID == theme.crackedBrick)) {
                        tile.blockID = theme.crackedBrick;
                    } else {
                        tile.blockID = TileID::empty;
                    }
                }
                tile.wallID = theme.brickWall;
                if (tile.liquid == Liquid::water ||
                    tile.liquid == Liquid::lava) {
                    tile.liquid = Liquid::none;
                }
            }
        }
    }

    void findConnectedZones(
        int x,
        int y,
        std::set<Point> &connectedZones,
        const std::set<Point> &allZones)
    {
        if (connectedZones.contains({x, y})) {
            return;
        }
        connectedZones.insert({x, y});
        for (auto [i, j] :
             {Point{-roomSize, 0},
              {roomSize, 0},
              {0, -roomSize},
              {0, roomSize}}) {
            if (allZones.contains({x + i, y + j})) {
                findConnectedZones(x + i, y + j, connectedZones, allZones);
            }
        }
    }

    Point getClosestPoint(int x, int y, const std::set<Point> &points)
    {
        Point closest = *points.begin();
        double minDist = hypot(closest, {x, y});
        for (Point pt : points) {
            double curDist = hypot(pt, {x, y});
            if (curDist < minDist) {
                minDist = curDist;
                closest = pt;
            }
        }
        return closest;
    }

    void makeHallway(Point from, Point to)
    {
        int deltaX = std::abs(to.x - from.x);
        int deltaY = std::abs(to.y - from.y);
        bool filled = std::min(from.y, to.y) > world.getCavernLevel() &&
                      rnd.getInt(0, 4) == 0;
        if (deltaY == 0 && !filled) {
            requestedDoors.emplace_back(
                std::min(from.x, to.x) + 1 + roomSize / 2,
                from.y - hallSize / 2);
            requestedDoors.emplace_back(
                std::max(from.x, to.x) - 2 - roomSize / 2,
                from.y - hallSize / 2);
        }
        if (deltaY > deltaX) {
            if (from.y > to.y) {
                std::swap(from, to);
            }
            drawRect(
                {from.x - hallSize, from.y - hallSize},
                {from.x + hallSize, to.y + hallSize},
                (to.x - from.x) / (2.0 * hallSize + deltaY),
                0,
                filled);
        } else {
            if (from.x > to.x) {
                std::swap(from, to);
            }
            drawRect(
                {from.x - hallSize, from.y - hallSize},
                {to.x + hallSize, from.y + hallSize},
                0,
                (to.y - from.y) / (2.0 * hallSize + deltaX),
                filled);
        }
    }

    void makeMapRoom(std::vector<Point> &zones)
    {
        int minX = world.getWidth();
        int maxX = 0;
        int maxY = 0;
        for (auto [x, y] : zones) {
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
        int padded = roomSize + 2 * wallThickness;
        minX += padded / 2;
        maxX -= padded / 2;
        maxY += padded / 2;
        int x =
            minX + roomSize * static_cast<int>(0.2 * (maxX - minX) / roomSize);
        int y = maxY + roomSize;
        drawRect({x, y}, {x + padded, y + padded}, 0, 0, false);
        zones.emplace_back(x + padded / 2, y + padded / 2);
        int scale = 80;
        int mapWidth = world.getWidth() / scale;
        int mapHeight = world.getHeight() / scale;
        x += 2 * roomSize;
        drawRect(
            {x, y},
            {x + mapWidth + 2 * wallThickness,
             y + mapHeight + 2 * wallThickness},
            0,
            0,
            false);
        makeHallway(
            {x - 2 * roomSize + padded / 2, y + padded / 2},
            {x + padded / 2, y + padded / 2});
        x += wallThickness;
        y += wallThickness;
        for (int i = 0; i < mapWidth; ++i) {
            for (int j = 0; j < mapHeight; ++j) {
                world.getTile(x + i, y + j).blockID = theme.brick;
            }
        }
        world.queuedDeco.emplace_back(
            [x, y, mapWidth, mapHeight, scale](Random &rnd, World &world) {
                parallelFor(
                    std::views::iota(0, mapWidth),
                    [x, y, mapHeight, scale, &world](int i) {
                        for (int j = 0; j < mapHeight; ++j) {
                            Tile &tile = world.getTile(x + i, y + j);
                            tile.blockID = getSectorColor(i, j, scale, world);
                            tile.blockPaint = Paint::none;
                            if (tile.blockID == TileID::torch) {
                                tile.frameX = 66;
                                tile.frameY = 286;
                            } else {
                                tile.actuated = true;
                            }
                        }
                    });
                if (world.conf.forTheWorthy) {
                    std::vector<Tile> mapTiles;
                    for (int j = 0; j < mapHeight; ++j) {
                        for (int i = 0; i < mapWidth; ++i) {
                            mapTiles.emplace_back(world.getTile(x + i, y + j));
                        }
                    }
                    int chunk = rnd.getInt(mapWidth * 3, mapWidth * 5);
                    for (size_t start = 0; start < mapTiles.size();
                         start += chunk) {
                        std::shuffle(
                            mapTiles.begin() + start,
                            mapTiles.begin() +
                                std::min(start + chunk, mapTiles.size()),
                            rnd.getPRNG());
                    }
                    for (int i = 0; i < mapWidth; ++i) {
                        for (int j = 0; j < mapHeight; ++j) {
                            if (rnd.getDouble(0, 1) < 0.15) {
                                world.getTile(x + i, y + j) =
                                    mapTiles[mapHeight + j * mapWidth];
                            }
                        }
                    }
                }
            });
    }

    bool isValidPlacementLocation(
        int x,
        int y,
        int width,
        int height,
        bool needsFloor)
    {
        if (needsFloor) {
            for (int i = 0; i < width; ++i) {
                if (world.getTile(x + i, y + 1).blockID != theme.brick) {
                    return false;
                }
            }
        }
        return world
            .regionPasses(x, y + 1 - height, width, height, [this](Tile &tile) {
                return tile.blockID == TileID::empty &&
                       tile.wallID == theme.brickWall &&
                       tile.liquid == Liquid::none;
            });
    }

    Point selectBiomeChestLocation(const std::vector<Point> &zones)
    {
        for (int tries = 0; tries < 1000; ++tries) {
            auto [x, y] = rnd.select(zones);
            x += rnd.getInt(-6, 2);
            y = scanWhileEmpty({x, y}, {0, 1}, world).y;
            if (isValidPlacementLocation(x, y, 6, 5, true)) {
                return {x, y};
            }
        }
        return {-1, -1};
    }

    void addBiomeChests(const std::vector<Point> &zones)
    {
        for (auto [chestType, lampType, platformBlock, dye, item] :
             {std::tuple{
                  Variant::jungle,
                  Variant::dynasty,
                  TileID::mudstoneBrick,
                  ItemID::chlorophyteDye,
                  Item{ItemID::piranhaGun, rnd.select(PrefixSet::ranged), 1}},
              {Variant::corruption,
               Variant::lesion,
               TileID::ebonstoneBrick,
               ItemID::reflectiveObsidianDye,
               {ItemID::scourgeOfTheCorruptor,
                rnd.select(PrefixSet::common),
                1}},
              {Variant::crimson,
               Variant::flesh,
               TileID::crimstoneBrick,
               ItemID::bloodbathDye,
               {ItemID::vampireKnives, rnd.select(PrefixSet::common), 1}},
              {Variant::hallowed,
               Variant::crystal,
               TileID::pearlstoneBrick,
               ItemID::rainbowDye,
               {ItemID::rainbowGun, rnd.select(PrefixSet::magic), 1}},
              {Variant::ice,
               Variant::frozen,
               TileID::iceBrick,
               ItemID::hadesDye,
               {ItemID::staffOfTheFrostHydra, rnd.select(PrefixSet::magic), 1}},
              {Variant::desert,
               Variant::sandstone,
               TileID::sandstoneBrick,
               ItemID::shiftingSandsDye,
               {ItemID::desertTigerStaff, rnd.select(PrefixSet::magic), 1}}}) {
            auto [x, y] = selectBiomeChestLocation(zones);
            if (x == -1) {
                continue;
            }
            for (int i = 0; i < 6; ++i) {
                world.getTile(x + i, y).blockID = platformBlock;
            }
            for (int i = 1; i < 5; ++i) {
                world.getTile(x + i, y - 1).blockID = platformBlock;
            }
            Chest &chest = world.placeChest(x + 2, y - 3, chestType);
            fillDungeonBiomeChest(chest, rnd, world, dye, std::move(item));
            world.placeFramedTile(x, y - 3, TileID::lamp, lampType);
            world.placeFramedTile(x + 5, y - 3, TileID::lamp, lampType);
        }
    }

    Point selectPlatformLocation(const std::vector<Point> &zones)
    {
        for (int tries = 0; tries < 100; ++tries) {
            auto [x, y] = rnd.select(zones);
            if (isValidPlacementLocation(x, y, 1, roomSize, false)) {
                y -= rnd.getInt(4, roomSize - 4);
                return {x, y};
            }
        }
        return {-1, -1};
    }

    void addPlatforms(const std::vector<Point> &zones)
    {
        std::set<int> usedRows;
        int maxPlatforms =
            (world.conf.dungeonSize > 1 ? 0.8 * world.conf.dungeonSize + 0.2
                                        : 1) *
            world.getHeight() / 40;
        for (int i = 0; i < maxPlatforms; ++i) {
            auto [centerX, centerY] = selectPlatformLocation(zones);
            if (centerX == -1 || usedRows.contains(centerY / 7)) {
                continue;
            }
            usedRows.insert(centerY / 7);
            int fromX = centerX - 4;
            int toX = centerX + 4;
            while (isValidPlacementLocation(
                fromX - 2,
                centerY + 5,
                2,
                10,
                false)) {
                fromX -= 2;
            }
            while (isValidPlacementLocation(toX, centerY + 5, 2, 10, false)) {
                toX += 2;
            }
            if (toX - fromX < roomSize) {
                continue;
            }
            auto partitions = rnd.partitionRange(rnd.getInt(2, 4), toX - fromX);
            auto partItr = partitions.begin();
            int drawTile = rnd.select(
                {theme.brick,
                 rnd.getInt(0, 9) == 0 ? theme.crackedBrick : TileID::empty});
            for (int x = fromX; x < toX; ++x) {
                if (partItr != partitions.end() && x - fromX == *partItr) {
                    ++partItr;
                    if (drawTile == theme.brick) {
                        drawTile = rnd.getInt(0, 9) == 0 ? theme.crackedBrick
                                                         : TileID::empty;
                    } else {
                        drawTile = theme.brick;
                    }
                }
                for (int y = centerY; y < centerY + 3; ++y) {
                    world.getTile(x, y).blockID = drawTile;
                }
            }
        }
    }

    void addDoors()
    {
        for (auto [x, y] : requestedDoors) {
            int floor = 0;
            for (int j = 0; j < hallSize + 1; ++j) {
                Tile &tile = world.getTile(x, y + j);
                if (tile.blockID != TileID::empty) {
                    if (tile.blockID == theme.brick) {
                        floor = j;
                    }
                    break;
                }
            }
            for (int j = 0; j < floor; ++j) {
                if (j < floor - 3) {
                    for (int i = -1; i < 2; ++i) {
                        Tile &tile = world.getTile(x + i, y + j);
                        tile.blockID = theme.brick;
                        if (j == floor - 4) {
                            if (i == -1) {
                                tile.slope = Slope::bottomLeft;
                            } else if (i == 1) {
                                tile.slope = Slope::bottomRight;
                            }
                        }
                    }
                } else if (j == floor - 3) {
                    world.placeFramedTile(
                        x,
                        y + j,
                        TileID::door,
                        Variant::dungeon);
                }
            }
        }
    }

    void addShelves(int dungeonCenter, int dungeonWidth)
    {
        rnd.shuffleNoise();
        for (int x = dungeonCenter - dungeonWidth;
             x < dungeonCenter + dungeonWidth + 2 * roomSize;
             ++x) {
            for (int y = world.getUndergroundLevel();
                 y < world.getUnderworldLevel();
                 ++y) {
                if (world.getTile(x, y).blockID != theme.brick ||
                    rnd.getCoarseNoise(x, y) < 0.15 ||
                    rnd.getDouble(0, 1) < 0.8) {
                    continue;
                }
                int shelfStyle;
                switch ((y / 40) % 4) {
                case 0:
                    shelfStyle = Platform::metalShelf;
                    break;
                case 1:
                    shelfStyle = Platform::brassShelf;
                    break;
                case 2:
                    shelfStyle = Platform::woodShelf;
                    break;
                default:
                    shelfStyle = Platform::dungeonShelf;
                    break;
                }
                if (isValidPlacementLocation(x + 1, y + 3, 5, 7, false)) {
                    for (int i = 0; i < 3; ++i) {
                        placePlatform(x + 1 + i, y, shelfStyle, world);
                        placeShelfItem(x + 1 + i, y - 1);
                    }
                } else if (isValidPlacementLocation(
                               x - 5,
                               y + 3,
                               5,
                               7,
                               false)) {
                    for (int i = 0; i < 3; ++i) {
                        placePlatform(x - 3 + i, y, shelfStyle, world);
                        placeShelfItem(x - 3 + i, y - 1);
                    }
                }
            }
        }
    }

    void placeShelfItem(int x, int y)
    {
        switch (rnd.getInt(0, 15)) {
        case 0:
        case 1:
            return;
        case 2:
            world.placeFramedTile(x, y, TileID::bottle, Variant::health);
            return;
        case 3:
            world.placeFramedTile(x, y, TileID::bottle, Variant::mana);
            return;
        case 4:
            world.getTile(x, y).blockID = TileID::waterCandle;
            return;
        }
        Tile &tile = world.getTile(x, y);
        tile.blockID = TileID::book;
        if (rnd.getInt(0, 40) == 0) {
            tile.frameX = 90; // Water Bolt.
        } else {
            tile.frameX = 18 * rnd.getInt(0, 4);
        }
    }

    void addSpikes(const std::vector<Point> &zones)
    {
        std::set<int> attachTiles{theme.brick, theme.crackedBrick};
        for (int numPatches = 0.7 * zones.size(); numPatches > 0;
             --numPatches) {
            Point delta = rnd.select({Point{1, 0}, {-1, 0}, {0, 1}, {0, -1}});
            Point wall = scanWhileEmpty(rnd.select(zones), delta, world);
            Point incr = delta.x == 0 ? Point{1, 0} : Point{0, 1};
            int patchSize = rnd.getInt(0.15 * roomSize, 0.6 * roomSize);
            Point patchIncr{patchSize * incr.x, patchSize * incr.y};
            Point minPos = wall - patchIncr;
            Point maxPos = wall + patchIncr;
            // Scan back for patch start.
            for (; wall > minPos &&
                   attachTiles.contains(world.getTile(wall + delta).blockID);
                 wall -= incr) {
                if (world.getTile(wall).blockID != TileID::empty) {
                    break;
                }
            }
            // Apply spike patch.
            for (wall += incr;
                 wall < maxPos &&
                 attachTiles.contains(world.getTile(wall + delta).blockID);
                 wall += incr) {
                Tile &tile = world.getTile(wall);
                if (tile.blockID != TileID::empty) {
                    break;
                }
                tile.blockID = TileID::spike;
                if ((wall.x + wall.y) % 2 == 0) {
                    Tile &spikeTile = world.getTile(wall - delta);
                    if (spikeTile.blockID == TileID::empty) {
                        spikeTile.blockID = TileID::spike;
                    }
                }
            }
        }
    }

    void addFloatingSpikes(int dungeonCenter, int dungeonWidth)
    {
        int numSpikes =
            dungeonWidth * world.getHeight() / rnd.getInt(10080, 15120);
        std::vector<Point> usedLocations;
        for (int iter = 0; iter < numSpikes; ++iter) {
            auto [x, y] = selectPaintingLocation(
                dungeonCenter,
                dungeonWidth,
                7,
                7,
                40,
                usedLocations);
            if (x == -1) {
                continue;
            }
            usedLocations.emplace_back(x, y);
            x += 3;
            y -= 5;
            bool asCross = rnd.getBool();
            for (int i = 0; i < 5; ++i) {
                for (int j = 0; j < 5; ++j) {
                    if (asCross ? i == 2 || j == 2
                                : std::abs(i - 2) == std::abs(j - 2)) {
                        world.getTile(x + i, y + j).blockID = TileID::spike;
                    }
                }
            }
        }
    }

    void addDartTraps(const std::vector<Point> &locations)
    {
        int numTraps = (world.conf.traps > 1 ? 1 + world.conf.traps / 25
                                             : world.conf.traps) *
                       locations.size() / rnd.getDouble(124.68, 187.03);
        while (numTraps > 0) {
            auto [x, y] = rnd.select(locations);
            if (!isValidPlacementLocation(x, y, 1, 4, true)) {
                continue;
            }
            std::vector<Point> traps;
            for (int j = 0; j < 4; ++j) {
                for (int dir : {-1, 1}) {
                    Point trap = scanWhileEmpty({x, y - j}, {dir, 0}, world);
                    double dist = hypot(trap, {x, y});
                    if (dist > 3 && dist < 40 &&
                        (world.getTile(trap.x, trap.y + 1).blockID ==
                             theme.brick ||
                         world.getTile(trap.x + dir, trap.y).blockID ==
                             theme.brick)) {
                        traps.push_back(trap);
                    }
                }
            }
            if (traps.empty()) {
                continue;
            }
            std::shuffle(traps.begin(), traps.end(), rnd.getPRNG());
            traps.resize(rnd.getInt(1, std::min<int>(traps.size(), 3)));
            for (auto trap : traps) {
                placeWire(trap, {x, y}, Wire::red, world);
                world.placeFramedTile(
                    trap.x,
                    trap.y,
                    TileID::trap,
                    trap.x > x ? Variant::dartLeft : Variant::dartRight);
            }
            Tile &tile = world.getTile(x, y);
            tile.blockID = TileID::pressurePlate;
            tile.frameY = 36;
            --numTraps;
        }
    }

    void addBoulderTraps(int dungeonCenter, int dungeonWidth)
    {
        int numBoulders =
            (world.conf.forTheWorthy ? std::max(15.0, world.conf.traps)
                                     : world.conf.traps) *
            dungeonWidth * world.getHeight() / 2222640;
        if (numBoulders <= 0 ||
            (world.conf.traps < 2 && !world.conf.forTheWorthy)) {
            return;
        }
        std::vector<Point> locations;
        for (int x = dungeonCenter - dungeonWidth;
             x < dungeonCenter + dungeonWidth + 2 * roomSize;
             ++x) {
            for (int y = world.getUndergroundLevel();
                 y < world.getUnderworldLevel();
                 ++y) {
                if (world.regionPasses(x - 2, y, 6, 4, [this](Tile &tile) {
                        return tile.blockID == theme.crackedBrick;
                    })) {
                    locations.emplace_back(x, y);
                }
            }
        }
        std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
        for (auto [x, y] : locations) {
            if (!world.regionPasses(x - 3, y - 4, 8, 10, [this](Tile &tile) {
                    return tile.blockID == theme.crackedBrick ||
                           tile.blockID == theme.brick;
                })) {
                continue;
            }
            world.placeFramedTile(x, y, TileID::bouncyBoulder);
            --numBoulders;
            if (numBoulders <= 0) {
                break;
            }
        }
    }

    void addFunctionalFurniture(int dungeonCenter, int dungeonWidth)
    {
        std::vector<Point> locations;
        for (int x = dungeonCenter - dungeonWidth;
             x < dungeonCenter + dungeonWidth + 2 * roomSize;
             ++x) {
            for (int y = world.getUndergroundLevel();
                 y < world.getUnderworldLevel();
                 ++y) {
                if (isValidPlacementLocation(x, y, 3, 3, true)) {
                    locations.emplace_back(x, y);
                }
            }
        }
        if (locations.empty()) {
            return;
        }
        addDartTraps(locations);
        for (auto [furnitureTile, lanternStyle] :
             {std::pair{TileID::alchemyTable, Variant::alchemy},
              {TileID::bewitchingTable, Variant::oilRagSconce},
              {TileID::boneWelder, Variant::bone}}) {
            int numPlacements = std::round(
                rnd.getDouble(0.5, 3.2) + locations.size() / 2737.56);
            for (int tries = 500 * numPlacements;
                 numPlacements > 0 && tries > 0;
                 --tries) {
                auto [x, y] = rnd.select(locations);
                if (isValidPlacementLocation(x, y, 3, 3, true)) {
                    world.placeFramedTile(x, y - 2, furnitureTile);
                    addLanternsFor(x, y, lanternStyle);
                    --numPlacements;
                }
            }
        }
        int numChests = world.conf.chests * locations.size() / 256.65;
        while (numChests > 0) {
            auto [x, y] = rnd.select(locations);
            if (isValidPlacementLocation(x, y, 2, 3, true)) {
                Chest &chest = world.placeChest(x, y - 1, Variant::goldLocked);
                fillDungeonChest(chest, rnd, world);
                --numChests;
            }
        }
    }

    void addLanternsFor(int x, int y, Variant lanternStyle)
    {
        int ceiling = scanWhileNotSolid({x, y}, {0, -1}, world).y;
        if (y - ceiling > 16 ||
            !world.regionPasses(
                x - 1,
                ceiling,
                5,
                2,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) ||
            !world.regionPasses(x - 1, ceiling - 1, 5, 1, [this](Tile &tile) {
                return tile.blockID == theme.brick;
            })) {
            return;
        }
        if (world.regionPasses(x - 1, ceiling + 2, 5, 5, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            TileBuffer hangingLantern =
                Data::getChainLantern(lanternStyle, world.getFramedTiles());
            world.placeBuffer(x - 1, ceiling, hangingLantern, Blend::blockOnly);
            world.placeBuffer(x + 3, ceiling, hangingLantern, Blend::blockOnly);
        } else {
            world
                .placeFramedTile(x - 1, ceiling, TileID::lantern, lanternStyle);
            world
                .placeFramedTile(x + 3, ceiling, TileID::lantern, lanternStyle);
        }
    }

    Point selectPaintingLocation(
        int dungeonCenter,
        int dungeonWidth,
        int width,
        int height,
        int radius,
        const std::vector<Point> &usedLocations)
    {
        for (int tries = 0; tries < 500; ++tries) {
            int x = rnd.getInt(
                dungeonCenter - dungeonWidth,
                dungeonCenter + dungeonWidth + 2 * roomSize);
            int y = rnd.getInt(
                world.getUndergroundLevel(),
                world.getUnderworldLevel());
            if (isValidPlacementLocation(x, y, width + 4, height + 4, false) &&
                !isLocationUsed(x, y, radius, usedLocations)) {
                return {x, y};
            }
        }
        return {-1, -1};
    }

    void addPaintings(int dungeonCenter, int dungeonWidth)
    {
        int numPaintings =
            dungeonWidth * world.getHeight() / rnd.getInt(20160, 30240);
        std::array dungeonPaintings{
            Painting::bloodMoonRising,
            Painting::boneWarp,
            Painting::gloryOfTheFire,
            Painting::skellingtonJSkellingsworth,
            Painting::theCursedMan,
            Painting::theGuardiansGaze,
            Painting::theHangedMan,
            Painting::dryadisque,
            Painting::facingTheCerebralMastermind,
            Painting::goblinsPlayingPoker,
            Painting::greatWave,
            Painting::impact,
            Painting::poweredByBirds,
            Painting::somethingEvilIsWatchingYou,
            Painting::sparky,
            Painting::starryNight,
            Painting::theCreationOfTheGuide,
            Painting::theDestroyer,
            Painting::theEyeSeesTheEnd,
            Painting::thePersistencyOfEyes,
            Painting::theScreamer,
            Painting::theTwinsHaveAwoken,
            Painting::trioSuperHeroes,
            Painting::unicornCrossingTheHallows,
        };
        std::shuffle(
            dungeonPaintings.begin(),
            dungeonPaintings.end(),
            rnd.getPRNG());
        std::vector<Point> usedLocations;
        for (int i = 0; i < numPaintings; ++i) {
            Painting curPainting = rnd.pool(dungeonPaintings);
            auto [width, height] = world.getPaintingDims(curPainting);
            auto [x, y] = selectPaintingLocation(
                dungeonCenter,
                dungeonWidth,
                width,
                height,
                40,
                usedLocations);
            if (x != -1) {
                usedLocations.emplace_back(x, y);
                world.placePainting(x + 2, y - height - 1, curPainting);
            }
        }
        numPaintings *= 2;
        for (int i = 0; i < numPaintings; ++i) {
            Painting curPainting =
                rnd.getInt(0, 3) != 0
                    ? rnd.select(
                          {Painting::hangingSkeleton, Painting::wallSkeleton})
                    : rnd.select(
                          {Painting::catacomb1,
                           Painting::catacomb2,
                           Painting::catacomb3,
                           Painting::catacomb4,
                           Painting::catacomb5,
                           Painting::catacomb6,
                           Painting::catacomb7,
                           Painting::catacomb8,
                           Painting::catacomb9});
            auto [width, height] = world.getPaintingDims(curPainting);
            auto [x, y] = selectPaintingLocation(
                dungeonCenter,
                dungeonWidth,
                width,
                height,
                25,
                usedLocations);
            if (x != -1) {
                usedLocations.emplace_back(x, y);
                world.placePainting(x + 2, y - height - 1, curPainting);
            }
        }
    }

    bool canPlaceFurniture(int x, int y, TileBuffer &data)
    {
        for (int i = 0; i < data.getWidth(); ++i) {
            for (int j = 0; j < data.getHeight(); ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.wallID != theme.brickWall) {
                    return false;
                }
                switch (data.getTile(i, j).blockID) {
                case TileID::empty:
                    if (isSolidBlock(tile.blockID)) {
                        return false;
                    }
                    break;
                case TileID::cloud:
                    if (tile.blockID != theme.brick &&
                        tile.blockID != theme.crackedBrick) {
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

    Point selectFurnitureLocation(
        int dungeonCenter,
        int dungeonWidth,
        TileBuffer &data,
        const std::vector<Point> &usedLocations)
    {
        bool anchorTop =
            data.getWidth() > 1 && data.getTile(1, 0).blockID == TileID::cloud;
        bool anchorBot =
            data.getWidth() > 1 &&
            data.getTile(1, data.getHeight() - 1).blockID == TileID::cloud;
        int anchorLeftAt = -1;
        int anchorRightAt = -1;
        for (int j = 1; j < data.getHeight() - 1; ++j) {
            if (data.getTile(0, j).blockID == TileID::cloud) {
                anchorLeftAt = j;
            } else if (
                data.getTile(data.getWidth() - 1, j).blockID == TileID::cloud) {
                anchorRightAt = j;
            }
        }
        for (int tries = 0; tries < 10000; ++tries) {
            int x = rnd.getInt(
                dungeonCenter - dungeonWidth,
                dungeonCenter + dungeonWidth + 2 * roomSize);
            int y = rnd.getInt(
                world.getUndergroundLevel(),
                world.getUnderworldLevel());
            if (world.getTile(x, y).wallID != theme.brickWall) {
                continue;
            }
            if (anchorTop && world.getTile(x + 1, y).blockID == TileID::empty) {
                y = scanWhileNotSolid({x + 1, y}, {0, -1}, world).y - 1;
            } else if (
                anchorBot &&
                world.getTile(x + 1, y + data.getHeight() - 1).blockID ==
                    TileID::empty) {
                y = scanWhileNotSolid(
                        {x + 1, y + data.getHeight() - 1},
                        {0, 1},
                        world)
                        .y -
                    data.getHeight() + 2;
            }
            if (anchorLeftAt != -1 &&
                world.getTile(x, y + anchorLeftAt).blockID == TileID::empty) {
                x = scanWhileNotSolid({x, y + anchorLeftAt}, {-1, 0}, world).x -
                    1;
            } else if (
                anchorRightAt != -1 &&
                world.getTile(x + data.getWidth() - 1, y + anchorRightAt)
                        .blockID == TileID::empty) {
                x = scanWhileNotSolid(
                        {x + data.getWidth() - 1, y + anchorRightAt},
                        {1, 0},
                        world)
                        .x -
                    data.getWidth() + 2;
            }
            if (canPlaceFurniture(x, y, data) &&
                !isLocationUsed(x, y, 12, usedLocations)) {
                return {x, y};
            }
        }
        return {-1, -1};
    }

    void addFurniture(int dungeonCenter, int dungeonWidth)
    {
        double numPlacements =
            dungeonWidth * world.getHeight() / rnd.getInt(4536, 6048);
        std::vector<Point> usedLocations;
        while (numPlacements > 0) {
            TileBuffer data = getFurniture(
                rnd.select(Data::furnitureLayouts),
                theme.furniture,
                world.getFramedTiles());
            auto [x, y] = selectFurnitureLocation(
                dungeonCenter,
                dungeonWidth,
                data,
                usedLocations);
            if (x == -1) {
                numPlacements -= 0.1;
                continue;
            }
            usedLocations.emplace_back(x, y);
            for (auto [chestX, chestY] :
                 world.placeBuffer(x, y, data, Blend::blockOnly)) {
                fillDresser(world.registerStorage(chestX, chestY), rnd);
            }
            --numPlacements;
        }
    }

    void selectEntry(int dungeonCenter)
    {
        int scanDist = 0.1 * world.getWidth();
        world.dungeon = {dungeonCenter, world.getSurfaceLevel(dungeonCenter)};
        int offset = world.dungeon.x < world.getWidth() / 2 ? 23 : -23;
        for (int swapI = 0; swapI < scanDist; ++swapI) {
            int i = swapI / 2;
            if (swapI % 2 == 0) {
                i = -i;
            }
            int surfaceLevel = std::midpoint(
                world.getSurfaceLevel(dungeonCenter + i - 8),
                world.getSurfaceLevel(dungeonCenter + i + 8));
            if (world.regionPasses(
                    dungeonCenter + i + offset - 45,
                    surfaceLevel - 30,
                    90,
                    30,
                    [](Tile &tile) {
                        return tile.blockID != TileID::livingWood &&
                               tile.blockID != TileID::leaf &&
                               tile.blockID != TileID::aetherium &&
                               tile.wallID != WallID::Unsafe::livingWood;
                    })) {
                world.dungeon.x = dungeonCenter + i;
                break;
            }
        }
        std::vector<int> surface;
        for (int i = -30; i < 30; ++i) {
            surface.push_back(
                world.getSurfaceLevel(world.dungeon.x + i + offset));
        }
        std::sort(surface.begin(), surface.end());
        world.dungeon.y = surface[0.15 * surface.size()];
    }

    void makeEntryHallway(Point entry, Point room)
    {
        if (2 * std::abs(entry.x - room.x) < std::abs(entry.y - room.y)) {
            makeHallway(entry, room);
            return;
        }
        Point mid{(2 * entry.x + room.x) / 3, (entry.y + 2 * room.y) / 3};
        Point offset{
            roomSize / 2 + wallThickness,
            roomSize / 2 + wallThickness};
        drawRect(mid - offset, mid + offset, 0, 0, false);
        makeHallway(entry, mid);
        makeHallway(mid, room);
    }

    void makeEntry()
    {
        bool isOnLeft = world.dungeon.x < world.getWidth() / 2;
        TileBuffer entry = Data::getDungeonRoom(
            isOnLeft ? Data::Room::entranceRight : Data::Room::entranceLeft,
            theme.furniture,
            world.getFramedTiles());
        auto [x, y] = world.dungeon;
        int minX = world.getWidth();
        int maxX = 0;
        for (int i = -20; i < 20; ++i) {
            if (world.getTile(x + i, y).blockID == theme.brick) {
                minX = std::min(minX, x + i);
                maxX = std::max(maxX, x + i);
            }
        }
        x = std::midpoint(minX, maxX);
        bool foundAnchor = false;
        for (int i = 0; i < entry.getWidth() && !foundAnchor; ++i) {
            for (int j = 0; j < entry.getHeight(); ++j) {
                Tile &tile = entry.getTile(i, j);
                if (tile.blockID == TileID::cloud &&
                    tile.blockPaint == Paint::purple) {
                    x -= i;
                    y -= j;
                    foundAnchor = true;
                    break;
                }
            }
        }
        for (int i = 0; i < entry.getWidth(); ++i) {
            bool foundSolidTile = false;
            for (int j = 0; j < entry.getHeight(); ++j) {
                Tile &entryTile = entry.getTile(i, j);
                Tile &tile = world.getTile(x + i, y + j);
                if (entryTile.blockID == TileID::cloud) {
                    if (entryTile.blockPaint == Paint::red) {
                        world.dungeon = {x + i, y + j};
                    }
                    if (entryTile.blockPaint == Paint::none) {
                        entryTile.blockID = tile.blockID;
                    } else {
                        entryTile.blockID = TileID::empty;
                        entryTile.blockPaint = Paint::none;
                    }
                }
                if (tile.wallID == theme.brickWall &&
                    tile.blockID == TileID::empty &&
                    entryTile.blockID == theme.brick) {
                    continue;
                }
                double noise = rnd.getFineNoise(x + 2 * i, y + j);
                double threshold = 1.9 * j / entry.getHeight() - 0.56;
                if (noise > threshold + 0.15) {
                    if (entryTile.blockID != TileID::empty &&
                        !world.getFramedTiles()[entryTile.blockID]) {
                        entryTile.blockID = TileID::empty;
                    }
                    entryTile.wallID = WallID::empty;
                } else if (noise > threshold) {
                    if (entryTile.blockID != TileID::empty &&
                        !world.getFramedTiles()[entryTile.blockID]) {
                        entryTile.blockID = TileID::leaf;
                    }
                    if (entryTile.wallID != WallID::empty) {
                        entryTile.wallID = WallID::Safe::livingLeaf;
                    }
                }
                if (entryTile.blockID == TileID::empty &&
                    entryTile.wallID == WallID::empty) {
                    double halfI = 0.5 * entry.getWidth();
                    double halfJ = 0.5 * entry.getHeight();
                    if (j < halfJ &&
                        std::hypot((i - halfI) / halfI, (j - halfJ) / halfJ) >
                            1.1) {
                        continue;
                    }
                }
                int existingTileId = tile.blockID;
                tile = entryTile;
                if (tile.wallID != WallID::empty &&
                    std::abs(rnd.getFineNoise(x + 2 * i, y + 2 * j)) <
                        0.1 - 0.1 * j / entry.getHeight()) {
                    if (shouldPaint()) {
                        tile.wallID = WallID::Safe::mudstoneBrick;
                        tile.wallPaint = selectWallPaintAt(x + i, y + j);
                    } else {
                        tile.wallID = WallID::Unsafe::craggyStone;
                    }
                }
                if (!foundSolidTile && tile.blockID != TileID::empty) {
                    foundSolidTile = true;
                    if (tile.blockID == theme.brick) {
                        tile.slope = rnd.select(
                            {Slope::none,
                             Slope::none,
                             Slope::half,
                             Slope::topRight,
                             Slope::topLeft});
                    }
                }
                if (tile.blockID == TileID::empty &&
                    (existingTileId == TileID::livingWood ||
                     existingTileId == TileID::leaf)) {
                    tile.blockID = existingTileId;
                }
            }
            int maxJ =
                world.conf.shattered ? entry.getHeight() + roomSize + 1 : 100;
            for (int j = entry.getHeight(); j < maxJ; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.wallID == theme.brickWall) {
                    if (j < 50) {
                        continue;
                    } else {
                        break;
                    }
                }
                if (tile.blockID != TileID::empty) {
                    constexpr auto grassMap = frozen::make_map<int, int>(
                        {{TileID::grass, TileID::dirt},
                         {TileID::hallowedGrass, TileID::dirt},
                         {TileID::jungleGrass, TileID::mud}});
                    if (grassMap.contains(tile.blockID)) {
                        tile.blockID = grassMap.at(tile.blockID);
                        Tile &nextTile = world.getTile(x + i, y + j + 1);
                        if (grassMap.contains(nextTile.blockID)) {
                            nextTile.blockID = grassMap.at(nextTile.blockID);
                        }
                    }
                    break;
                }
                tile.blockID = theme.brick;
                tile.wallID = theme.brickWall;
            }
        }
        for (int i = 1; i < entry.getWidth() - 1; ++i) {
            for (int j = 0; j < entry.getHeight(); ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if ((tile.blockID == TileID::dirt ||
                     tile.blockID == TileID::mud) &&
                    world.isExposed(x + i, y + j)) {
                    tile.blockID = tile.blockID == TileID::dirt
                                       ? TileID::grass
                                       : TileID::jungleGrass;
                }
            }
        }
        int delta = isOnLeft ? 1 : -1;
        x += isOnLeft ? entry.getWidth() : -1;
        y += entry.getHeight() - 1;
        int maxJ = world.conf.shattered ? roomSize + 2 : 100;
        for (int i = 0; std::abs(i) < 50; i += delta) {
            int minJ = 2 * std::abs(i);
            if (world.getTile(x + i, y + minJ).blockID != TileID::empty) {
                break;
            }
            for (int j = minJ; j < maxJ; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID != TileID::empty) {
                    if (tile.blockID == TileID::grass) {
                        tile.blockID = TileID::dirt;
                    }
                    break;
                }
                tile.blockID = theme.brick;
                tile.wallID = theme.brickWall;
            }
        }
    }

    void applyWallVariety(const std::vector<Point> &zones)
    {
        size_t zonePortion = zones.size() / 3;
        int roomSpan = roomSize / 2 + wallThickness;
        for (size_t idx = 0; idx < zonePortion; ++idx) {
            auto [x, y] = zones[idx];
            for (int i = -roomSpan; i < roomSpan; ++i) {
                for (int j = -roomSpan; j < roomSpan; ++j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.wallID == theme.brickWall) {
                        tile.wallID = theme.slabWall;
                    }
                }
            }
        }
        for (size_t idx = zonePortion; idx < 2 * zonePortion; ++idx) {
            auto [x, y] = zones[idx];
            for (int i = -roomSpan; i < roomSpan; ++i) {
                for (int j = -roomSpan; j < roomSpan; ++j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.wallID == theme.brickWall) {
                        tile.wallID = theme.tiledWall;
                    }
                }
            }
        }
    }

    bool shouldPaint()
    {
        return !world.conf.unpainted &&
               (world.conf.doubleTrouble || world.conf.hiveQueen ||
                world.conf.celebration || world.conf.forTheWorthy ||
                world.conf.dontDigUp);
    }

    void applyPaint(int dungeonCenter, int dungeonWidth)
    {
        int maxX = std::max(
            world.dungeon.x + 80,
            dungeonCenter + dungeonWidth + roomSize);
        int yDivide =
            wallThickness +
            (world.conf.dontDigUp
                 ? (4 * world.getUndergroundLevel() + world.getCavernLevel()) /
                       5
                 : (world.getUndergroundLevel() + 4 * world.getCavernLevel()) /
                       5);
        for (int x =
                 std::min(world.dungeon.x - 80, dungeonCenter - dungeonWidth);
             x < maxX;
             ++x) {
            for (int y = world.dungeon.y - 40; y < yDivide; ++y) {
                applyPaintAt(x, y);
            }
        }
        maxX = dungeonCenter + dungeonWidth + 2 * roomSize;
        for (int x = dungeonCenter - dungeonWidth; x < maxX; ++x) {
            for (int y = yDivide; y < world.getHeight(); ++y) {
                applyPaintAt(x, y);
            }
        }
    }

    void applyPaintAt(int x, int y)
    {
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == theme.brick || tile.blockID == theme.crackedBrick) {
            tile.blockPaint = theme.paint;
        }
        if (tile.wallID == theme.brickWall || tile.wallID == theme.slabWall ||
            tile.wallID == theme.tiledWall) {
            tile.wallPaint = selectWallPaintAt(x, y);
            if (tile.blockID == TileID::pressurePlate ||
                tile.blockID == TileID::trap) {
                tile.blockPaint = theme.paint;
            }
        }
    }

    int selectWallPaintAt(int x, int y)
    {
        if (world.conf.hiveQueen) {
            Point centroid = getHexCentroid(x, y, 8);
            if (static_cast<int>(
                    99999 * (1 + rnd.getFineNoise(centroid.x, centroid.y))) %
                    11 <
                4) {
                return theme.altPaint;
            }
        }
        return world.conf.dontDigUp ? theme.altPaint : theme.paint;
    }

public:
    Dungeon(Random &r, World &w)
        : rnd(r), world(w), roomSize(16), hallSize(9), wallThickness(5)
    {
        theme.apply(
            rnd.select(
                {TileID::blueBrick, TileID::greenBrick, TileID::pinkBrick}),
            rnd);
        if (world.conf.hiveQueen) {
            theme.paint = rnd.select(
                {Paint::yellow,
                 Paint::orange,
                 Paint::deepYellow,
                 Paint::deepOrange});
            theme.altPaint = theme.paint;
            while (theme.paint == theme.altPaint) {
                theme.altPaint = rnd.select(
                    {Paint::yellow,
                     Paint::orange,
                     Paint::deepYellow,
                     Paint::deepOrange});
            }
        } else if (!world.conf.forTheWorthy) {
            if (world.conf.dontDigUp) {
                theme.paint =
                    world.isCrimson ? Paint::deepRed : Paint::deepPurple;
                theme.altPaint = Paint::black;
            } else if (world.conf.celebration) {
                theme.paint = rnd.select({Paint::deepPink, Paint::deepViolet});
                theme.altPaint = theme.paint;
            }
        }
    }

    void gen(int dungeonCenter)
    {
        if (world.conf.dontDigUp && world.conf.doubleTrouble &&
            (dungeonCenter < world.getWidth() / 2) !=
                (world.surfaceEvilCenter < world.getWidth() / 2)) {
            theme.paint = theme.paint == Paint::deepPurple ? Paint::deepRed
                                                           : Paint::deepPurple;
        }
        selectEntry(dungeonCenter);
        auto [dungeonWidth, dungeonHeight] = selectSize(dungeonCenter);
        int yMin =
            world.conf.dontDigUp
                ? (4 * world.getUndergroundLevel() + world.getCavernLevel()) / 5
                : (world.getUndergroundLevel() + 4 * world.getCavernLevel()) /
                      5;
        int shuffleX = rnd.getInt(0, world.getWidth());
        int shuffleY = rnd.getInt(0, world.getHeight());
        std::vector<Point> zones;
        for (int y = yMin; y < yMin + dungeonHeight; y += roomSize) {
            for (int x = dungeonCenter - dungeonWidth;
                 x < dungeonCenter + dungeonWidth;
                 x += roomSize) {
                double threshold = std::max(
                    0.09,
                    rnd.getCoarseNoise(x + shuffleX, y + shuffleY));
                if (rnd.getCoarseNoise(1.5 * x, 3 * y) > threshold) {
                    int padded = roomSize + 2 * wallThickness;
                    drawRect({x, y}, {x + padded, y + padded}, 0, 0, false);
                    zones.emplace_back(x + padded / 2, y + padded / 2);
                }
            }
        }
        makeMapRoom(zones);
        std::set<Point> connectedZones;
        std::set<Point> allZones{zones.begin(), zones.end()};
        sortInitialZones(dungeonCenter, dungeonWidth, zones);
        for (auto [x, y] : zones) {
            if (connectedZones.empty()) {
                findConnectedZones(x, y, connectedZones, allZones);
                makeEntryHallway(
                    {world.dungeon.x, world.dungeon.y + hallSize - 3},
                    {x, y});
                continue;
            } else if (connectedZones.contains({x, y})) {
                continue;
            }
            Point closest = getClosestPoint(x, y, connectedZones);
            findConnectedZones(x, y, connectedZones, allZones);
            makeHallway({x, y}, closest);
        }
        for (int x = dungeonCenter - dungeonWidth;
             x < dungeonCenter + dungeonWidth + roomSize;
             x += 4) {
            bool prevWasDungeon = false;
            bool checkedForGround = false;
            for (int y = yMin; y < world.getUnderworldLevel(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (prevWasDungeon && tile.blockID == TileID::empty &&
                    tile.wallID != theme.brickWall) {
                    if (checkedForGround) {
                        tile.blockID = theme.brick;
                        tile.actuated = true;
                    } else if (
                        scanWhileEmpty({x, y}, {0, 1}, world).y - y < 35) {
                        tile.blockID = theme.brick;
                        tile.actuated = true;
                        checkedForGround = true;
                    } else {
                        prevWasDungeon = false;
                    }
                } else if (tile.blockID == theme.brick) {
                    prevWasDungeon = true;
                    checkedForGround = false;
                } else {
                    prevWasDungeon = false;
                }
            }
        }
        addBiomeChests(zones);
        addPlatforms(zones);
        addDoors();
        addShelves(dungeonCenter, dungeonWidth);
        addFunctionalFurniture(dungeonCenter, dungeonWidth);
        addSpikes(zones);
        addBoulderTraps(dungeonCenter, dungeonWidth);
        makeEntry();
        addPaintings(dungeonCenter, dungeonWidth);
        addFurniture(dungeonCenter, dungeonWidth);
        if (world.conf.forTheWorthy) {
            addFloatingSpikes(dungeonCenter, dungeonWidth);
        }
        std::sort(
            zones.begin(),
            zones.end(),
            [this, shuffleX, shuffleY](Point a, Point b) {
                return rnd.getCoarseNoise(a.x + shuffleX, a.y + shuffleY) <
                       rnd.getCoarseNoise(b.x + shuffleX, b.y + shuffleY);
            });
        applyWallVariety(zones);
        if (shouldPaint()) {
            applyPaint(dungeonCenter, dungeonWidth);
        }
    }
};

int computeDungeonCenter(World &world)
{
    std::vector avoidPoints{
        0,
        world.spawn.x,
        world.getWidth(),
        world.surfaceEvilCenter,
        static_cast<int>(world.desertCenter),
        static_cast<int>(world.jungleCenter),
        static_cast<int>(world.snowCenter)};
    std::sort(avoidPoints.begin(), avoidPoints.end());
    int maxSpan = 0;
    int dungeonCenter = world.getWidth() / 4;
    for (size_t i = 0; i + 1 < avoidPoints.size(); ++i) {
        int span = avoidPoints[i + 1] - avoidPoints[i];
        if (span > maxSpan) {
            maxSpan = span;
            dungeonCenter = avoidPoints[i] + span / 2;
        }
    }
    return dungeonCenter;
}

void genDungeon(Random &rnd, World &world)
{
    if (world.conf.dungeonSize < 0.01) {
        return;
    }
    std::cout << "Employing the undead\n";
    rnd.shuffleNoise();
    Dungeon structure(rnd, world);
    structure.gen(computeDungeonCenter(world));
}
