#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "Point.h"
#include <functional>
#include <vector>

typedef std::function<bool(Point)> BoundsCheck;
typedef std::function<int(Point)> NodeCost;

std::vector<Point>
findPath(Point from, Point to, NodeCost costAt, BoundsCheck isValid);

#endif // PATHFINDER_H
