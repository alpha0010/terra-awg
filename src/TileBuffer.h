#ifndef TILEBUFFER_H
#define TILEBUFFER_H

#include "Tile.h"
#include <cstdint>
#include <vector>

class TileBuffer
{
private:
    std::vector<Tile> tiles;
    int width;
    int height;

public:
    TileBuffer(
        int w,
        int h,
        const uint16_t *data,
        const std::vector<bool> &framedTiles);

    Tile &getTile(int x, int y);

    int getWidth() const
    {
        return width;
    }

    int getHeight() const
    {
        return height;
    }
};

#endif // TILEBUFFER_H
