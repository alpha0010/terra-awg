#include "structures/data/DecoGems.h"

namespace
{
// clang-format off
std::vector<std::vector<uint16_t>> gemData{
// gem1
{
    514, 36867, 267, 2048
},
// gem2
{
    514, 36864, 267, 2072, 36864, 267, 2088, 36864, 267, 2064, 36864, 267, 2080
},
// gem3
{
    515, 36864, 267, 2072, 36864, 267, 2048, 36864, 267, 2088, 36864, 267,
    2064, 36864, 267, 2048, 36864, 267, 2080
},
// gem4
{
    771, 36864, 267, 2072, 36864, 267, 2048, 36864, 267, 2088, 36866, 267,
    2048, 36864, 267, 2064, 36864, 267, 2048, 36864, 267, 2080
},
// gem5
{
    514, 36864, 267, 2072, 36865, 267, 2048, 36864, 267, 2080
},
// gem6
{
    514, 36864, 267, 2048, 36864, 267, 2088, 36864, 267, 2064, 36864, 267, 2048
},
};
// clang-format on
} // namespace

namespace Data
{

TileBuffer getDecoGem(int gemId, const std::vector<bool> &framedTiles)
{
    return {gemData[gemId].data(), framedTiles};
}

} // namespace Data
