#ifndef TILEBUFFER_H
#define TILEBUFFER_H

#include "Tile.h"
#include <cstdint>
#include <vector>

class TileBuffer
{
private:
    int width;
    int height;
    std::vector<Tile> tiles;

public:
    TileBuffer() = default;
    /**
     * Max dimensions supported by data format are 255x255 tiles.
     */
    TileBuffer(const uint16_t *data, const std::vector<bool> &framedTiles);

    Tile &getTile(int x, int y);

    const Tile &getTile(int x, int y) const;

    /**
     * Existing tiles are invalidated if height changes.
     */
    void resize(int w, int h);

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
