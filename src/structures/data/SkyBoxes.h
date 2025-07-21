#ifndef SKYBOXES_H
#define SKYBOXES_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

inline auto skyBoxes = std::views::iota(0, 51);

TileBuffer getSkyBox(int skyBoxId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // SKYBOXES_H
