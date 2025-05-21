#ifndef DYNCABIN_H
#define DYNCABIN_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto cabins = std::views::iota(0, 5);

TileBuffer
getCabin(int cabinId, int targetWidth, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // DYNCABIN_H
