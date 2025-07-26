#ifndef WALLID_H
#define WALLID_H

#include <array>

namespace WallID
{
enum { empty = 0 };

namespace Safe
{
enum {
    stone = 1,
    wood = 4,
    grayBrick = 5,
    redBrick = 6,
    goldBrick = 10,
    obsidianBrick = 20,
    pearlstoneBrick = 22,
    iridescentBrick = 23,
    mudstoneBrick = 24,
    planked = 27,
    snowBrick = 31,
    demoniteBrick = 33,
    sandstoneBrick = 34,
    ebonstoneBrick = 35,
    richMahogany = 42,
    rainbowBrick = 44,
    tinBrick = 45,
    platinumBrick = 47,
    livingLeaf = 60,
    cloud = 73,
    livingWood = 78,
    disc = 82,
    iceBrick = 84,
    shadewood = 85,
    redStainedGlass = 92,
    multicoloredStainedGlass = 93,
    leadFence = 107,
    hive = 108,
    titanstone = 111,
    hay = 114,
    spookyWood = 115,
    fancyGrayWallpaper = 126,
    richMahoganyFence = 139,
    whiteDynasty = 142,
    blueDynasty = 143,
    stoneSlab = 147,
    sail = 148,
    borealWood = 149,
    palmWood = 151,
    palmWoodFence = 152,
    amberGemspark = 153,
    amethystGemspark = 154,
    emeraldGemspark = 156,
    rubyGemspark = 164,
    sapphireGemspark = 165,
    topazGemspark = 166,
    honeyfall = 172,
    crimtaneBrick = 174,
    smoothMarble = 179,
    smoothGranite = 181,
    marble = 183,
    granite = 184,
    crystalBlock = 186,
    sillyPinkBalloon = 228,
    sillyPurpleBalloon = 229,
    sillyGreenBalloon = 230,
    ironBrick = 231,
    crimstoneBrick = 234,
    smoothSandstone = 235,
    snow = 249,
    ice = 266,
    obsidian = 267,
    sandstone = 275,
    hallowedPrism = 288,
    hallowedCrystalline = 291,
    lichenStone = 292,
    leafyJungle = 293,
    ember = 296,
    wornStone = 300,
    stalactiteStone = 301,
    hardenedSand = 304,
    bamboo = 312,
    largeBamboo = 313,
    ashWood = 316,
    aetherium = 321,
    aetheriumBrick = 322,
    lunarRustBrick = 323,
    darkCelestialBrick = 324,
    cryocoreBrick = 327,
    argonMossBrick = 342,
    kryptonMossBrick = 343,
    neonMossBrick = 345,
};
}

namespace Unsafe
{
enum {
    dirt = 2,
    ebonstone = 3,
    blueBrick = 7,
    greenBrick = 8,
    pinkBrick = 9,
    hellstoneBrick = 13,
    obsidianBrick = 14,
    mud = 15,
    snow = 40,
    amethystStone = 48,
    topazStone = 49,
    sapphireStone = 50,
    emeraldStone = 51,
    rubyStone = 52,
    diamondStone = 53,
    greenMossy = 54,
    brownMossy = 55,
    redMossy = 56,
    blueMossy = 57,
    purpleMossy = 58,
    rockyDirt = 59,
    oldStone = 61,
    spider = 62,
    grass = 63,
    jungle = 64,
    flower = 65,
    corruptGrass = 69,
    hallowedGrass = 70,
    ice = 71,
    mushroom = 80,
    crimsonGrass = 81,
    crimstone = 83,
    hive = 86,
    lihzahrdBrick = 87,
    blueSlab = 94,
    blueTiled = 95,
    pinkSlab = 96,
    pinkTiled = 97,
    greenSlab = 98,
    greenTiled = 99,
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
    hallowedPrism = 200,
    hallowedCavern = 201,
    hallowedShard = 202,
    hallowedCrystalline = 203,
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
    hardenedPearlsand = 219,
    ebonsandstone = 220,
    crimsandstone = 221,
    pearlsandstone = 222,
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

// Excludes: oldStone
inline std::array const stone = {
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

// Excludes: crimsonBlister
inline std::array const crimson = {
    WallID::Unsafe::crimstone,
    WallID::Unsafe::crimsonCrust,
    WallID::Unsafe::crimsonScab,
    WallID::Unsafe::crimsonTeeth};

// Excludes: corruptTendril
inline std::array const corruption = {
    WallID::Unsafe::ebonstone,
    WallID::Unsafe::corruptGrowth,
    WallID::Unsafe::corruptMass,
    WallID::Unsafe::corruptPustule};

inline std::array const hallow = {
    WallID::Unsafe::hallowedPrism,
    WallID::Unsafe::hallowedCavern,
    WallID::Unsafe::hallowedShard,
    WallID::Unsafe::hallowedCrystalline};

inline std::array const dungeon = {
    WallID::Unsafe::blueBrick,
    WallID::Unsafe::greenBrick,
    WallID::Unsafe::pinkBrick,
    WallID::Unsafe::blueSlab,
    WallID::Unsafe::greenSlab,
    WallID::Unsafe::pinkSlab,
    WallID::Unsafe::blueTiled,
    WallID::Unsafe::greenTiled,
    WallID::Unsafe::pinkTiled};

inline std::array const underworld = {
    WallID::Unsafe::ember,
    WallID::Unsafe::cinder,
    WallID::Unsafe::magma,
    WallID::Unsafe::smoulderingStone};
} // namespace WallVariants

#endif // WALLID_H
