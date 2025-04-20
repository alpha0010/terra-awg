#include "structures/data/SwordShrines.h"

namespace
{
// clang-format off
std::vector<std::vector<uint16_t>> shrineData{
// swordShrine1
{
    2308, 2, 36864, 273, 8, 2, 4096, 273, 1, 36864, 273, 8, 4096, 273, 4096,
    187, 918, 0, 4096, 187, 918, 18, 4097, 273, 4096, 187, 936, 0, 4096, 187,
    936, 18, 4097, 273, 4096, 187, 954, 0, 4096, 187, 954, 18, 4097, 273, 1,
    36864, 273, 8, 4096, 273, 2, 4096, 273, 2, 36864, 273, 8
},
};
// clang-format on
} // namespace

namespace Data
{

TileBuffer getSwordShrine(const std::vector<bool> &framedTiles)
{
    return {shrineData[0].data(), framedTiles};
}

} // namespace Data
