#ifndef BUILDINGS_H
#define BUILDINGS_H

#include "TileBuffer.h"

namespace Data
{

enum class Building {
    desertTomb1 = 0,
    desertTomb2,
    desertTomb3,
    desertTomb4,
    spiderHall1,
    spiderHall2
};

TileBuffer
getBuilding(Building buildingId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // BUILDINGS_H
