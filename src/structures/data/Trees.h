#ifndef TREES_H
#define TREES_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto trees = std::views::iota(0, 25);

TileBuffer getTree(int treeId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // TREES_H
