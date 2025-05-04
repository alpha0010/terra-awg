#ifndef MUSHROOMS_H
#define MUSHROOMS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto mushrooms = std::views::iota(0, 14);

TileBuffer getMushroom(int mushroomId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // MUSHROOMS_H
