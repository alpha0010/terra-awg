#include "structures/data/Furniture.h"

#include "vendor/frozen/map.h"

namespace
{
// clang-format off
std::vector<std::vector<uint16_t>> furnitureData{
// furniture1
{
    1798, 4, 4096, 189, 1, 4096, 93, 0, 0, 4096, 93, 0, 18, 4096, 93, 0, 36,
    4096, 189, 2, 4096, 89, 0, 0, 4096, 89, 0, 18, 4096, 189, 2, 4096, 89, 18,
    0, 4096, 89, 18, 18, 4096, 189, 2, 4096, 89, 36, 0, 4096, 89, 36, 18, 4096,
    189, 1, 4096, 93, 0, 0, 4096, 93, 0, 18, 4096, 93, 0, 36, 4096, 189, 4,
    4096, 189
},
// furniture2
{
    1030, 4097, 189, 3, 4096, 189, 4, 4096, 189, 4096, 42, 0, 0, 4096, 42, 0,
    18, 2, 4096, 189, 4
},
// furniture3
{
    1030, 4096, 189, 4, 4096, 189, 4096, 42, 0, 0, 4096, 42, 0, 18, 2, 4096,
    189, 4, 4097, 189, 3
},
// furniture4
{
    1798, 4096, 189, 4, 4096, 189, 4096, 42, 0, 0, 4096, 42, 0, 18, 2, 4096,
    189, 4, 4096, 189, 4, 4096, 189, 4, 4096, 189, 4096, 42, 0, 0, 4096, 42, 0,
    18, 2, 4096, 189, 4
},
// furniture5
{
    1541, 2, 4096, 18, 0, 0, 4096, 189, 1, 4096, 33, 0, 0, 4096, 18, 18, 0,
    4096, 189, 1, 4096, 87, 0, 0, 4096, 87, 0, 18, 4096, 189, 1, 4096, 87, 18,
    0, 4096, 87, 18, 18, 4096, 189, 1, 4096, 87, 36, 0, 4096, 87, 36, 18, 4096,
    189, 3, 4096, 189
},
// furniture6
{
    1286, 4, 4096, 189, 2, 4096, 15, 18, 0, 4096, 15, 18, 18, 4096, 189, 2,
    4096, 14, 0, 0, 4096, 14, 0, 18, 4096, 189, 0, 4096, 100, 0, 0, 4096, 100,
    0, 18, 4096, 14, 18, 0, 4096, 14, 18, 18, 4096, 189, 0, 4096, 100, 18, 0,
    4096, 100, 18, 18, 4096, 14, 36, 0, 4096, 14, 36, 18, 4096, 189
},
// furniture7
{
    1798, 0, 4096, 101, 0, 0, 4096, 101, 0, 18, 4096, 101, 0, 36, 4096, 101, 0,
    54, 4096, 189, 0, 4096, 101, 18, 0, 4096, 101, 18, 18, 4096, 101, 18, 36,
    4096, 101, 18, 54, 4096, 189, 0, 4096, 101, 36, 0, 4096, 101, 36, 18, 4096,
    101, 36, 36, 4096, 101, 36, 54, 4096, 189, 0, 4096, 33, 0, 0, 4096, 19, 90,
    0, 1, 4096, 189, 0, 4096, 101, 0, 0, 4096, 101, 0, 18, 4096, 101, 0, 36,
    4096, 101, 0, 54, 4096, 189, 0, 4096, 101, 18, 0, 4096, 101, 18, 18, 4096,
    101, 18, 36, 4096, 101, 18, 54, 4096, 189, 0, 4096, 101, 36, 0, 4096, 101,
    36, 18, 4096, 101, 36, 36, 4096, 101, 36, 54, 4096, 189
},
// furniture8
{
    2053, 1, 4098, 189, 1, 4096, 79, 72, 0, 4096, 79, 72, 18, 4096, 189, 1,
    4096, 79, 90, 0, 4096, 79, 90, 18, 4096, 189, 1, 4096, 79, 108, 0, 4096,
    79, 108, 18, 4096, 189, 1, 4096, 79, 126, 0, 4096, 79, 126, 18, 4096, 189,
    1, 4096, 21, 0, 0, 4096, 21, 0, 18, 4096, 189, 1, 4096, 21, 18, 0, 4096,
    21, 18, 18, 4096, 189, 3, 4096, 189
},
// furniture9
{
    1287, 4096, 189, 5, 4096, 189, 4096, 34, 0, 0, 4096, 34, 0, 18, 4096, 34,
    0, 36, 2, 4096, 189, 4096, 34, 18, 0, 4096, 34, 18, 18, 4096, 34, 18, 36,
    2, 4096, 189, 4096, 34, 36, 0, 4096, 34, 36, 18, 4096, 34, 36, 36, 2, 4096,
    189, 5
},
// furniture10
{
    1031, 0, 4096, 104, 0, 0, 4096, 104, 0, 18, 4096, 104, 0, 36, 4096, 104, 0,
    54, 4096, 104, 0, 72, 4096, 189, 0, 4096, 104, 18, 0, 4096, 104, 18, 18,
    4096, 104, 18, 36, 4096, 104, 18, 54, 4096, 104, 18, 72, 4096, 189, 2,
    4096, 105, 0, 162, 4096, 105, 0, 180, 4096, 105, 0, 198, 4096, 189, 2,
    4096, 105, 18, 162, 4096, 105, 18, 180, 4096, 105, 18, 198, 4096, 189
},
// furniture11
{
    1797, 0, 4096, 105, 0, 0, 4096, 105, 0, 18, 4096, 105, 0, 36, 4096, 189, 0,
    4096, 105, 18, 0, 4096, 105, 18, 18, 4096, 105, 18, 36, 4096, 189, 1, 4096,
    90, 0, 0, 4096, 90, 0, 18, 4096, 189, 1, 4096, 90, 18, 0, 4096, 90, 18, 18,
    4096, 189, 1, 4096, 90, 36, 0, 4096, 90, 36, 18, 4096, 189, 1, 4096, 90,
    54, 0, 4096, 90, 54, 18, 4096, 189, 0, 4096, 93, 0, 0, 4096, 93, 0, 18,
    4096, 93, 0, 36, 4096, 189
},
// furniture12
{
    1030, 4, 4096, 189, 1, 4096, 105, 0, 0, 4096, 105, 0, 18, 4096, 105, 0, 36,
    4096, 189, 1, 4096, 105, 18, 0, 4096, 105, 18, 18, 4096, 105, 18, 36, 4096,
    189, 4, 4096, 189
},
// furniture13
{
    2053, 1, 4096, 88, 0, 0, 4096, 88, 0, 18, 4096, 189, 0, 4096, 33, 0, 0,
    4096, 88, 18, 0, 4096, 88, 18, 18, 4096, 189, 1, 4096, 88, 36, 0, 4096, 88,
    36, 18, 4096, 189, 3, 4096, 189, 1, 4096, 79, 0, 0, 4096, 79, 0, 18, 4096,
    189, 1, 4096, 79, 18, 0, 4096, 79, 18, 18, 4096, 189, 1, 4096, 79, 36, 0,
    4096, 79, 36, 18, 4096, 189, 1, 4096, 79, 54, 0, 4096, 79, 54, 18, 4096,
    189
},
// furniture14
{
    1285, 2, 4097, 189, 1, 4096, 497, 18, 0, 4096, 497, 18, 18, 4096, 189, 3,
    4096, 189, 1, 4096, 172, 0, 0, 4096, 172, 0, 18, 4096, 189, 1, 4096, 172,
    18, 0, 4096, 172, 18, 18, 4096, 189
},
// furniture15
{
    1542, 0, 4096, 101, 0, 0, 4096, 101, 0, 18, 4096, 101, 0, 36, 4096, 101, 0,
    54, 4096, 189, 0, 4096, 101, 18, 0, 4096, 101, 18, 18, 4096, 101, 18, 36,
    4096, 101, 18, 54, 4096, 189, 0, 4096, 101, 36, 0, 4096, 101, 36, 18, 4096,
    101, 36, 36, 4096, 101, 36, 54, 4096, 189, 2, 4096, 89, 0, 0, 4096, 89, 0,
    18, 4096, 189, 2, 4096, 89, 18, 0, 4096, 89, 18, 18, 4096, 189, 2, 4096,
    89, 36, 0, 4096, 89, 36, 18, 4096, 189
},
// furniture16
{
    775, 4098, 189, 4, 36864, 19, 90, 0, 8, 4096, 42, 0, 0, 4096, 42, 0, 18, 9
},
// furniture17
{
    775, 7, 36864, 19, 90, 0, 8, 4096, 42, 0, 0, 4096, 42, 0, 18, 2, 4098, 189,
    3
},
// furniture18
{
    1029, 1, 4096, 172, 0, 0, 4096, 172, 0, 18, 4096, 189, 1, 4096, 172, 18, 0,
    4096, 172, 18, 18, 4096, 189, 0, 4096, 100, 0, 0, 4096, 100, 0, 18, 4096,
    18, 0, 0, 4096, 189, 0, 4096, 100, 18, 0, 4096, 100, 18, 18, 4096, 18, 18,
    0, 4096, 189
},
// furniture19
{
    775, 2, 4096, 93, 0, 0, 4096, 93, 0, 18, 4096, 93, 0, 36, 4096, 189, 0,
    4096, 104, 0, 0, 4096, 104, 0, 18, 4096, 104, 0, 36, 4096, 104, 0, 54,
    4096, 104, 0, 72, 4096, 189, 0, 4096, 104, 18, 0, 4096, 104, 18, 18, 4096,
    104, 18, 36, 4096, 104, 18, 54, 4096, 104, 18, 72, 4096, 189
},
// furniture20
{
    1286, 2, 4096, 87, 0, 0, 4096, 87, 0, 18, 4096, 189, 2, 4096, 87, 18, 0,
    4096, 87, 18, 18, 4096, 189, 2, 4096, 87, 36, 0, 4096, 87, 36, 18, 4096,
    189, 1, 4096, 105, 0, 162, 4096, 105, 0, 180, 4096, 105, 0, 198, 4096, 189,
    1, 4096, 105, 18, 162, 4096, 105, 18, 180, 4096, 105, 18, 198, 4096, 189
},
// furniture21
{
    1798, 4, 4096, 189, 2, 4096, 15, 18, 0, 4096, 15, 18, 18, 4096, 189, 2,
    4096, 14, 0, 0, 4096, 14, 0, 18, 4096, 189, 2, 4096, 14, 18, 0, 4096, 14,
    18, 18, 4096, 189, 2, 4096, 14, 36, 0, 4096, 14, 36, 18, 4096, 189, 2,
    4096, 15, 0, 0, 4096, 15, 0, 18, 4096, 189, 4, 4096, 189
},
// furniture22
{
    1799, 4096, 189, 5, 4096, 189, 5, 4096, 189, 4096, 34, 0, 0, 4096, 34, 0,
    18, 4096, 34, 0, 36, 2, 4096, 189, 4096, 34, 18, 0, 4096, 34, 18, 18, 4096,
    34, 18, 36, 2, 4096, 189, 4096, 34, 36, 0, 4096, 34, 36, 18, 4096, 34, 36,
    36, 2, 4096, 189, 5, 4096, 189, 5
},
// furniture23
{
    1542, 4, 4096, 189, 4, 4096, 189, 2, 4096, 15, 0, 0, 4096, 15, 0, 18, 4096,
    189, 0, 4096, 101, 0, 0, 4096, 101, 0, 18, 4096, 101, 0, 36, 4096, 101, 0,
    54, 4096, 189, 0, 4096, 101, 18, 0, 4096, 101, 18, 18, 4096, 101, 18, 36,
    4096, 101, 18, 54, 4096, 189, 0, 4096, 101, 36, 0, 4096, 101, 36, 18, 4096,
    101, 36, 36, 4096, 101, 36, 54, 4096, 189
},
// furniture24
{
    1797, 1, 4096, 90, 72, 0, 4096, 90, 72, 18, 4096, 189, 1, 4096, 90, 90, 0,
    4096, 90, 90, 18, 4096, 189, 1, 4096, 90, 108, 0, 4096, 90, 108, 18, 4096,
    189, 1, 4096, 90, 126, 0, 4096, 90, 126, 18, 4096, 189, 0, 4096, 93, 0, 0,
    4096, 93, 0, 18, 4096, 93, 0, 36, 4096, 189, 3, 4096, 189, 1, 4096, 497, 0,
    0, 4096, 497, 0, 18, 4096, 189
},
// furniture25
{
    1797, 3, 4096, 189, 2, 4096, 18, 0, 0, 4096, 189, 2, 4096, 18, 18, 0, 4096,
    189, 1, 4096, 89, 0, 0, 4096, 89, 0, 18, 4096, 189, 1, 4096, 89, 18, 0,
    4096, 89, 18, 18, 4096, 189, 1, 4096, 89, 36, 0, 4096, 89, 36, 18, 4096,
    189, 3, 4096, 189
},
// furniture26
{
    773, 1, 4096, 33, 0, 0, 4096, 18, 0, 0, 4096, 189, 2, 4096, 18, 18, 0,
    4096, 189, 1, 4096, 15, 0, 0, 4096, 15, 0, 18, 4096, 189
},
// furniture27
{
    2310, 4, 4096, 189, 0, 4096, 101, 0, 0, 4096, 101, 0, 18, 4096, 101, 0, 36,
    4096, 101, 0, 54, 4096, 189, 0, 4096, 101, 18, 0, 4096, 101, 18, 18, 4096,
    101, 18, 36, 4096, 101, 18, 54, 4096, 189, 0, 4096, 101, 36, 0, 4096, 101,
    36, 18, 4096, 101, 36, 36, 4096, 101, 36, 54, 4096, 189, 4, 4096, 189, 2,
    4096, 88, 0, 0, 4096, 88, 0, 18, 4096, 189, 2, 4096, 88, 18, 0, 4096, 88,
    18, 18, 4096, 189, 2, 4096, 88, 36, 0, 4096, 88, 36, 18, 4096, 189, 4,
    4096, 189
},
// furniture28
{
    1799, 5, 4096, 189, 0, 4096, 104, 0, 0, 4096, 104, 0, 18, 4096, 104, 0, 36,
    4096, 104, 0, 54, 4096, 104, 0, 72, 4096, 189, 0, 4096, 104, 18, 0, 4096,
    104, 18, 18, 4096, 104, 18, 36, 4096, 104, 18, 54, 4096, 104, 18, 72, 4096,
    189, 3, 4096, 87, 0, 0, 4096, 87, 0, 18, 4096, 189, 3, 4096, 87, 18, 0,
    4096, 87, 18, 18, 4096, 189, 3, 4096, 87, 36, 0, 4096, 87, 36, 18, 4096,
    189, 5, 4096, 189
},
// furniture29
{
    1542, 4, 4096, 189, 2, 4096, 14, 0, 0, 4096, 14, 0, 18, 4096, 189, 2, 4096,
    14, 18, 0, 4096, 14, 18, 18, 4096, 189, 2, 4096, 14, 36, 0, 4096, 14, 36,
    18, 4096, 189, 2, 4096, 15, 0, 0, 4096, 15, 0, 18, 4096, 189, 4, 4096, 189
},
};

std::vector<std::vector<uint16_t>> lanternData{
// chainLantern1
{
    263, 4099, 214, 4096, 19, 90, 162, 4096, 42, 0, 144, 4096, 42, 0, 162
},
// chainLantern2
{
    263, 4099, 214, 4096, 19, 90, 162, 4096, 42, 0, 216, 4096, 42, 0, 234
},
// chainLantern3
{
    263, 4099, 214, 4096, 19, 90, 162, 4096, 42, 18, 900, 4096, 42, 18, 918
},
};
// clang-format on
} // namespace

namespace Data
{

inline constexpr auto frameDetail =
    frozen::make_map<std::pair<int, Variant>, std::pair<int, int>>({
        // Base: Metal Bathtub
        {{TileID::bathtub, Variant::blueDungeon}, {0, 756}},
        {{TileID::bathtub, Variant::greenDungeon}, {0, 792}},
        {{TileID::bathtub, Variant::pinkDungeon}, {0, 828}},
        {{TileID::bathtub, Variant::mushroom}, {0, 648}},
        {{TileID::bathtub, Variant::obsidian}, {0, 900}},
        // Base: Wooden Bed
        {{TileID::bed, Variant::blueDungeon}, {0, 180}},
        {{TileID::bed, Variant::greenDungeon}, {0, 216}},
        {{TileID::bed, Variant::pinkDungeon}, {0, 252}},
        {{TileID::bed, Variant::mushroom}, {0, 828}},
        {{TileID::bed, Variant::obsidian}, {0, 288}},
        // Base: Wooden Bench
        {{TileID::bench, Variant::blueDungeon}, {324, 0}},
        {{TileID::bench, Variant::greenDungeon}, {378, 0}},
        {{TileID::bench, Variant::pinkDungeon}, {432, 0}},
        {{TileID::bench, Variant::mushroom}, {918, 0}},
        {{TileID::bench, Variant::obsidian}, {540, 0}},
        // Base: Wooden Bookcase
        {{TileID::bookcase, Variant::blueDungeon}, {54, 0}},
        {{TileID::bookcase, Variant::greenDungeon}, {108, 0}},
        {{TileID::bookcase, Variant::pinkDungeon}, {162, 0}},
        {{TileID::bookcase, Variant::mushroom}, {1296, 0}},
        {{TileID::bookcase, Variant::obsidian}, {216, 0}},
        // Base: Candelabra
        {{TileID::candelabra, Variant::blueDungeon}, {0, 792}},
        {{TileID::candelabra, Variant::greenDungeon}, {0, 828}},
        {{TileID::candelabra, Variant::pinkDungeon}, {0, 864}},
        {{TileID::candelabra, Variant::mushroom}, {36, 684}},
        {{TileID::candelabra, Variant::obsidian}, {0, 900}},
        {{TileID::candelabra, Variant::ashWood}, {0, 1512}},
        {{TileID::candelabra, Variant::balloon}, {0, 1476}},
        {{TileID::candelabra, Variant::boreal}, {0, 720}},
        {{TileID::candelabra, Variant::forest}, {0, 0}},
        {{TileID::candelabra, Variant::granite}, {0, 1044}},
        {{TileID::candelabra, Variant::honey}, {0, 144}},
        {{TileID::candelabra, Variant::livingWood}, {0, 468}},
        {{TileID::candelabra, Variant::mahogany}, {0, 252}},
        {{TileID::candelabra, Variant::marble}, {0, 1080}},
        {{TileID::candelabra, Variant::palm}, {0, 648}},
        {{TileID::candelabra, Variant::skyware}, {0, 396}},
        // Base: Candle
        {{TileID::candle, Variant::blueDungeon}, {0, 22}},
        {{TileID::candle, Variant::greenDungeon}, {0, 44}},
        {{TileID::candle, Variant::pinkDungeon}, {0, 66}},
        {{TileID::candle, Variant::mushroom}, {18, 418}},
        {{TileID::candle, Variant::obsidian}, {0, 550}},
        {{TileID::candle, Variant::ashWood}, {0, 902}},
        {{TileID::candle, Variant::balloon}, {0, 880}},
        {{TileID::candle, Variant::boreal}, {0, 440}},
        {{TileID::candle, Variant::forest}, {0, 0}},
        {{TileID::candle, Variant::granite}, {0, 616}},
        {{TileID::candle, Variant::honey}, {0, 484}},
        {{TileID::candle, Variant::livingWood}, {0, 308}},
        {{TileID::candle, Variant::mahogany}, {0, 198}},
        {{TileID::candle, Variant::marble}, {0, 638}},
        {{TileID::candle, Variant::palm}, {0, 396}},
        {{TileID::candle, Variant::skyware}, {0, 264}},
        // Base: Wooden Chair
        {{TileID::chair, Variant::blueDungeon}, {0, 520}},
        {{TileID::chair, Variant::greenDungeon}, {0, 560}},
        {{TileID::chair, Variant::pinkDungeon}, {0, 600}},
        {{TileID::chair, Variant::mushroom}, {0, 360}},
        {{TileID::chair, Variant::obsidian}, {0, 640}},
        {{TileID::chair, Variant::ashWood}, {0, 1880}},
        {{TileID::chair, Variant::balloon}, {0, 1840}},
        {{TileID::chair, Variant::boreal}, {0, 1200}},
        {{TileID::chair, Variant::forest}, {0, 0}},
        {{TileID::chair, Variant::granite}, {0, 1360}},
        {{TileID::chair, Variant::honey}, {0, 880}},
        {{TileID::chair, Variant::livingWood}, {0, 200}},
        {{TileID::chair, Variant::mahogany}, {0, 120}},
        {{TileID::chair, Variant::marble}, {0, 1400}},
        {{TileID::chair, Variant::palm}, {0, 1160}},
        {{TileID::chair, Variant::skyware}, {0, 400}},
        // Base: Copper Chandelier
        {{TileID::chandelier, Variant::blueDungeon}, {0, 1458}},
        {{TileID::chandelier, Variant::greenDungeon}, {0, 1512}},
        {{TileID::chandelier, Variant::pinkDungeon}, {0, 1566}},
        {{TileID::chandelier, Variant::mushroom}, {54, 1296}},
        {{TileID::chandelier, Variant::obsidian}, {0, 1728}},
        {{TileID::chandelier, Variant::ashWood}, {108, 594}},
        {{TileID::chandelier, Variant::balloon}, {108, 540}},
        {{TileID::chandelier, Variant::boreal}, {0, 1350}},
        {{TileID::chandelier, Variant::forest}, {0, 0}},
        {{TileID::chandelier, Variant::granite}, {0, 1890}},
        {{TileID::chandelier, Variant::honey}, {0, 540}},
        {{TileID::chandelier, Variant::livingWood}, {0, 972}},
        {{TileID::chandelier, Variant::mahogany}, {0, 648}},
        {{TileID::chandelier, Variant::marble}, {0, 1944}},
        {{TileID::chandelier, Variant::palm}, {0, 1242}},
        {{TileID::chandelier, Variant::skyware}, {0, 810}},
        // Base: Wooden Chest
        {{TileID::chest, Variant::blueDungeon}, {1404, 0}},
        {{TileID::chest, Variant::greenDungeon}, {1260, 0}},
        {{TileID::chest, Variant::pinkDungeon}, {1332, 0}},
        {{TileID::chest, Variant::obsidian}, {1584, 0}},
        {{TileID::chest, Variant::ashWood}, {576, 0}},
        {{TileID::chest, Variant::balloon}, {540, 0}},
        {{TileID::chest, Variant::boreal}, {1188, 0}},
        {{TileID::chest, Variant::forest}, {0, 0}},
        {{TileID::chest, Variant::granite}, {1800, 0}},
        {{TileID::chest, Variant::honey}, {1044, 0}},
        {{TileID::chest, Variant::livingWood}, {432, 0}},
        {{TileID::chest, Variant::mahogany}, {288, 0}},
        {{TileID::chest, Variant::marble}, {1836, 0}},
        {{TileID::chest, Variant::palm}, {1116, 0}},
        {{TileID::chest, Variant::skyware}, {468, 0}},
        // Base: Grandfather Clock
        {{TileID::clock, Variant::blueDungeon}, {1080, 0}},
        {{TileID::clock, Variant::greenDungeon}, {1116, 0}},
        {{TileID::clock, Variant::pinkDungeon}, {1152, 0}},
        {{TileID::clock, Variant::obsidian}, {612, 0}},
        // Base: Wooden Door
        {{TileID::door, Variant::blueDungeon}, {0, 864}},
        {{TileID::door, Variant::greenDungeon}, {0, 918}},
        {{TileID::door, Variant::pinkDungeon}, {0, 972}},
        {{TileID::door, Variant::obsidian}, {0, 1026}},
        {{TileID::door, Variant::ashWood}, {54, 648}},
        {{TileID::door, Variant::balloon}, {54, 594}},
        {{TileID::door, Variant::boreal}, {0, 1620}},
        {{TileID::door, Variant::forest}, {0, 0}},
        {{TileID::door, Variant::granite}, {0, 1836}},
        {{TileID::door, Variant::honey}, {0, 1188}},
        {{TileID::door, Variant::livingWood}, {0, 378}},
        {{TileID::door, Variant::mahogany}, {0, 108}},
        {{TileID::door, Variant::marble}, {0, 1890}},
        {{TileID::door, Variant::palm}, {0, 1566}},
        {{TileID::door, Variant::skyware}, {0, 486}},
        // Base: Wooden Dresser
        {{TileID::dresser, Variant::blueDungeon}, {270, 0}},
        {{TileID::dresser, Variant::greenDungeon}, {324, 0}},
        {{TileID::dresser, Variant::pinkDungeon}, {378, 0}},
        {{TileID::dresser, Variant::mushroom}, {918, 0}},
        {{TileID::dresser, Variant::obsidian}, {486, 0}},
        // Base: Tiki Torch
        {{TileID::lamp, Variant::blueDungeon}, {0, 1296}},
        {{TileID::lamp, Variant::greenDungeon}, {0, 1350}},
        {{TileID::lamp, Variant::pinkDungeon}, {0, 1404}},
        {{TileID::lamp, Variant::mushroom}, {18, 1026}},
        {{TileID::lamp, Variant::obsidian}, {0, 1242}},
        {{TileID::lamp, Variant::ashWood}, {0, 2268}},
        {{TileID::lamp, Variant::balloon}, {0, 2214}},
        {{TileID::lamp, Variant::boreal}, {0, 1080}},
        {{TileID::lamp, Variant::forest}, {0, 216}},
        {{TileID::lamp, Variant::granite}, {0, 1566}},
        {{TileID::lamp, Variant::honey}, {0, 594}},
        {{TileID::lamp, Variant::livingWood}, {0, 702}},
        {{TileID::lamp, Variant::mahogany}, {0, 324}},
        {{TileID::lamp, Variant::marble}, {0, 1620}},
        {{TileID::lamp, Variant::palm}, {0, 972}},
        {{TileID::lamp, Variant::skyware}, {0, 486}},
        // Base: Chain Lantern
        {{TileID::lantern, Variant::blueDungeon}, {0, 36}},
        {{TileID::lantern, Variant::greenDungeon}, {0, 216}},
        {{TileID::lantern, Variant::pinkDungeon}, {0, 180}},
        {{TileID::lantern, Variant::mushroom}, {18, 1008}},
        {{TileID::lantern, Variant::obsidian}, {0, 1152}},
        {{TileID::lantern, Variant::ashWood}, {0, 1728}},
        {{TileID::lantern, Variant::balloon}, {0, 1692}},
        {{TileID::lantern, Variant::boreal}, {0, 1044}},
        {{TileID::lantern, Variant::forest}, {0, 108}},
        {{TileID::lantern, Variant::granite}, {0, 1260}},
        {{TileID::lantern, Variant::honey}, {0, 468}},
        {{TileID::lantern, Variant::livingWood}, {0, 792}},
        {{TileID::lantern, Variant::mahogany}, {0, 576}},
        {{TileID::lantern, Variant::marble}, {0, 1296}},
        {{TileID::lantern, Variant::palm}, {0, 972}},
        {{TileID::lantern, Variant::skyware}, {0, 720}},
        // Base: Wooden Piano
        {{TileID::piano, Variant::blueDungeon}, {594, 0}},
        {{TileID::piano, Variant::greenDungeon}, {648, 0}},
        {{TileID::piano, Variant::pinkDungeon}, {702, 0}},
        {{TileID::piano, Variant::mushroom}, {1188, 0}},
        {{TileID::piano, Variant::obsidian}, {810, 0}},
        // Base: Wood Platform
        {{TileID::platform, Variant::blueDungeon}, {0, 108}},
        {{TileID::platform, Variant::greenDungeon}, {0, 144}},
        {{TileID::platform, Variant::pinkDungeon}, {0, 126}},
        {{TileID::platform, Variant::mushroom}, {0, 324}},
        {{TileID::platform, Variant::obsidian}, {0, 234}},
        {{TileID::platform, Variant::ashWood}, {0, 846}},
        {{TileID::platform, Variant::balloon}, {0, 828}},
        {{TileID::platform, Variant::boreal}, {0, 342}},
        {{TileID::platform, Variant::forest}, {0, 0}},
        {{TileID::platform, Variant::granite}, {0, 504}},
        {{TileID::platform, Variant::honey}, {0, 432}},
        {{TileID::platform, Variant::livingWood}, {0, 414}},
        {{TileID::platform, Variant::mahogany}, {0, 36}},
        {{TileID::platform, Variant::marble}, {0, 522}},
        {{TileID::platform, Variant::palm}, {0, 306}},
        {{TileID::platform, Variant::skyware}, {0, 396}},
        // Base: Wooden Sink
        {{TileID::sink, Variant::blueDungeon}, {0, 380}},
        {{TileID::sink, Variant::greenDungeon}, {0, 418}},
        {{TileID::sink, Variant::pinkDungeon}, {0, 456}},
        {{TileID::sink, Variant::mushroom}, {0, 912}},
        {{TileID::sink, Variant::obsidian}, {0, 494}},
        // Base: Armor Statue
        {{TileID::statue, Variant::blueDungeon}, {1656, 0}},
        {{TileID::statue, Variant::greenDungeon}, {1692, 0}},
        {{TileID::statue, Variant::pinkDungeon}, {1728, 0}},
        {{TileID::statue, Variant::obsidian}, {1764, 0}},
        // Base: Wooden Table
        {{TileID::table, Variant::blueDungeon}, {540, 0}},
        {{TileID::table, Variant::greenDungeon}, {594, 0}},
        {{TileID::table, Variant::pinkDungeon}, {648, 0}},
        {{TileID::table, Variant::mushroom}, {1458, 0}},
        {{TileID::table, Variant::obsidian}, {702, 0}},
        {{TileID::table, Variant::ashWood}, {594, 0}},
        {{TileID::table, Variant::balloon}, {540, 0}},
        {{TileID::table, Variant::boreal}, {1512, 0}},
        {{TileID::table, Variant::forest}, {0, 0}},
        {{TileID::table, Variant::granite}, {1782, 0}},
        {{TileID::table, Variant::honey}, {1026, 0}},
        {{TileID::table, Variant::livingWood}, {324, 0}},
        {{TileID::table, Variant::mahogany}, {108, 0}},
        {{TileID::table, Variant::marble}, {1836, 0}},
        {{TileID::table, Variant::palm}, {1404, 0}},
        {{TileID::table, Variant::skyware}, {378, 0}},
        // Base: Ebonwood Toilet
        {{TileID::toilet, Variant::blueDungeon}, {0, 440}},
        {{TileID::toilet, Variant::greenDungeon}, {0, 480}},
        {{TileID::toilet, Variant::pinkDungeon}, {0, 520}},
        {{TileID::toilet, Variant::mushroom}, {0, 280}},
        {{TileID::toilet, Variant::obsidian}, {0, 560}},
        // Base: Wooden Work Bench
        {{TileID::workBench, Variant::blueDungeon}, {396, 0}},
        {{TileID::workBench, Variant::greenDungeon}, {432, 0}},
        {{TileID::workBench, Variant::pinkDungeon}, {468, 0}},
        {{TileID::workBench, Variant::mushroom}, {252, 0}},
        {{TileID::workBench, Variant::obsidian}, {504, 0}},
    });

bool convertFurniture(Tile &tile, Variant furnitureSet)
{
    auto itr = frameDetail.find(std::pair{tile.blockID, furnitureSet});
    if (itr == frameDetail.end()) {
        return false;
    }
    auto [frameX, frameY] = itr->second;
    tile.frameX += frameX;
    tile.frameY += frameY;
    if (furnitureSet == Variant::ashWood || furnitureSet == Variant::balloon) {
        if (tile.blockID == TileID::chest) {
            tile.blockID = TileID::chestGroup2;
        } else if (tile.blockID == TileID::table) {
            tile.blockID = TileID::tableGroup2;
        }
    }
    return true;
}

TileBuffer
getFurniture(int id, Variant furnitureSet, const std::vector<bool> &framedTiles)
{
    TileBuffer data{furnitureData[id].data(), framedTiles};
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &tile = data.getTile(i, j);
            if (tile.blockID == TileID::cloud) {
                continue;
            }
            if (!convertFurniture(tile, furnitureSet)) {
                tile.blockID = TileID::empty;
                tile.frameX = 0;
                tile.frameY = 0;
            }
        }
    }
    return data;
}

TileBuffer
getChainLantern(::Variant lanternStyle, const std::vector<bool> &framedTiles)
{
    switch (lanternStyle) {
    case ::Variant::alchemy:
        return {lanternData[0].data(), framedTiles};
    case ::Variant::oilRagSconce:
        return {lanternData[1].data(), framedTiles};
    case ::Variant::bone:
        return {lanternData[2].data(), framedTiles};
    default:
        return {};
    }
}

} // namespace Data
