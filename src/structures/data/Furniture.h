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
    pine,
    skyware,
};

inline auto furnitureLayouts = std::views::iota(0, 29);

bool convertFurniture(Tile &tile, Variant furnitureSet);

TileBuffer
getFurniture(int id, Variant furnitureSet, const FramedBitset &framedTiles);

TileBuffer
getChainLantern(::Variant lanternStyle, const FramedBitset &framedTiles);

} // namespace Data

#endif // FURNITURE_H
