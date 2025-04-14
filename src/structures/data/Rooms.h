#ifndef ROOMS_H
#define ROOMS_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto pyramidRooms = std::views::iota(0, 5);
inline auto treeRooms = std::views::iota(5, 19);

TileBuffer getRoom(int roomId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // ROOMS_H
