#ifndef WORLD_H
#define WORLD_H

#include "Chest.h"
#include "Tile.h"
#include <vector>

class Random;

enum class Variant {
    none,
    corruption,
    crimson,
    desert,
    flesh,
    frozen,
    gold,
    goldLocked,
    granite,
    hallowed,
    honey,
    ice,
    jungle,
    lesion,
    lihzahrd,
    marble,
    meteorite,
    mushroom,
    palmWood,
    reef,
    richMahogany,
    sandstone,
    shadow,
    water
};

class World
{
private:
    int width;
    int height;
    Tile scratchTile;
    std::vector<Tile> tiles;
    std::vector<Chest> chests;

public:
    World();

    int getWidth() const;
    int getHeight() const;
    int getUndergroundLevel() const;
    int getCavernLevel() const;
    int getUnderworldLevel() const;
    Tile &getTile(int x, int y);
    void
    placeFramedTile(int x, int y, int blockID, Variant type = Variant::none);
    Chest &placeChest(int x, int y, Variant type);

    const std::vector<Chest> &getChests() const
    {
        return chests;
    }

    bool isExposed(int x, int y) const;
    void planBiomes(Random &rnd);

    bool isCrimson;
    int copperVariant;
    int ironVariant;
    int silverVariant;
    int goldVariant;

    int spawnY;

    double desertCenter;
    double jungleCenter;
    double snowCenter;
    int surfaceEvilCenter;
    int dungeonCenter;
};

#endif // WORLD_H
