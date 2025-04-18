#include "structures/StructureUtil.h"

#include "World.h"
#include <cmath>
#include <set>

int binLocation(int x, int y, int maxY)
{
    int factor = 128;
    x /= factor;
    y /= factor;
    maxY /= factor;
    return x * maxY + y;
}

bool isLocationUsed(
    int x,
    int y,
    int radius,
    const std::vector<Point> &usedLocations)
{
    for (auto [usedX, usedY] : usedLocations) {
        if (std::hypot(x - usedX, y - usedY) < radius) {
            return true;
        }
    }
    return false;
}

inline const std::set<int> nonSolidTiles{
    TileID::empty,         TileID::alchemyTable,  TileID::bathtub,
    TileID::bed,           TileID::bench,         TileID::bewitchingTable,
    TileID::boneWelder,    TileID::book,          TileID::bookcase,
    TileID::bottle,        TileID::bubble,        TileID::candelabra,
    TileID::candle,        TileID::catacomb,      TileID::chair,
    TileID::chandelier,    TileID::chest,         TileID::chestGroup2,
    TileID::clock,         TileID::dresser,       TileID::hellforge,
    TileID::lantern,       TileID::lihzahrdAltar, TileID::painting3x3,
    TileID::painting6x4,   TileID::piano,         TileID::pot,
    TileID::rollingCactus, TileID::rope,          TileID::silverCoin,
    TileID::sink,          TileID::smallPile,     TileID::statue,
    TileID::table,         TileID::toilet,        TileID::waterCandle,
    TileID::woodenBeam,    TileID::workBench,
};

bool isSolidBlock(int tileId)
{
    return !nonSolidTiles.contains(tileId);
}

Point addPts(Point a, Point b)
{
    return {a.first + b.first, a.second + b.second};
}

Point subPts(Point a, Point b)
{
    return {a.first - b.first, a.second - b.second};
}

bool isInBounds(Point pt, World &world)
{
    return pt.first > 5 && pt.first < world.getWidth() - 5 && pt.second > 5 &&
           pt.second < world.getHeight() - 5;
}

Point scanWhileEmpty(Point from, Point delta, World &world)
{
    while (world.getTile(addPts(from, delta)).blockID == TileID::empty &&
           isInBounds(from, world)) {
        from = addPts(from, delta);
    }
    return from;
}

void placeWire(Point from, Point to, World &world)
{
    world.getTile(from.first, from.second).wireRed = true;
    while (from != to) {
        if (from.first < to.first) {
            ++from.first;
        } else if (from.first > to.first) {
            --from.first;
        }
        world.getTile(from.first, from.second).wireRed = true;
        if (from.second < to.second) {
            ++from.second;
        } else if (from.second > to.second) {
            --from.second;
        }
        world.getTile(from.first, from.second).wireRed = true;
    }
}
