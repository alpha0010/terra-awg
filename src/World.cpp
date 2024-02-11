#include "World.h"

World::World() : width(6400), height(1800), tiles(width * height) {}

int World::getWidth()
{
    return width;
}

int World::getHeight()
{
    return height;
}

int World::getUndergroundLevel()
{
    return 0.28 * height;
}

int World::getCavernLevel()
{
    return 0.41 * height;
}

Tile &World::getTile(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return scratchTile;
    }
    return tiles[y + x * height];
}
