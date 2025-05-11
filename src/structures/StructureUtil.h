#ifndef STRUCTUREUTIL_H
#define STRUCTUREUTIL_H

#include <map>
#include <vector>

class World;

typedef std::pair<int, int> Point;
typedef std::map<int, std::vector<Point>> LocationBins;

enum class Wire { red = 0, blue, green, yellow };

int binLocation(int x, int y, int maxY);

bool isLocationUsed(
    int x,
    int y,
    int radius,
    const std::vector<Point> &usedLocations,
    int maxCount = 1);

bool isSolidBlock(int tileId);

Point addPts(Point a, Point b);

Point subPts(Point a, Point b);

Point scanWhileEmpty(Point from, Point delta, World &world);

Point scanWhileNotSolid(Point from, Point delta, World &world);

void placeWire(Point from, Point to, Wire wire, World &world);

#endif // STRUCTUREUTIL_H
