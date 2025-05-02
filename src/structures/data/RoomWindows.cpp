#include "structures/data/RoomWindows.h"

#include "ids/WallID.h"

namespace
{
// clang-format off
std::vector<std::vector<uint16_t>> windowData{
// window1
{
    1285, 36869, 189, 2048, 8194, 73, 36865, 189, 2048, 8194, 73, 36865, 189,
    2048, 8194, 73, 36869, 189, 2048
},
// window2
{
    1286, 0, 36864, 189, 2072, 36867, 189, 2048, 36864, 189, 2072, 45056, 189,
    73, 2080, 8194, 73, 36865, 189, 2048, 8195, 73, 36864, 189, 2048, 36864,
    189, 2064, 45056, 189, 73, 2088, 8194, 73, 36864, 189, 2048, 0, 36864, 189,
    2064, 36867, 189, 2048
},
};
// clang-format on
} // namespace

namespace Data
{

TileBuffer getWindow(
    Window windowId,
    int frameBlockId,
    int paneWallId,
    const std::vector<bool> &framedTiles)
{
    TileBuffer data{windowData[static_cast<int>(windowId)].data(), framedTiles};
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &tile = data.getTile(i, j);
            if (tile.blockID == TileID::cloud) {
                tile.blockID = frameBlockId;
            }
            if (tile.wallID == WallID::Safe::cloud) {
                tile.wallID = paneWallId;
            }
        }
    }
    return data;
}

} // namespace Data
