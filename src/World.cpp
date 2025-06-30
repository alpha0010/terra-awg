#include "World.h"

#include "Config.h"
#include "Random.h"
#include "TileBuffer.h"
#include "ids/WallID.h"
#include <iostream>

constexpr std::vector<bool> genFramedTileLookup()
{
    int tileIds[] = {
        3,   4,   5,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
        21,  24,  26,  27,  28,  29,  31,  33,  34,  35,  36,  42,  49,  50,
        55,  61,  71,  72,  73,  74,  77,  78,  79,  81,  82,  83,  84,  85,
        86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
        100, 101, 102, 103, 104, 105, 106, 110, 113, 114, 125, 126, 128, 129,
        132, 133, 134, 135, 136, 137, 138, 139, 141, 142, 143, 144, 149, 165,
        171, 172, 173, 174, 178, 184, 185, 186, 187, 201, 207, 209, 210, 212,
        215, 216, 217, 218, 219, 220, 227, 228, 231, 233, 235, 236, 237, 238,
        239, 240, 241, 242, 243, 244, 245, 246, 247, 254, 269, 270, 271, 275,
        276, 277, 278, 279, 280, 281, 282, 283, 285, 286, 287, 288, 289, 290,
        291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304,
        305, 306, 307, 308, 309, 310, 314, 316, 317, 318, 319, 320, 323, 324,
        334, 335, 337, 338, 339, 349, 354, 355, 356, 358, 359, 360, 361, 362,
        363, 364, 372, 373, 374, 375, 376, 377, 378, 380, 386, 387, 388, 389,
        390, 391, 392, 393, 394, 395, 405, 406, 410, 411, 412, 413, 414, 419,
        420, 423, 424, 425, 427, 428, 429, 435, 436, 437, 438, 439, 440, 441,
        442, 443, 444, 445, 452, 453, 454, 455, 456, 457, 461, 462, 463, 464,
        465, 466, 467, 468, 469, 470, 471, 475, 476, 480, 484, 485, 486, 487,
        488, 489, 490, 491, 493, 494, 497, 499, 505, 506, 509, 510, 511, 518,
        519, 520, 521, 522, 523, 524, 525, 526, 527, 529, 530, 531, 532, 533,
        538, 542, 543, 544, 545, 547, 548, 549, 550, 551, 552, 553, 554, 555,
        556, 558, 559, 560, 564, 565, 567, 568, 569, 570, 571, 572, 573, 579,
        580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593,
        594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607,
        608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 619, 620, 621, 622,
        623, 624, 629, 630, 631, 632, 634, 637, 639, 640, 642, 643, 644, 645,
        646, 647, 648, 649, 650, 651, 652, 653, 654, 656, 657, 658, 660, 663,
        664, 665};
    std::vector<bool> table(693);
    for (int tileId : tileIds) {
        table[tileId] = true;
    }
    return table;
}

std::pair<int, int> framedTileToDims(int tileID)
{
    switch (tileID) {
    case TileID::painting2x3:
        return {2, 3};
    case TileID::painting3x2:
        return {3, 2};
    case TileID::catacomb:
        return {4, 3};
    case TileID::painting3x3:
        return {3, 3};
    case TileID::painting6x4:
        return {6, 4};
    default:
        return {1, 1};
    }
}

uint32_t fnv1a32pt(uint32_t x, uint32_t y)
{
    const uint32_t prime = 16777619;
    uint32_t hash = 2166136261;
    hash ^= x;
    hash *= prime;
    hash ^= y;
    hash *= prime;
    return hash;
}

World::World(const Config &c)
    : width(c.width), height(c.height), tiles(width * height),
      framedTiles(genFramedTileLookup()), surface(width), conf(c)
{
}

void World::initBiomeData()
{
    biomeMap.resize(width * height);
}

int World::getWidth() const
{
    return width;
}

int World::getHeight() const
{
    return height;
}

int &World::getSurfaceLevel(int x)
{
    return surface[(x + surface.size()) % surface.size()];
}

int World::getUndergroundLevel() const
{
    return 0.28 * height;
}

int World::getCavernLevel() const
{
    return 0.41 * height;
}

int World::getUnderworldLevel() const
{
    return height - 230;
}

Tile &World::getTile(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height) {
        // Handle out-of-bounds request with junk data.
        return scratchTile;
    }
    return tiles[y + x * height];
}

BiomeData &World::getBiome(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return biomeMap.front();
    }
    return biomeMap[y + x * height];
}

std::vector<std::pair<int, int>>
World::placeBuffer(int x, int y, const TileBuffer &data, Blend blendMode)
{
    std::vector<std::pair<int, int>> storageLocations;
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            const Tile &dataTile = data.getTile(i, j);
            if (blendMode == Blend::blockOnly &&
                dataTile.blockID == TileID::empty) {
                continue;
            }
            Tile &tile = getTile(x + i, y + j);
            if (dataTile.blockID == TileID::cloud) {
                if (dataTile.slope != Slope::none &&
                    tile.blockID != TileID::empty) {
                    tile.slope = dataTile.slope;
                    tile.guarded = true;
                }
                continue;
            }
            if (blendMode == Blend::blockOnly) {
                tile.blockID = dataTile.blockID;
                tile.frameX = dataTile.frameX;
                tile.frameY = dataTile.frameY;
                tile.blockPaint = dataTile.blockPaint;
                tile.slope = dataTile.slope;
            } else {
                int wallId = tile.wallID;
                int wallPaint = tile.wallPaint;
                tile = dataTile;
                if (dataTile.wallID == WallID::Safe::cloud) {
                    tile.wallID = WallID::empty;
                } else if (dataTile.wallID == WallID::empty) {
                    tile.wallID = wallId;
                    tile.wallPaint = wallPaint;
                }
            }
            tile.guarded =
                tile.blockID != TileID::empty || tile.wallID != WallID::empty;
            if (((tile.blockID == TileID::dresser && tile.frameX % 54 == 0) ||
                 ((tile.blockID == TileID::chest ||
                   tile.blockID == TileID::chestGroup2) &&
                  tile.frameX % 36 == 0)) &&
                tile.frameY == 0) {
                storageLocations.emplace_back(x + i, y + j);
            }
        }
    }
    return storageLocations;
}

struct FrameDetails {
    int width;
    int height;
    int offsetX;
    int offsetY;
};

inline const std::map<std::pair<int, Variant>, FrameDetails> tileFrameData{
    {{TileID::alchemyTable, Variant::none}, {3, 3, 0, 0}},
    {{TileID::altar, Variant::corruption}, {3, 2, 0, 0}},
    {{TileID::altar, Variant::crimson}, {3, 2, 54, 0}},
    {{TileID::ashPlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::banner, Variant::ankh}, {1, 3, 72, 0}},
    {{TileID::banner, Variant::omega}, {1, 3, 108, 0}},
    {{TileID::banner, Variant::snake}, {1, 3, 90, 0}},
    {{TileID::bewitchingTable, Variant::none}, {3, 3, 0, 0}},
    {{TileID::boneWelder, Variant::none}, {3, 3, 0, 0}},
    {{TileID::bottle, Variant::health}, {1, 1, 18, 0}},
    {{TileID::bottle, Variant::mana}, {1, 1, 36, 0}},
    {{TileID::chest, Variant::barrel}, {2, 2, 180, 0}},
    {{TileID::chest, Variant::corruption}, {2, 2, 864, 0}},
    {{TileID::chest, Variant::crimson}, {2, 2, 900, 0}},
    {{TileID::chest, Variant::flesh}, {2, 2, 1548, 0}},
    {{TileID::chest, Variant::frozen}, {2, 2, 396, 0}},
    {{TileID::chest, Variant::gold}, {2, 2, 36, 0}},
    {{TileID::chest, Variant::goldLocked}, {2, 2, 72, 0}},
    {{TileID::chest, Variant::granite}, {2, 2, 1800, 0}},
    {{TileID::chest, Variant::hallowed}, {2, 2, 936, 0}},
    {{TileID::chest, Variant::honey}, {2, 2, 1044, 0}},
    {{TileID::chest, Variant::ice}, {2, 2, 972, 0}},
    {{TileID::chest, Variant::ivy}, {2, 2, 360, 0}},
    {{TileID::chest, Variant::jungle}, {2, 2, 828, 0}},
    {{TileID::chest, Variant::lihzahrd}, {2, 2, 576, 0}},
    {{TileID::chest, Variant::livingWood}, {2, 2, 432, 0}},
    {{TileID::chest, Variant::marble}, {2, 2, 1836, 0}},
    {{TileID::chest, Variant::meteorite}, {2, 2, 1764, 0}},
    {{TileID::chest, Variant::mushroom}, {2, 2, 1152, 0}},
    {{TileID::chest, Variant::palmWood}, {2, 2, 1116, 0}},
    {{TileID::chest, Variant::pearlwood}, {2, 2, 324, 0}},
    {{TileID::chest, Variant::richMahogany}, {2, 2, 288, 0}},
    {{TileID::chest, Variant::shadow}, {2, 2, 144, 0}},
    {{TileID::chest, Variant::skyware}, {2, 2, 468, 0}},
    {{TileID::chest, Variant::water}, {2, 2, 612, 0}},
    {{TileID::chestGroup2, Variant::ashWood}, {2, 2, 576, 0}},
    {{TileID::chestGroup2, Variant::deadMans}, {2, 2, 144, 0}},
    {{TileID::chestGroup2, Variant::desert}, {2, 2, 468, 0}},
    {{TileID::chestGroup2, Variant::lesion}, {2, 2, 108, 0}},
    {{TileID::chestGroup2, Variant::reef}, {2, 2, 504, 0}},
    {{TileID::chestGroup2, Variant::sandstone}, {2, 2, 360, 0}},
    {{TileID::corruptPlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::crimsonPlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::door, Variant::dungeon}, {1, 3, 0, 702}},
    {{TileID::door, Variant::lihzahrd}, {1, 3, 0, 594}},
    {{TileID::door, Variant::mushroom}, {1, 3, 0, 324}},
    {{TileID::door, Variant::obsidian}, {1, 3, 0, 1026}},
    {{TileID::fallenLog, Variant::none}, {3, 2, 0, 0}},
    {{TileID::grassPlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::geyser, Variant::none}, {2, 1, 0, 0}},
    {{TileID::hellforge, Variant::none}, {3, 2, 0, 0}},
    {{TileID::junglePlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::lamp, Variant::crystal}, {1, 3, 0, 1674}},
    {{TileID::lamp, Variant::dynasty}, {1, 3, 0, 918}},
    {{TileID::lamp, Variant::flesh}, {1, 3, 0, 162}},
    {{TileID::lamp, Variant::frozen}, {1, 3, 0, 270}},
    {{TileID::lamp, Variant::lesion}, {1, 3, 0, 1782}},
    {{TileID::lamp, Variant::lihzahrd}, {1, 3, 0, 432}},
    {{TileID::lamp, Variant::sandstone}, {1, 3, 0, 2052}},
    {{TileID::lantern, Variant::alchemy}, {1, 2, 0, 144}},
    {{TileID::lantern, Variant::oilRagSconce}, {1, 2, 0, 216}},
    {{TileID::lantern, Variant::bone}, {1, 2, 18, 900}},
    {{TileID::largePile, Variant::bone}, {3, 2, 0, 0}},
    {{TileID::largePile, Variant::furniture}, {3, 2, 1188, 0}},
    {{TileID::largePile, Variant::ice}, {3, 2, 1404, 0}},
    {{TileID::largePile, Variant::mushroom}, {3, 2, 1728, 0}},
    {{TileID::largePile, Variant::stone}, {3, 2, 378, 0}},
    {{TileID::largePileGroup2, Variant::ash}, {3, 2, 324, 0}},
    {{TileID::largePileGroup2, Variant::dryBone}, {3, 2, 2808, 0}},
    {{TileID::largePileGroup2, Variant::forest}, {3, 2, 756, 0}},
    {{TileID::largePileGroup2, Variant::granite}, {3, 2, 1890, 0}},
    {{TileID::largePileGroup2, Variant::jungle}, {3, 2, 0, 0}},
    {{TileID::largePileGroup2, Variant::lihzahrd}, {3, 2, 972, 0}},
    {{TileID::largePileGroup2, Variant::livingWood}, {3, 2, 2538, 0}},
    {{TileID::largePileGroup2, Variant::livingLeaf}, {3, 2, 2700, 0}},
    {{TileID::largePileGroup2, Variant::marble}, {3, 2, 2214, 0}},
    {{TileID::largePileGroup2, Variant::sandstone}, {3, 2, 1566, 0}},
    {{TileID::largePileGroup2, Variant::spider}, {3, 2, 486, 0}},
    {{TileID::larva, Variant::none}, {3, 3, 0, 0}},
    {{TileID::lihzahrdAltar, Variant::none}, {3, 2, 0, 0}},
    {{TileID::loom, Variant::none}, {3, 2, 0, 0}},
    {{TileID::mushroomPlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::orbHeart, Variant::crimson}, {2, 2, 36, 0}},
    {{TileID::pot, Variant::corruption}, {2, 2, 0, 576}},
    {{TileID::pot, Variant::crimson}, {2, 2, 0, 792}},
    {{TileID::pot, Variant::desert}, {2, 2, 0, 1224}},
    {{TileID::pot, Variant::dungeon}, {2, 2, 0, 360}},
    {{TileID::pot, Variant::jungle}, {2, 2, 0, 252}},
    {{TileID::pot, Variant::lihzahrd}, {2, 2, 0, 1008}},
    {{TileID::pot, Variant::marble}, {2, 2, 0, 1116}},
    {{TileID::pot, Variant::pyramid}, {2, 2, 0, 900}},
    {{TileID::pot, Variant::spider}, {2, 2, 0, 684}},
    {{TileID::pot, Variant::tundra}, {2, 2, 0, 144}},
    {{TileID::pot, Variant::underworld}, {2, 2, 0, 468}},
    {{TileID::pressurePlate, Variant::lihzahrd}, {1, 1, 0, 108}},
    {{TileID::sharpeningStation, Variant::none}, {3, 2, 0, 0}},
    {{TileID::smallPile, Variant::bone}, {2, 1, 216, 0}},
    {{TileID::smallPile, Variant::dirt}, {1, 1, 108, 0}},
    {{TileID::smallPile, Variant::forest}, {2, 1, 1368, 18}},
    {{TileID::smallPile, Variant::furniture}, {2, 1, 1116, 18}},
    {{TileID::smallPile, Variant::gold}, {2, 1, 648, 18}},
    {{TileID::smallPile, Variant::granite}, {2, 1, 0, 0}},
    {{TileID::smallPile, Variant::ice}, {2, 1, 0, 0}},
    {{TileID::smallPile, Variant::livingWood}, {2, 1, 2124, 18}},
    {{TileID::smallPile, Variant::marble}, {2, 1, 0, 0}},
    {{TileID::smallPile, Variant::sandstone}, {2, 1, 0, 0}},
    {{TileID::smallPile, Variant::spider}, {2, 1, 0, 0}},
    {{TileID::smallPile, Variant::stone}, {2, 1, 0, 0}},
    {{TileID::statue, Variant::lihzahrd}, {2, 3, 1548, 0}},
    {{TileID::sunflower, Variant::none}, {2, 4, 0, 0}},
    {{TileID::tallGrassPlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::tallJunglePlant, Variant::none}, {1, 1, 0, 0}},
    {{TileID::trap, Variant::dartLeft}, {1, 1, 0, 0}},
    {{TileID::trap, Variant::dartRight}, {1, 1, 18, 0}},
    {{TileID::trap, Variant::flameLeft}, {1, 1, 0, 36}},
    {{TileID::trap, Variant::flameRight}, {1, 1, 18, 36}},
    {{TileID::trap, Variant::spear}, {1, 1, 0, 72}},
    {{TileID::trap, Variant::spikyBall}, {1, 1, 0, 54}},
    {{TileID::trap, Variant::superDartLeft}, {1, 1, 0, 18}},
    {{TileID::trap, Variant::superDartRight}, {1, 1, 18, 18}},
};

void World::placeFramedTile(int x, int y, int blockID, Variant type)
{
    if (blockID == TileID::chest &&
        (type == Variant::ashWood || type == Variant::crystal ||
         type == Variant::deadMans || type == Variant::desert ||
         type == Variant::lesion || type == Variant::reef ||
         type == Variant::sandstone)) {
        blockID = TileID::chestGroup2;
    }
    int offsetX = 0;
    int offsetY = 0;
    int frameWidth = 2;
    int frameHeight = 2;
    auto itr = tileFrameData.find({blockID, type});
    if (itr != tileFrameData.end()) {
        const FrameDetails &data = itr->second;
        offsetX = data.offsetX;
        offsetY = data.offsetY;
        frameWidth = data.width;
        frameHeight = data.height;
    }
    uint32_t coordHash = fnv1a32pt(x, y);
    if (blockID == TileID::pot) {
        // Cycle pot variations based on world coordinates.
        switch (type) {
        case Variant::crimson:
        case Variant::marble:
        case Variant::pyramid:
            offsetY += 36 * (coordHash % 3);
            break;
        case Variant::forest:
            offsetX += 36 * (coordHash % 3);
            offsetY += 36 * (coordHash % 4);
            break;
        case Variant::lihzahrd:
            offsetX += 36 * (coordHash % 2);
            offsetY += 36 * (coordHash % 3);
            break;
        default:
            offsetX += 36 * (coordHash % 3);
            offsetY += 36 * (coordHash % 3);
        }
    } else if (blockID == TileID::statue && type == Variant::lihzahrd) {
        offsetX += 36 * (coordHash % 3);
        offsetY += 162 * (coordHash % 2);
    } else if (blockID == TileID::smallPile) {
        if (type == Variant::forest && coordHash % 41 > 30) {
            type = Variant::stone;
        }
        switch (type) {
        case Variant::bone: {
            int mod = coordHash % 26;
            if (mod < 16) {
                frameWidth = 1;
                offsetX += 18 * mod;
            } else {
                offsetX += 36 * (mod - 16);
                offsetY = 18;
            }
        } break;
        case Variant::dirt:
            offsetX += 18 * (coordHash % 6);
            break;
        case Variant::forest:
        case Variant::furniture:
        case Variant::livingWood:
            offsetX += 36 * (coordHash % 3);
            break;
        case Variant::granite: {
            int mod = coordHash % 12;
            if (mod < 6) {
                frameWidth = 1;
                offsetX = 1080 + 18 * mod;
            } else {
                offsetX = 1692 + 36 * (mod - 6);
                offsetY = 18;
            }
        } break;
        case Variant::ice: {
            int mod = coordHash % 18;
            if (mod < 12) {
                frameWidth = 1;
                offsetX = 648 + 18 * mod;
            } else {
                offsetX = 900 + 36 * (mod - 12);
                offsetY = 18;
            }
        } break;
        case Variant::marble: {
            int mod = coordHash % 12;
            if (mod < 6) {
                frameWidth = 1;
                offsetX = 1188 + 18 * mod;
            } else {
                offsetX = 1908 + 36 * (mod - 6);
                offsetY = 18;
            }
        } break;
        case Variant::sandstone: {
            int mod = coordHash % 12;
            if (mod < 6) {
                frameWidth = 1;
                offsetX = 972 + 18 * mod;
            } else {
                offsetX = 1476 + 36 * (mod - 6);
                offsetY = 18;
            }
        } break;
        case Variant::spider: {
            int mod = coordHash % 10;
            if (mod < 6) {
                frameWidth = 1;
                offsetX = 864 + 18 * mod;
            } else {
                offsetX = 1224 + 36 * (mod - 6);
                offsetY = 18;
            }
        } break;
        case Variant::stone: {
            int mod = coordHash % 12;
            if (mod < 6) {
                frameWidth = 1;
                offsetX = 18 * mod;
            } else {
                offsetX = 36 * (mod - 6);
                offsetY = 18;
            }
        } break;
        default:
            break;
        }
    } else if (
        blockID == TileID::largePile || blockID == TileID::largePileGroup2) {
        switch (type) {
        case Variant::ash:
        case Variant::dryBone:
        case Variant::forest:
        case Variant::lihzahrd:
        case Variant::livingWood:
        case Variant::mushroom:
            offsetX += 54 * (coordHash % 3);
            break;
        case Variant::bone:
        case Variant::granite:
        case Variant::ice:
        case Variant::jungle:
        case Variant::marble:
        case Variant::sandstone:
        case Variant::stone:
            offsetX += 54 * (coordHash % 6);
            break;
        case Variant::furniture:
            offsetX += 54 * (coordHash % 4);
            break;
        case Variant::livingLeaf:
            offsetX += 54 * (coordHash % 2);
            break;
        case Variant::spider:
            offsetX += 54 * (coordHash % 5);
            break;
        default:
            break;
        }
    } else {
        switch (blockID) {
        case TileID::ashPlant:
            offsetX += 18 * (coordHash % 11);
            break;
        case TileID::corruptPlant:
        case TileID::crimsonPlant:
            offsetX += 18 * (coordHash % 23);
            break;
        case TileID::grassPlant:
        case TileID::tallGrassPlant: {
            int mod = coordHash % (coordHash % 7 == 1 ? 45 : 6);
            if (mod == 8 && blockID == TileID::tallGrassPlant) {
                mod = coordHash % 6;
            }
            offsetX += 18 * mod;
        } break;
        case TileID::junglePlant: {
            int mod = coordHash % (coordHash % 7 == 1 ? 23 : 6);
            if ((mod == 8 && y < getCavernLevel()) ||
                (mod == 9 && y < getUndergroundLevel())) {
                mod = coordHash % 6;
            }
            offsetX += 18 * mod;
        } break;
        case TileID::largeJunglePlant: {
            int mod = coordHash % 21;
            if (mod > 8) {
                offsetX = 36 * (mod - 9);
                offsetY = 36;
            } else {
                frameWidth = 3;
                offsetX += 54 * mod;
            }
        } break;
        case TileID::mushroomPlant:
            offsetX += 18 * (coordHash % 5);
            break;
        case TileID::tallJunglePlant: {
            int mod = coordHash % (coordHash % 7 == 1 ? 17 : 6);
            if (mod == 8) {
                mod = coordHash % 6;
            }
            offsetX += 18 * mod;
        } break;
        case TileID::sunflower:
            offsetX += 36 * (coordHash % 3);
            break;
        case TileID::tombstone:
            offsetX += 36 * (coordHash % 11);
            break;
        }
    }
    for (int i = 0; i < frameWidth; ++i) {
        for (int j = 0; j < frameHeight; ++j) {
            Tile &tile = getTile(x + i, y + j);
            tile.blockID = blockID;
            tile.frameX = 18 * i + offsetX;
            tile.frameY = 18 * j + offsetY;
        }
    }
}

struct PaintingDetails {
    int blockID;
    int offsetX;
    int offsetY;
};

inline const std::map<Painting, PaintingDetails> paintingData{
    {Painting::americanExplosive, {TileID::painting2x3, 180, 0}},
    {Painting::darkness, {TileID::painting2x3, 36, 0}},
    {Painting::darkSoulReaper, {TileID::painting2x3, 72, 0}},
    {Painting::gloriousNight, {TileID::painting2x3, 216, 0}},
    {Painting::happyLittleTree, {TileID::painting2x3, 684, 0}},
    {Painting::land, {TileID::painting2x3, 108, 0}},
    {Painting::secrets, {TileID::painting2x3, 756, 0}},
    {Painting::strangeDeadFellows, {TileID::painting2x3, 720, 0}},
    {Painting::strangeGrowth, {TileID::painting2x3, 540, 0}},
    {Painting::sufficientlyAdvanced, {TileID::painting2x3, 504, 0}},
    {Painting::trappedGhost, {TileID::painting2x3, 144, 0}},
    {Painting::auroraBorealis, {TileID::painting3x2, 0, 1296}},
    {Painting::bifrost, {TileID::painting3x2, 0, 1188}},
    {Painting::bioluminescence, {TileID::painting3x2, 0, 1080}},
    {Painting::demonsEye, {TileID::painting3x2, 0, 0}},
    {Painting::livingGore, {TileID::painting3x2, 0, 576}},
    {Painting::findingGold, {TileID::painting3x2, 0, 36}},
    {Painting::flowingMagma, {TileID::painting3x2, 0, 612}},
    {Painting::forestTroll, {TileID::painting3x2, 0, 1260}},
    {Painting::heartlands, {TileID::painting3x2, 0, 1224}},
    {Painting::vikingVoyage, {TileID::painting3x2, 0, 1152}},
    {Painting::wildflowers, {TileID::painting3x2, 0, 1116}},
    {Painting::aHorribleNightForAlchemy, {TileID::painting3x3, 324, 108}},
    {Painting::bloodMoonRising, {TileID::painting3x3, 648, 0}},
    {Painting::boneWarp, {TileID::painting3x3, 810, 0}},
    {Painting::catSword, {TileID::painting3x3, 540, 108}},
    {Painting::crownoDevoursHisLunch, {TileID::painting3x3, 1836, 0}},
    {Painting::discover, {TileID::painting3x3, 1404, 0}},
    {Painting::fairyGuides, {TileID::painting3x3, 270, 108}},
    {Painting::fatherOfSomeone, {TileID::painting3x3, 1296, 0}},
    {Painting::gloryOfTheFire, {TileID::painting3x3, 756, 0}},
    {Painting::guidePicasso, {TileID::painting3x3, 1188, 0}},
    {Painting::handEarth, {TileID::painting3x3, 1458, 0}},
    {Painting::hangingSkeleton, {TileID::painting3x3, 918, 0}},
    {Painting::impFace, {TileID::painting3x3, 1620, 0}},
    {Painting::morningHunt, {TileID::painting3x3, 378, 108}},
    {Painting::nurseLisa, {TileID::painting3x3, 1350, 0}},
    {Painting::oldMiner, {TileID::painting3x3, 1512, 0}},
    {Painting::ominousPresence, {TileID::painting3x3, 1674, 0}},
    {Painting::outcast, {TileID::painting3x3, 216, 108}},
    {Painting::rareEnchantment, {TileID::painting3x3, 1890, 0}},
    {Painting::shiningMoon, {TileID::painting3x3, 1728, 0}},
    {Painting::skelehead, {TileID::painting3x3, 1566, 0}},
    {Painting::skellingtonJSkellingsworth, {TileID::painting3x3, 972, 0}},
    {Painting::sunflowers, {TileID::painting3x3, 1080, 0}},
    {Painting::terrarianGothic, {TileID::painting3x3, 1134, 0}},
    {Painting::theCursedMan, {TileID::painting3x3, 1026, 0}},
    {Painting::theGuardiansGaze, {TileID::painting3x3, 1242, 0}},
    {Painting::theHangedMan, {TileID::painting3x3, 702, 0}},
    {Painting::theMerchant, {TileID::painting3x3, 1782, 0}},
    {Painting::wallSkeleton, {TileID::painting3x3, 864, 0}},
    {Painting::catacomb1, {TileID::catacomb, 0, 0}},
    {Painting::catacomb2, {TileID::catacomb, 0, 54}},
    {Painting::catacomb3, {TileID::catacomb, 0, 108}},
    {Painting::catacomb4, {TileID::catacomb, 0, 162}},
    {Painting::catacomb5, {TileID::catacomb, 0, 216}},
    {Painting::catacomb6, {TileID::catacomb, 0, 270}},
    {Painting::catacomb7, {TileID::catacomb, 0, 324}},
    {Painting::catacomb8, {TileID::catacomb, 0, 378}},
    {Painting::catacomb9, {TileID::catacomb, 0, 432}},
    {Painting::ancientTablet, {TileID::painting6x4, 108, 936}},
    {Painting::dryadisque, {TileID::painting6x4, 0, 360}},
    {Painting::facingTheCerebralMastermind, {TileID::painting6x4, 0, 936}},
    {Painting::goblinsPlayingPoker, {TileID::painting6x4, 0, 288}},
    {Painting::greatWave, {TileID::painting6x4, 0, 792}},
    {Painting::impact, {TileID::painting6x4, 0, 432}},
    {Painting::lakeOfFire, {TileID::painting6x4, 0, 1008}},
    {Painting::lifeAboveTheSand, {TileID::painting6x4, 108, 720}},
    {Painting::oasis, {TileID::painting6x4, 108, 792}},
    {Painting::poweredByBirds, {TileID::painting6x4, 0, 504}},
    {Painting::somethingEvilIsWatchingYou, {TileID::painting6x4, 0, 72}},
    {Painting::sparky, {TileID::painting6x4, 108, 216}},
    {Painting::starryNight, {TileID::painting6x4, 0, 864}},
    {Painting::theCreationOfTheGuide, {TileID::painting6x4, 0, 1152}},
    {Painting::theDestroyer, {TileID::painting6x4, 0, 576}},
    {Painting::theEyeSeesTheEnd, {TileID::painting6x4, 0, 0}},
    {Painting::thePersistencyOfEyes, {TileID::painting6x4, 0, 648}},
    {Painting::theScreamer, {TileID::painting6x4, 0, 216}},
    {Painting::theTwinsHaveAwoken, {TileID::painting6x4, 0, 144}},
    {Painting::trioSuperHeroes, {TileID::painting6x4, 0, 1080}},
    {Painting::unicornCrossingTheHallows, {TileID::painting6x4, 0, 720}},
};

void World::placePainting(int x, int y, Painting painting)
{
    auto itr = paintingData.find(painting);
    if (itr == paintingData.end()) {
        std::cout << "Failed to find painting data\n";
        return;
    }
    auto [blockID, offsetX, offsetY] = itr->second;
    auto [frameWidth, frameHeight] = framedTileToDims(blockID);
    if (painting == Painting::strangeGrowth) {
        offsetX += 36 * (fnv1a32pt(x, y) % 4);
    }
    for (int i = 0; i < frameWidth; ++i) {
        for (int j = 0; j < frameHeight; ++j) {
            Tile &tile = getTile(x + i, y + j);
            tile.blockID = blockID;
            tile.frameX = 18 * i + offsetX;
            tile.frameY = 18 * j + offsetY;
        }
    }
}

std::pair<int, int> World::getPaintingDims(Painting painting)
{
    auto itr = paintingData.find(painting);
    if (itr == paintingData.end()) {
        return {0, 0};
    }
    return framedTileToDims(itr->second.blockID);
}

Chest &World::placeChest(int x, int y, Variant type)
{
    placeFramedTile(x, y, TileID::chest, type);
    return chests.emplace_back(x, y);
}

Chest &World::registerStorage(int x, int y)
{
    return chests.emplace_back(x, y);
}

bool World::isExposed(int x, int y) const
{
    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1) {
        return false;
    }
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            int tileId = tiles[(y + j) + (x + i) * height].blockID;
            if (tileId == TileID::empty || tileId == TileID::minecartTrack) {
                return true;
            }
        }
    }
    return false;
}

bool World::isIsolated(int x, int y) const
{
    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1) {
        return false;
    }
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            if (tiles[(y + j) + (x + i) * height].blockID != TileID::empty) {
                return false;
            }
        }
    }
    return true;
}

void World::planBiomes(Random &rnd)
{
    std::cout << "Planning biomes\n";
    for (int tries = 0; tries < 1000; ++tries) {
        desertCenter = rnd.getDouble(0.09, 0.91);
        jungleCenter = rnd.getDouble(0.12, 0.39);
        if (rnd.getBool()) {
            jungleCenter = 1 - jungleCenter;
        }
        snowCenter = rnd.getDouble(0.12, 0.88);

        if (std::abs(desertCenter - jungleCenter) >
                0.075 * std::lerp(
                            conf.desertSize + conf.jungleSize,
                            2.0,
                            tries / 1000.0) &&
            std::abs(desertCenter - snowCenter) >
                0.075 * std::lerp(
                            conf.desertSize + conf.snowSize,
                            2.0,
                            tries / 1000.0) &&
            std::abs(snowCenter - jungleCenter) >
                0.075 * std::lerp(
                            conf.snowSize + conf.jungleSize,
                            2.0,
                            tries / 1000.0)) {
            break;
        }
    }
    desertCenter *= width;
    jungleCenter *= width;
    snowCenter *= width;
}
