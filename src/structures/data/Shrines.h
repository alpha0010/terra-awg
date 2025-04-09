#ifndef SHRINES_H
#define SHRINES_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto shrines = std::views::iota(0, 32);

TileBuffer getShrine(int shrineId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // SHRINES_H
