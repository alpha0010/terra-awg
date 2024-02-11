#ifndef WORLD_H
#define WORLD_H

#include "Tile.h"
#include <vector>

class World
{
private:
    int width;
    int height;
    Tile scratchTile;
    std::vector<Tile> tiles;

public:
    World();

    int getWidth();
    int getHeight();
    int getUndergroundLevel();
    int getCavernLevel();
    Tile &getTile(int x, int y);
};

#endif // WORLD_H
