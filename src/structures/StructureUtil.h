#ifndef STRUCTUREUTIL_H
#define STRUCTUREUTIL_H

#include "Point.h"
#include <map>
#include <vector>

class World;

typedef std::map<int, std::vector<Point>> LocationBins;

enum class Wire { red = 0, blue, green, yellow };

int makeCongruent(int val, int mod);

int binLocation(int x, int y, int maxY);

bool isLocationUsed(
    int x,
    int y,
    int radius,
    const std::vector<Point> &usedLocations,
    int maxCount = 1);

bool isSolidBlock(int tileId);

Point scanWhileEmpty(Point from, Point delta, World &world);

Point scanWhileNotSolid(Point from, Point delta, World &world);

int getRainbowPaint(int x, int y);

int getDeepRainbowPaint(int x, int y);

void placeWire(Point from, Point to, Wire wire, World &world);

#endif // STRUCTUREUTIL_H
