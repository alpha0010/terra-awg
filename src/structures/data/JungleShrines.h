#ifndef JUNGLESHRINES_H
#define JUNGLESHRINES_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto shrines = std::views::iota(0, 76);

TileBuffer getShrine(int shrineId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // JUNGLESHRINES_H
