#ifndef FURNITURE_H
#define FURNITURE_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

enum class Variant { blueDungeon, greenDungeon, pinkDungeon };

inline auto furnitureLayouts = std::views::iota(0, 29);

TileBuffer getFurniture(
    int id,
    Variant furnitureSet,
    const std::vector<bool> &framedTiles);

} // namespace Data

#endif // FURNITURE_H
