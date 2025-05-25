#ifndef DECOGEMS_H
#define DECOGEMS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto gems = std::views::iota(0, 6);

TileBuffer getDecoGem(int gemId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // DECOGEMS_H
