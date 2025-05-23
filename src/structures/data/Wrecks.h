#ifndef WRECKS_H
#define WRECKS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto wrecks = std::views::iota(0, 11);

TileBuffer getWreck(int wreckId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // WRECKS_H
