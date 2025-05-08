#ifndef BUILDINGS_H
#define BUILDINGS_H

#include "TileBuffer.h"

namespace Data
{

enum class Building { spiderHall = 0 };

TileBuffer
getBuilding(Building buildingId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // BUILDINGS_H
