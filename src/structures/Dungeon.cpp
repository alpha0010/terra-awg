#include "structures/Dungeon.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>
#include <set>

typedef std::pair<int, int> Point;

class Dungeon
{
private:
    Random &rnd;
    World &world;
    int roomSize;
    int hallSize;
    int wallThickness;

    void drawRect(Point topLeft, Point bottomRight, double skewX, double skewY)
    {
        int width = bottomRight.first - topLeft.first;
        int height = bottomRight.second - topLeft.second;
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                Tile &tile = world.getTile(
                    topLeft.first + i + skewX * j,
                    topLeft.second + j + skewY * i);
                if (i < wallThickness || j < wallThickness ||
                    width - i <= wallThickness || height - j <= wallThickness) {
                    if (tile.wallID != WallID::Unsafe::blueBrick) {
                        tile.blockID = TileID::blueBrick;
                    }
                } else {
                    tile.blockID = TileID::empty;
                }
                tile.wallID = WallID::Unsafe::blueBrick;
            }
        }
    }

    void findConnectedZones(int x, int y, std::set<Point> &connectedZones)
    {
        if (connectedZones.contains({x, y}) ||
            world.getTile(x, y).wallID != WallID::Unsafe::blueBrick) {
            return;
        }
        connectedZones.insert({x, y});
        for (auto [i, j] :
             {Point{-roomSize, 0},
              {roomSize, 0},
              {0, -roomSize},
              {0, roomSize}}) {
            Tile &tile = world.getTile(x + i / 2, y + j / 2);
            if (tile.blockID == TileID::empty &&
                tile.wallID == WallID::Unsafe::blueBrick) {
                findConnectedZones(x + i, y + j, connectedZones);
            }
        }
    }

    Point getClosestPoint(int x, int y, const std::set<Point> &points)
    {
        Point closest = *points.begin();
        double minDist = std::hypot(x - closest.first, y - closest.second);
        for (auto [px, py] : points) {
            double curDist = std::hypot(x - px, y - py);
            if (curDist < minDist) {
                minDist = curDist;
                closest = {px, py};
            }
        }
        return closest;
    }

    void makeHallway(Point from, Point to)
    {
        int deltaX = std::abs(to.first - from.first);
        int deltaY = std::abs(to.second - from.second);
        if (deltaY > deltaX) {
            if (from.second > to.second) {
                std::swap(from, to);
            }
            drawRect(
                {from.first - hallSize, from.second - hallSize},
                {from.first + hallSize, to.second + hallSize},
                (to.first - from.first) / (2.0 * hallSize + deltaY),
                0);
        } else {
            if (from.first > to.first) {
                std::swap(from, to);
            }
            drawRect(
                {from.first - hallSize, from.second - hallSize},
                {to.first + hallSize, from.second + hallSize},
                0,
                (to.second - from.second) / (2.0 * hallSize + deltaX));
        }
    }

    void applyBrickTheme(
        const std::vector<Point> &zones,
        int dungeonCenter,
        int dungeonWidth)
    {
        int themeBrick = rnd.select(
            {TileID::blueBrick, TileID::greenBrick, TileID::pinkBrick});
        int brickWall = WallID::Unsafe::blueBrick;
        int slabWall = WallID::Unsafe::blueSlab;
        int tiledWall = WallID::Unsafe::blueTiled;
        if (themeBrick == TileID::greenBrick) {
            brickWall = WallID::Unsafe::greenBrick;
            slabWall = WallID::Unsafe::greenSlab;
            tiledWall = WallID::Unsafe::greenTiled;
        } else if (themeBrick == TileID::pinkBrick) {
            brickWall = WallID::Unsafe::pinkBrick;
            slabWall = WallID::Unsafe::pinkSlab;
            tiledWall = WallID::Unsafe::pinkTiled;
        }
        size_t zonePortion = zones.size() / 3;
        int roomSpan = roomSize / 2 + wallThickness;
        for (size_t idx = 0; idx < zonePortion; ++idx) {
            auto [x, y] = zones[idx];
            for (int i = -roomSpan; i < roomSpan; ++i) {
                for (int j = -roomSpan; j < roomSpan; ++j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.wallID == WallID::Unsafe::blueBrick) {
                        tile.wallID = slabWall;
                    }
                }
            }
        }
        for (size_t idx = zonePortion; idx < 2 * zonePortion; ++idx) {
            auto [x, y] = zones[idx];
            for (int i = -roomSpan; i < roomSpan; ++i) {
                for (int j = -roomSpan; j < roomSpan; ++j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    if (tile.wallID == WallID::Unsafe::blueBrick) {
                        tile.wallID = tiledWall;
                    }
                }
            }
        }
        if (themeBrick == TileID::blueBrick) {
            return;
        }
        for (int x = dungeonCenter - dungeonWidth;
             x < dungeonCenter + dungeonWidth + roomSize + wallThickness;
             ++x) {
            for (int y = 0; y < world.getUnderworldLevel(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::blueBrick) {
                    tile.blockID = themeBrick;
                }
                if (tile.wallID == WallID::Unsafe::blueBrick) {
                    tile.wallID = brickWall;
                }
            }
        }
    }

public:
    Dungeon(Random &r, World &w)
        : rnd(r), world(w), roomSize(16), hallSize(9), wallThickness(5)
    {
    }

    void gen(int dungeonCenter)
    {
        int dungeonWidth = rnd.getDouble(0.029, 0.034) * world.getWidth();
        int dungeonHeight = rnd.getDouble(0.35, 0.4) * world.getHeight();
        int yMin =
            (world.getUndergroundLevel() + 4 * world.getCavernLevel()) / 5;
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
                    drawRect({x, y}, {x + padded, y + padded}, 0, 0);
                    zones.emplace_back(x + padded / 2, y + padded / 2);
                }
            }
        }
        std::set<Point> connectedZones;
        for (auto [x, y] : zones) {
            if (connectedZones.empty()) {
                findConnectedZones(x, y, connectedZones);
                makeHallway({dungeonCenter, world.spawnY}, {x, y});
                continue;
            }
            Point closest = getClosestPoint(x, y, connectedZones);
            findConnectedZones(x, y, connectedZones);
            makeHallway({x, y}, closest);
        }
        for (int x = dungeonCenter - dungeonWidth;
             x < dungeonCenter + dungeonWidth + roomSize;
             x += 4) {
            bool prevWasDungeon = false;
            for (int y = yMin; y < world.getUnderworldLevel(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (prevWasDungeon && tile.blockID == TileID::empty &&
                    tile.wallID != WallID::Unsafe::blueBrick) {
                    tile.blockID = TileID::blueBrick;
                    tile.actuated = true;
                } else if (tile.blockID == TileID::blueBrick) {
                    prevWasDungeon = true;
                } else {
                    prevWasDungeon = false;
                }
            }
        }
        std::sort(
            zones.begin(),
            zones.end(),
            [this, shuffleX, shuffleY](Point a, Point b) {
                return rnd.getCoarseNoise(
                           a.first + shuffleX,
                           a.second + shuffleY) <
                       rnd.getCoarseNoise(
                           b.first + shuffleX,
                           b.second + shuffleY);
            });
        applyBrickTheme(zones, dungeonCenter, dungeonWidth);
    }
};

void genDungeon(Random &rnd, World &world)
{
    std::cout << "Employing the undead\n";
    rnd.shuffleNoise();
    std::vector avoidPoints{
        0,
        world.getWidth() / 2,
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
    world.dungeonCenter = dungeonCenter;
    Dungeon structure(rnd, world);
    structure.gen(dungeonCenter);
}
