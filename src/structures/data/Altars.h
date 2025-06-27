#ifndef ALTARS_H
#define ALTARS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto corruptAltars = std::views::iota(0, 6);
inline auto crimsonAltars = std::views::iota(6, 12);

TileBuffer getAltar(int altarId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // ALTARS_H
