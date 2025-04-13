#ifndef WORLD_H
#define WORLD_H

#include "Chest.h"
#include "Tile.h"
#include <functional>
#include <vector>

class Random;

enum class Variant {
    none,
    ankh,
    ashWood,
    corruption,
    crimson,
    crystal,
    desert,
    dungeon,
    dynasty,
    flesh,
    forest,
    frozen,
    gold,
    goldLocked,
    granite,
    hallowed,
    health,
    honey,
    ice,
    ivy,
    jungle,
    lesion,
    lihzahrd,
    livingWood,
    mana,
    marble,
    meteorite,
    mushroom,
    omega,
    palmWood,
    pyramid,
    reef,
    richMahogany,
    sandstone,
    shadow,
    skyware,
    snake,
    spider,
    tundra,
    underworld,
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
    std::vector<bool> framedTiles;

public:
    World();

    int getWidth() const;
    int getHeight() const;
    int getUndergroundLevel() const;
    int getCavernLevel() const;
    int getUnderworldLevel() const;
    Tile &getTile(int x, int y);
    Tile &getTile(std::pair<int, int> pt)
    {
        return getTile(pt.first, pt.second);
    }
    void
    placeFramedTile(int x, int y, int blockID, Variant type = Variant::none);
    Chest &placeChest(int x, int y, Variant type);

    const std::vector<Chest> &getChests() const
    {
        return chests;
    }

    const std::vector<bool> &getFramedTiles() const
    {
        return framedTiles;
    }

    /**
     * Check if all tiles in the area pass a predicate.
     */
    template <typename Func>
    bool regionPasses(int x, int y, int width, int height, Func f)
    {
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                if (!f(getTile(x + i, y + j))) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Check if the given location is not solidly surrounded by tiles.
     */
    bool isExposed(int x, int y) const;
    /**
     * Select desert, jungle, and snow locations.
     */
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

    std::vector<std::function<void(Random &, World &)>> queuedTreasures;
};

#endif // WORLD_H
