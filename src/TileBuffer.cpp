#include "TileBuffer.h"

TileBuffer::TileBuffer(
    const uint16_t *data,
    const std::vector<bool> &framedTiles)
    : width(*data >> 8), height(*data & 0xff), tiles(width * height)
{
    ++data;
    int rle = 0;
    for (size_t i = 0; i < tiles.size(); ++i) {
        if (rle > 0) {
            // Copy previous tile to handle run-length encoding compression.
            tiles[i] = tiles[i - 1];
            --rle;
            continue;
        }
        uint16_t header = *data;
        ++data;
        rle = header & 0x0fff;
        Tile &tile = tiles[i];
        if (header & 0x1000) {
            tile.blockID = *data;
            ++data;
            if (framedTiles[tile.blockID]) {
                tile.frameX = *data;
                ++data;
                tile.frameY = *data;
                ++data;
            }
        }
        if (header & 0x2000) {
            tile.wallID = *data;
            ++data;
        }
        if (header & 0x4000) {
            tile.blockPaint = *data & 0x00ff;
            tile.wallPaint = *data >> 8;
            ++data;
        }
        if (header & 0x8000) {
            uint16_t flags = *data;
            ++data;
            switch (flags & 0x0007) {
            case 1:
                tile.liquid = Liquid::water;
                break;
            case 2:
                tile.liquid = Liquid::lava;
                break;
            case 3:
                tile.liquid = Liquid::honey;
                break;
            case 4:
                tile.liquid = Liquid::shimmer;
                break;
            }
            tile.slope = static_cast<Slope>((flags >> 3) & 0x0007);
            tile.wireRed = (flags & 0x0040) != 0;
            tile.wireBlue = (flags & 0x0080) != 0;
            tile.wireGreen = (flags & 0x0100) != 0;
            tile.wireYellow = (flags & 0x0200) != 0;
            tile.actuator = (flags & 0x0400) != 0;
            tile.actuated = (flags & 0x0800) != 0;
            tile.echoCoatBlock = (flags & 0x1000) != 0;
            tile.echoCoatWall = (flags & 0x2000) != 0;
            tile.illuminantBlock = (flags & 0x4000) != 0;
            tile.illuminantWall = (flags & 0x8000) != 0;
        }
    }
}

Tile &TileBuffer::getTile(int x, int y)
{
    return tiles[y + x * height];
}

const Tile &TileBuffer::getTile(int x, int y) const
{
    return tiles[y + x * height];
}

void TileBuffer::resize(int w, int h)
{
    width = w;
    height = h;
    tiles.resize(w * h);
}
