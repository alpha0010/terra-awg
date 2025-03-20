#ifndef WORLD_H
#define WORLD_H

#include "Tile.h"
#include <vector>

class Random;

class World
{
private:
    int width;
    int height;
    Tile scratchTile;
    std::vector<Tile> tiles;

public:
    World();

    int getWidth() const;
    int getHeight() const;
    int getUndergroundLevel() const;
    int getCavernLevel() const;
    int getUnderworldLevel() const;
    Tile &getTile(int x, int y);

    bool isExposed(int x, int y) const;
    void planBiomes(Random &rnd);

    bool isCrimson;
    int copperVariant;
    int ironVariant;
    int silverVariant;
    int goldVariant;

    double desertCenter;
    double jungleCenter;
    double snowCenter;
};

#endif // WORLD_H
