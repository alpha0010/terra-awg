#ifndef TORCHES_H
#define TORCHES_H

#include "TileBuffer.h"

namespace Data
{

enum class Torch { favor = 0, left, right, up };

TileBuffer getTorch(Torch torchId, const FramedBitset &framedTiles);

} // namespace Data

#endif // TORCHES_H
