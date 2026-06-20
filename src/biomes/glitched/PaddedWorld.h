#ifndef GLITCHED_PADDEDWORLD_H
#define GLITCHED_PADDEDWORLD_H

#include "Tile.h"
#include <vector>

class World;

class PaddedWorld
{
private:
    static constexpr int padding = 50;
    std::vector<Tile> bufTop;
    std::vector<Tile> bufBot;
    World &world;

public:
    PaddedWorld(World &w);

    int getWidth() const;
    int getHeight() const;
    Tile &getTile(int x, int y);
};

#endif // GLITCHED_PADDEDWORLD_H
