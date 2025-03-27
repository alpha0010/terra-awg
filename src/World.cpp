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
        }
        break;
    case Variant::crimson:
        if (blockID == TileID::altar) {
            offsetX = 54;
        } else if (blockID == TileID::orbHeart) {
            offsetX = 36;
        } else if (blockID == TileID::chest) {
            offsetX = 900;
        }
        break;
    case Variant::desert:
        if (blockID == TileID::chest) {
            blockID = 467; // Chest group 2.
            offsetX = 468;
        }
        break;
    case Variant::frozen:
        offsetX = 792;
        break;
    case Variant::hallowed:
        offsetX = 936;
        break;
    case Variant::jungle:
        offsetX = 828;
        break;
    case Variant::gold:
        offsetX = 36;
        break;
    case Variant::goldLocked:
        offsetX = 72;
        break;
    case Variant::lihzahrd:
        offsetX = 576;
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
