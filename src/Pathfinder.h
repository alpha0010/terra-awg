#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "Point.h"
#include <future>
#include <map>
#include <numeric>
#include <queue>
#include <vector>

namespace detail
{
int distHeuristic(const Point &a, const Point &b);

template <typename BoundsCheck>
std::vector<Point> neighbors(Point pt, BoundsCheck isValid)
{
    std::vector<Point> ret;
    for (Point delta : {Point{1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
        Point opt = pt + delta;
        if (isValid(opt)) {
            ret.push_back(opt);
        }
    }
    return ret;
}

/**
 * Approximate pathfind. Resulting path is reversed.
 */
template <typename NodeCost, typename BoundsCheck>
std::vector<Point>
aStarSearch(Point start, Point goal, NodeCost costAt, BoundsCheck isValid)
{
    std::map<Point, int> costEst{{start, distHeuristic(start, goal)}};
    auto costCmp = [&costEst](const Point &a, const Point &b) {
        auto itrA = costEst.find(a);
        int costA = itrA == costEst.end() ? std::numeric_limits<int>::max()
                                          : itrA->second;
        auto itrB = costEst.find(b);
        int costB = itrB == costEst.end() ? std::numeric_limits<int>::max()
                                          : itrB->second;
        return costA > costB;
    };
    std::priority_queue<Point, std::vector<Point>, decltype(costCmp)> frontier{
        costCmp};
    frontier.push(start);
    std::map<Point, int> exactCostTo{{start, 0}};
    std::map<Point, Point> parents;
    while (!frontier.empty()) {
        Point current = frontier.top();
        if (current == goal) {
            std::vector<Point> path;
            while (true) {
                path.push_back(current);
                auto itr = parents.find(current);
                if (itr == parents.end()) {
                    break;
                }
                current = itr->second;
            }
            return path;
        }
        frontier.pop();
        for (Point neighbor : neighbors(current, isValid)) {
            if (!costEst.contains(neighbor)) {
                int stepCost = exactCostTo[current] + costAt(neighbor);
                exactCostTo[neighbor] = stepCost;
                costEst[neighbor] = stepCost + distHeuristic(neighbor, goal);
                frontier.push(neighbor);
                parents[neighbor] = current;
            }
        }
    }
    return {};
}

template <typename NodeCost> int splashCost(Point pt, NodeCost costAt)
{
    int cost = 0;
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            cost += costAt(pt + Point{i, j});
        }
    }
    return cost;
}

template <typename NodeCost> Point cheapestPointNear(Point pt, NodeCost costAt)
{
    Point best = pt;
    int bestCost = splashCost(pt, costAt);
    for (int i = -3; i < 4; ++i) {
        for (int j = -3; j < 4; ++j) {
            Point current = pt + Point{i, j};
            int cost = splashCost(current, costAt);
            if (cost < bestCost) {
                best = current;
                bestCost = cost;
            }
        }
    }
    return best;
}

typedef std::vector<Point>::iterator PathItr;

void joinPath(std::vector<Point> &path, PathItr segBeg, PathItr segEnd);
} // namespace detail

template <typename NodeCost, typename BoundsCheck>
std::vector<Point>
findPath(Point from, Point to, NodeCost costAt, BoundsCheck isValid)
{
    double dist = hypot(from, to);
    if (dist < 400) {
        return detail::aStarSearch(to, from, costAt, isValid);
    }
    Point mid = detail::cheapestPointNear(
        {std::midpoint(from.x, to.x), std::midpoint(from.y, to.y)},
        costAt);
    std::vector<Point> path;
    std::vector<Point> segment;
    if (dist > 900) {
        auto pathPromise = std::async(
            std::launch::async,
            findPath<NodeCost, BoundsCheck>,
            from,
            mid,
            costAt,
            isValid);
        segment = findPath(mid, to, costAt, isValid);
        path = pathPromise.get();
    } else {
        path = findPath(from, mid, costAt, isValid);
        segment = findPath(mid, to, costAt, isValid);
    }
    if (path.empty() || segment.empty()) {
        return {};
    }
    detail::joinPath(path, segment.begin(), segment.end());
    return path;
}

#endif // PATHFINDER_H
