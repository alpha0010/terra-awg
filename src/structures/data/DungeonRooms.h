#ifndef DUNGEONROOMS_H
#define DUNGEONROOMS_H

#include "structures/data/Furniture.h"

namespace Data
{

enum class Room { entranceLeft = 0, entranceRight };

TileBuffer getDungeonRoom(
    Room roomId,
    Variant furnitureSet,
    const std::vector<bool> &framedTiles);

} // namespace Data

#endif // DUNGEONROOMS_H
