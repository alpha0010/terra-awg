#include "Pathfinder.h"

#include <set>

namespace detail
{
int distHeuristic(const Point &a, const Point &b)
{
    return 2 * (std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

void joinPath(std::vector<Point> &path, PathItr segBeg, PathItr segEnd)
{
    std::set<Point> segCheck{segBeg, segBeg + 50};
    for (auto itr = path.end() - 50; itr != path.end(); ++itr) {
        if (segCheck.contains(*itr)) {
            while (*segBeg != *itr) {
                ++segBeg;
            }
            path.erase(itr, path.end());
            break;
        }
    }
    path.insert(path.end(), segBeg, segEnd);
}
} // namespace detail
