#ifndef HOMES_H
#define HOMES_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto homes = std::views::iota(0, 6);

TileBuffer getHome(int homeId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // HOMES_H
