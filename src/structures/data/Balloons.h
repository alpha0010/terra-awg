#ifndef BALLOONS_H
#define BALLOONS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto balloons = std::views::iota(0, 11);

TileBuffer
getBalloon(int id, int tileId, int paint, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // BALLOONS_H
