#ifndef ROOMWINDOWS_H
#define ROOMWINDOWS_H

#include "TileBuffer.h"

namespace Data
{

enum class Window { square = 0, tall };

TileBuffer getWindow(
    Window windowId,
    int frameBlockId,
    int paneWallId,
    const std::vector<bool> &framedTiles);

} // namespace Data

#endif // ROOMWINDOWS_H
