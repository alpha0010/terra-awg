#include "World.h"

#include "Random.h"
#include <iostream>

World::World() : width(6400), height(1800), tiles(width * height) {}

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
    {{TileID::chest, Variant::jungle}, {2, 2, 828, 0}},
    {{TileID::chest, Variant::lihzahrd}, {2, 2, 576, 0}},
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
    {{TileID::lamp, Variant::crystal}, {1, 3, 0, 1674}},
    {{TileID::lamp, Variant::dynasty}, {1, 3, 0, 918}},
    {{TileID::lamp, Variant::flesh}, {1, 3, 0, 162}},
    {{TileID::lamp, Variant::frozen}, {1, 3, 0, 270}},
    {{TileID::lamp, Variant::lesion}, {1, 3, 0, 1782}},
    {{TileID::lamp, Variant::sandstone}, {1, 3, 0, 2052}},
    {{TileID::larva, Variant::none}, {3, 3, 0, 0}},
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
