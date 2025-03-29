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

void World::placeFramedTile(int x, int y, int blockID, Variant type)
{
    int offsetX = 0;
    int offsetY = 0;
    int frameWidth = 2;
    int frameHeight = 2;
    switch (type) {
    case Variant::corruption:
        if (blockID == TileID::chest) {
            offsetX = 864;
        } else if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 576 + 36 * (x % 3);
        }
        break;
    case Variant::crimson:
        if (blockID == TileID::altar) {
            offsetX = 54;
        } else if (blockID == TileID::orbHeart) {
            offsetX = 36;
        } else if (blockID == TileID::chest) {
            offsetX = 900;
        } else if (blockID == TileID::pot) {
            offsetY = 792 + 36 * ((x + y) % 3);
        }
        break;
    case Variant::crystal:
        if (blockID == TileID::lamp) {
            offsetY = 1674;
        }
        break;
    case Variant::desert:
        if (blockID == TileID::chest) {
            blockID = TileID::chestGroup2;
            offsetX = 468;
        } else if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 1224 + 36 * (x % 3);
        }
        break;
    case Variant::dungeon:
        if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 360 + 36 * (x % 3);
        }
        break;
    case Variant::dynasty:
        if (blockID == TileID::lamp) {
            offsetY = 936;
        }
        break;
    case Variant::flesh:
        if (blockID == TileID::chest) {
            offsetX = 1548;
        } else if (blockID == TileID::lamp) {
            offsetY = 162;
        }
        break;
    case Variant::forest:
        if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 36 * (x % 4);
        }
        break;
    case Variant::frozen:
        if (blockID == TileID::chest) {
            offsetX = 396;
        } else if (blockID == TileID::lamp) {
            offsetY = 270;
        }
        break;
    case Variant::hallowed:
        if (blockID == TileID::chest) {
            offsetX = 936;
        }
        break;
    case Variant::honey:
        if (blockID == TileID::chest) {
            offsetX = 1044;
        }
        break;
    case Variant::ice:
        if (blockID == TileID::chest) {
            offsetX = 972;
        }
        break;
    case Variant::jungle:
        if (blockID == TileID::chest) {
            offsetX = 828;
        } else if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 252 + 36 * (x % 3);
        }
        break;
    case Variant::gold:
        if (blockID == TileID::chest) {
            offsetX = 36;
        }
        break;
    case Variant::goldLocked:
        if (blockID == TileID::chest) {
            offsetX = 72;
        }
        break;
    case Variant::granite:
        if (blockID == TileID::chest) {
            offsetX = 1800;
        }
        break;
    case Variant::lesion:
        if (blockID == TileID::chest) {
            blockID = TileID::chestGroup2;
            offsetX = 108;
        } else if (blockID == TileID::lamp) {
            offsetY = 1782;
        }
        break;
    case Variant::lihzahrd:
        if (blockID == TileID::chest) {
            offsetX = 576;
        } else if (blockID == TileID::pot) {
            offsetX = 36 * (y % 2);
            offsetY = 1008 + 36 * (x % 3);
        }
        break;
    case Variant::marble:
        if (blockID == TileID::chest) {
            offsetX = 1836;
        } else if (blockID == TileID::pot) {
            offsetY = 1116 + 36 * ((x + y) % 3);
        }
        break;
    case Variant::meteorite:
        if (blockID == TileID::chest) {
            offsetX = 1764;
        }
        break;
    case Variant::mushroom:
        if (blockID == TileID::chest) {
            offsetX = 1152;
        }
        break;
    case Variant::palmWood:
        if (blockID == TileID::chest) {
            offsetX = 1116;
        }
        break;
    case Variant::reef:
        if (blockID == TileID::chest) {
            blockID = TileID::chestGroup2;
            offsetX = 504;
        }
        break;
    case Variant::richMahogany:
        if (blockID == TileID::chest) {
            offsetX = 288;
        }
        break;
    case Variant::sandstone:
        if (blockID == TileID::chest) {
            blockID = TileID::chestGroup2;
            offsetX = 360;
        } else if (blockID == TileID::lamp) {
            offsetY = 2052;
        }
        break;
    case Variant::shadow:
        if (blockID == TileID::chest) {
            offsetX = 144;
        }
        break;
    case Variant::spider:
        if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 684 + 36 * (x % 3);
        }
        break;
    case Variant::tundra:
        if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 144 + 36 * (x % 3);
        }
        break;
    case Variant::underworld:
        if (blockID == TileID::pot) {
            offsetX = 36 * (y % 3);
            offsetY = 468 + 36 * (x % 3);
        }
        break;
    case Variant::water:
        if (blockID == TileID::chest) {
            offsetX = 612;
        }
        break;
    default:
        break;
    }
    switch (blockID) {
    case TileID::altar:
        frameWidth = 3;
        break;
    case TileID::larva:
        frameWidth = 3;
        frameHeight = 3;
        break;
    case TileID::lamp:
        frameWidth = 1;
        frameHeight = 3;
        break;
    default:
        break;
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
