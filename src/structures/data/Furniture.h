#ifndef FURNITURE_H
#define FURNITURE_H

#include "TileBuffer.h"
#include <ranges>

namespace Data
{

enum class Variant {
    blueDungeon,
    greenDungeon,
    pinkDungeon,
    mushroom,
    obsidian
};

inline auto furnitureLayouts = std::views::iota(0, 29);

bool convertFurniture(Tile &tile, Variant furnitureSet);

TileBuffer getFurniture(
    int id,
    Variant furnitureSet,
    const std::vector<bool> &framedTiles);

} // namespace Data

#endif // FURNITURE_H
