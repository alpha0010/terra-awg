#include "World.h"

#include "Random.h"
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

World::World()
    : width(6400), height(1800), tiles(width * height),
      framedTiles(genFramedTileLookup())
{
}

int World::getWidth() const
{
    return width;
}

int World::getHeight() const
{
    return height;
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
    {{TileID::bewitchingTable, Variant::none}, {3, 3, 0, 0}},
    {{TileID::boneWelder, Variant::none}, {3, 3, 0, 0}},
    {{TileID::bottle, Variant::health}, {1, 1, 18, 0}},
    {{TileID::bottle, Variant::mana}, {1, 1, 36, 0}},
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
    {{TileID::chest, Variant::richMahogany}, {2, 2, 288, 0}},
    {{TileID::chest, Variant::shadow}, {2, 2, 144, 0}},
    {{TileID::chest, Variant::water}, {2, 2, 612, 0}},
    {{TileID::chestGroup2, Variant::desert}, {2, 2, 468, 0}},
    {{TileID::chestGroup2, Variant::lesion}, {2, 2, 108, 0}},
    {{TileID::chestGroup2, Variant::reef}, {2, 2, 504, 0}},
    {{TileID::chestGroup2, Variant::sandstone}, {2, 2, 360, 0}},
    {{TileID::door, Variant::dungeon}, {1, 3, 0, 702}},
    {{TileID::door, Variant::lihzahrd}, {1, 3, 0, 594}},
    {{TileID::lamp, Variant::crystal}, {1, 3, 0, 1674}},
    {{TileID::lamp, Variant::dynasty}, {1, 3, 0, 918}},
    {{TileID::lamp, Variant::flesh}, {1, 3, 0, 162}},
    {{TileID::lamp, Variant::frozen}, {1, 3, 0, 270}},
    {{TileID::lamp, Variant::lesion}, {1, 3, 0, 1782}},
    {{TileID::lamp, Variant::lihzahrd}, {1, 3, 0, 432}},
    {{TileID::lamp, Variant::sandstone}, {1, 3, 0, 2052}},
    {{TileID::larva, Variant::none}, {3, 3, 0, 0}},
    {{TileID::lihzahrdAltar, Variant::none}, {3, 2, 0, 0}},
    {{TileID::orbHeart, Variant::crimson}, {2, 2, 36, 0}},
    {{TileID::pot, Variant::corruption}, {2, 2, 0, 576}},
    {{TileID::pot, Variant::crimson}, {2, 2, 0, 792}},
    {{TileID::pot, Variant::desert}, {2, 2, 0, 1224}},
    {{TileID::pot, Variant::dungeon}, {2, 2, 0, 360}},
    {{TileID::pot, Variant::jungle}, {2, 2, 0, 252}},
    {{TileID::pot, Variant::lihzahrd}, {2, 2, 0, 1008}},
    {{TileID::pot, Variant::marble}, {2, 2, 0, 1116}},
    {{TileID::pot, Variant::spider}, {2, 2, 0, 684}},
    {{TileID::pot, Variant::tundra}, {2, 2, 0, 144}},
    {{TileID::pot, Variant::underworld}, {2, 2, 0, 468}},
    {{TileID::statue, Variant::lihzahrd}, {2, 3, 1548, 0}},
};

void World::placeFramedTile(int x, int y, int blockID, Variant type)
{
    if (blockID == TileID::chest &&
        (type == Variant::desert || type == Variant::lesion ||
         type == Variant::reef || type == Variant::sandstone)) {
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
    if (blockID == TileID::pot) {
        // Cycle pot variations based on world coordinates.
        switch (type) {
        case Variant::crimson:
        case Variant::marble:
            offsetY += 36 * ((x + y) % 3);
            break;
        case Variant::forest:
            offsetX += 36 * (y % 3);
            offsetY += 36 * (x % 4);
            break;
        case Variant::lihzahrd:
            offsetX += 36 * (y % 2);
            offsetY += 36 * (x % 3);
            break;
        default:
            offsetX += 36 * (y % 3);
            offsetY += 36 * (x % 3);
        }
    } else if (blockID == TileID::statue && type == Variant::lihzahrd) {
        offsetX += 36 * ((x + y) % 3);
        offsetY += 162 * ((x + y) % 2);
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

Chest &World::placeChest(int x, int y, Variant type)
{
    placeFramedTile(x, y, TileID::chest, type);
    return chests.emplace_back(x, y);
}

bool World::isExposed(int x, int y) const
{
    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1) {
        return false;
    }
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (tiles[(y + j) + (x + i) * height].blockID == TileID::empty) {
                return true;
            }
        }
    }
    return false;
}

void World::planBiomes(Random &rnd)
{
    std::cout << "Planning biomes\n";
    while (true) {
        desertCenter = rnd.getDouble(0.09, 0.91);
        jungleCenter = rnd.getDouble(0.12, 0.39);
        if (rnd.getBool()) {
            jungleCenter = 1 - jungleCenter;
        }
        snowCenter = rnd.getDouble(0.12, 0.88);

        if (std::abs(desertCenter - jungleCenter) > 0.15 &&
            std::abs(desertCenter - snowCenter) > 0.15 &&
            std::abs(snowCenter - jungleCenter) > 0.15) {
            break;
        }
    }
    desertCenter *= width;
    jungleCenter *= width;
    snowCenter *= width;
}
