#include "World.h"

World::World() : width(6400), height(1800) {}

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
