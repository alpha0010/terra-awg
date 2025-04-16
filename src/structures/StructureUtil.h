#ifndef STRUCTUREUTIL_H
#define STRUCTUREUTIL_H

#include <map>
#include <vector>

class World;

typedef std::pair<int, int> Point;
typedef std::map<int, std::vector<Point>> LocationBins;

int binLocation(int x, int y, int maxY);

bool isLocationUsed(
    int x,
    int y,
    int radius,
    const std::vector<Point> &usedLocations);

Point addPts(Point a, Point b);

Point subPts(Point a, Point b);

Point scanWhileEmpty(Point from, Point delta, World &world);

void placeWire(Point from, Point to, World &world);

#endif // STRUCTUREUTIL_H
