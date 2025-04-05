#ifndef BOATS_H
#define BOATS_H

#include "TileBuffer.h"

namespace Data
{

enum class Boat { frozen = 0 };

TileBuffer getBoat(Boat boatId, const std::vector<bool> &framedTiles);

} // namespace Data

#endif // BOATS_H
