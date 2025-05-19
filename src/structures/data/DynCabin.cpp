#include "structures/data/Homes.h"

namespace
{
// clang-format off
std::vector<std::vector<uint16_t>> cabinLeftData{
// cabinLeft1
{
    519, 4098, 30, 4096, 10, 54, 432, 4096, 10, 90, 450, 4096, 10, 72, 468,
    4096, 38, 4096, 30, 8193, 5, 8192, 27, 8193, 4, 4096, 38
},
};

std::vector<std::vector<uint16_t>> cabinCenterData{
// cabinCenter1
{
    519, 4096, 30, 8193, 5, 8192, 27, 8193, 4, 4096, 38, 4096, 30, 8193, 5,
    8192, 27, 8193, 4, 4096, 38
},
};

std::vector<std::vector<uint16_t>> cabinRightData{
// cabinRight1
{
    519, 4096, 30, 8193, 5, 8192, 27, 8193, 4, 4096, 38, 4098, 30, 4096, 10,
    72, 432, 4096, 10, 54, 450, 4096, 10, 54, 468, 4096, 38
},
};
// clang-format on
} // namespace

namespace Data
{

TileBuffer
getCabin(int cabinId, int targetWidth, const std::vector<bool> &framedTiles)
{
    TileBuffer left = {cabinLeftData[cabinId].data(), framedTiles};
    TileBuffer center = {cabinCenterData[cabinId].data(), framedTiles};
    TileBuffer right = {cabinRightData[cabinId].data(), framedTiles};
    int width = left.getWidth() + right.getWidth();
    while (width < targetWidth) {
        width += center.getWidth();
    }
    int x = left.getWidth();
    left.resize(width, left.getHeight());
    for (; x < width - right.getWidth(); x += center.getWidth()) {
        for (int i = 0; i < center.getWidth(); ++i) {
            for (int j = 0; j < center.getHeight(); ++j) {
                left.getTile(x + i, j) = center.getTile(i, j);
            }
        }
    }
    for (int i = 0; i < right.getWidth(); ++i) {
        for (int j = 0; j < right.getHeight(); ++j) {
            left.getTile(x + i, j) = right.getTile(i, j);
        }
    }
    return left;
}

} // namespace Data
