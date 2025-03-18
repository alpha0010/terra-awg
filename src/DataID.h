#ifndef DATAID_H
#define DATAID_H

namespace TileID
{
enum {
    empty = -1,
    dirt = 0,
    stone = 1,
    grass = 2,
    ironOre = 6,
    copperOre = 7,
    goldOre = 8,
    silverOre = 9,
    demonite = 22,
    corruptGrass = 23,
    ebonstone = 25,
    clay = 40,
    sand = 53,
    ash = 57,
    mud = 59,
    jungleGrass = 60,
    mushroomGrass = 70,
    silt = 123,
    snow = 147,
    ice = 161,
    thinIce = 162,
    corruptIce = 163,
    tinOre = 166,
    leadOre = 167,
    tungstenOre = 168,
    platinumOre = 169,
    cloud = 189,
    livingWood = 191,
    leaf = 192,
    rainCloud = 196,
    asphalt = 198,
    crimsonGrass = 199,
    crimsonIce = 200,
    sunplate = 202,
    crimstone = 203,
    crimtane = 204,
    slush = 224,
    hive = 225,
    lihzahrdBrick = 226,
    crimsand = 234,
    coralstone = 315,
    marble = 367,
    granite = 368,
    sandstone = 396,
    hardenedSand = 397,
    desertFossil = 404,
    snowCloud = 460,
    shellPile = 495
};
}

namespace WallID
{
namespace Unsafe
{
enum {
    dirt = 2,
    ebonstone = 3,
    mud = 15,
    snow = 40,
    spider = 62,
    jungle = 64,
    ice = 71,
    mushroom = 80,
    crimstone = 83,
    hive = 86,
    marble = 178,
    granite = 180,
    sandstone = 187,
    livingWood = 244,
};
}
} // namespace WallID

#endif // DATAID_H
