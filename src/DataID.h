#ifndef DATAID_H
#define DATAID_H

#include <array>

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
    lifeCrystal = 12,
    demonite = 22,
    corruptGrass = 23,
    ebonstone = 25,
    altar = 26,
    orbHeart = 31,
    clay = 40,
    sand = 53,
    ash = 57,
    mud = 59,
    jungleGrass = 60,
    mushroomGrass = 70,
    ebonsand = 112,
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
    flesh = 195,
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
    hardenedEbonsand = 398,
    hardenedCrimsand = 399,
    ebonsandstone = 400,
    crimsandstone = 401,
    desertFossil = 404,
    snowCloud = 460,
    lesion = 474,
    shellPile = 495,
    corruptJungleGrass = 661,
    crimsonJungleGrass = 662
};
}

namespace WallID
{
enum { empty = 0 };

namespace Unsafe
{
enum {
    dirt = 2,
    ebonstone = 3,
    mud = 15,
    snow = 40,
    rockyDirt = 59,
    oldStone = 61,
    spider = 62,
    grass = 63,
    jungle = 64,
    flower = 65,
    corruptGrass = 69,
    ice = 71,
    mushroom = 80,
    crimsonGrass = 81,
    crimstone = 83,
    hive = 86,
    caveDirt = 170,
    roughDirt = 171,
    marble = 178,
    granite = 180,
    craggyStone = 185,
    sandstone = 187,
    corruptGrowth = 188,
    corruptMass = 189,
    corruptPustule = 190,
    corruptTendril = 191,
    crimsonCrust = 192,
    crimsonScab = 193,
    crimsonTeeth = 194,
    crimsonBlister = 195,
    layeredDirt = 196,
    crumblingDirt = 197,
    crackedDirt = 198,
    wavyDirt = 199,
    lichenStone = 204,
    leafyJungle = 205,
    ivyStone = 206,
    jungleVine = 207,
    ember = 208,
    cinder = 209,
    magma = 210,
    smoulderingStone = 211,
    wornStone = 212,
    stalactiteStone = 213,
    mottledStone = 214,
    fracturedStone = 215,
    hardenedSand = 216,
    hardenedEbonsand = 217,
    hardenedCrimsand = 218,
    ebonsandstone = 220,
    crimsandstone = 221,
    livingWood = 244,
};
}
} // namespace WallID

namespace WallVariants
{
inline std::array const dirt = {
    WallID::Unsafe::dirt,
    WallID::Unsafe::rockyDirt,
    WallID::Unsafe::caveDirt,
    WallID::Unsafe::roughDirt,
    WallID::Unsafe::layeredDirt,
    WallID::Unsafe::crumblingDirt,
    WallID::Unsafe::crackedDirt,
    WallID::Unsafe::wavyDirt};

inline std::array const stone = {
    WallID::Unsafe::oldStone,
    WallID::Unsafe::craggyStone,
    WallID::Unsafe::wornStone,
    WallID::Unsafe::stalactiteStone,
    WallID::Unsafe::mottledStone,
    WallID::Unsafe::fracturedStone};

inline std::array const jungle = {
    WallID::Unsafe::mud,
    WallID::Unsafe::lichenStone,
    WallID::Unsafe::leafyJungle,
    WallID::Unsafe::ivyStone,
    WallID::Unsafe::jungleVine};

inline std::array const crimson = {
    WallID::Unsafe::crimstone,
    WallID::Unsafe::crimsonCrust,
    WallID::Unsafe::crimsonScab,
    WallID::Unsafe::crimsonTeeth,
    WallID::Unsafe::crimsonBlister};

inline std::array const corruption = {
    WallID::Unsafe::ebonstone,
    WallID::Unsafe::corruptGrowth,
    WallID::Unsafe::corruptMass,
    WallID::Unsafe::corruptPustule,
    WallID::Unsafe::corruptTendril};

inline std::array const underworld = {
    WallID::Unsafe::ember,
    WallID::Unsafe::cinder,
    WallID::Unsafe::magma,
    WallID::Unsafe::smoulderingStone};
} // namespace WallVariants

#endif // DATAID_H
