#include "structures/StructureUtil.h"

#include "World.h"

int binLocation(int x, int y, int maxY)
{
    int factor = 128;
    x /= factor;
    y /= factor;
    maxY /= factor;
    return x * maxY + y;
}

Point addPts(Point a, Point b)
{
    return {a.first + b.first, a.second + b.second};
}

Point subPts(Point a, Point b)
{
    return {a.first - b.first, a.second - b.second};
}

Point scanWhileEmpty(Point from, Point delta, World &world)
{
    while (world.getTile(addPts(from, delta)).blockID == TileID::empty) {
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
