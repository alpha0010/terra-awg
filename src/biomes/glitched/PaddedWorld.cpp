#include "biomes/glitched/PaddedWorld.h"

#include "World.h"

PaddedWorld::PaddedWorld(World &w) : bufTop(w.getWidth() * padding), world(w)
{
    Tile tile{};
    tile.blockID = TileID::ash;
    bufBot.resize(w.getWidth() * padding, tile);
}

int PaddedWorld::getWidth() const
{
    return world.getWidth();
}

int PaddedWorld::getHeight() const
{
    return 2 * padding + world.getHeight();
}

Tile &PaddedWorld::getTile(int x, int y)
{
    if (y < padding) {
        return bufTop[y + x * padding];
    } else if (y < padding + world.getHeight()) {
        return world.getTile(x, y - padding);
    } else {
        return bufBot[y - padding - world.getHeight() + x * padding];
    }
}
