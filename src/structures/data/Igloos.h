#ifndef IGLOOS_H
#define IGLOOS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto igloos = std::views::iota(0, 10);

TileBuffer getIgloo(int iglooId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // IGLOOS_H
