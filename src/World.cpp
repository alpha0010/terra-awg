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
    switch (blockID) {
    case TileID::lifeCrystal:
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                Tile &tile = getTile(x + i, y + j);
                tile.blockID = blockID;
                tile.frameX = 18 * i;
                tile.frameY = 18 * j;
            }
        }
        break;
    case TileID::altar:
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                Tile &tile = getTile(x + i, y + j);
                tile.blockID = blockID;
                tile.frameX = 18 * i + (type == Variant::crimson ? 54 : 0);
                tile.frameY = 18 * j;
            }
        }
        break;
    case TileID::orbHeart:
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                Tile &tile = getTile(x + i, y + j);
                tile.blockID = blockID;
                tile.frameX = 18 * i + (type == Variant::crimson ? 36 : 0);
                tile.frameY = 18 * j;
            }
        }
        break;
    case TileID::larva:
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                Tile &tile = getTile(x + i, y + j);
                tile.blockID = blockID;
                tile.frameX = 18 * i;
                tile.frameY = 18 * j;
            }
        }
        break;
    default:
        break;
    }
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
