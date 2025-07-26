#ifndef FURNITURE_H
#define FURNITURE_H

#include "TileBuffer.h"
#include "ids/TileVariant.h"
#include <ranges>

namespace Data
{

enum class Variant {
    blueDungeon,
    greenDungeon,
    pinkDungeon,
    mushroom,
    obsidian,
    ashWood,
    balloon,
    boreal,
    forest,
    granite,
    honey,
    livingWood,
    mahogany,
    marble,
    palm,
    skyware,
};

inline auto furnitureLayouts = std::views::iota(0, 29);

bool convertFurniture(Tile &tile, Variant furnitureSet);

TileBuffer getFurniture(
    int id,
    Variant furnitureSet,
    const std::vector<bool> &framedTiles);

TileBuffer
getChainLantern(::Variant lanternStyle, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // FURNITURE_H
