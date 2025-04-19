#ifndef WORLD_H
#define WORLD_H

#include "Chest.h"
#include "Tile.h"
#include "ids/TileVariant.h"
#include <functional>
#include <vector>

class Random;

class World
{
private:
    int width;
    int height;
    Tile scratchTile;
    std::vector<Tile> tiles;
    std::vector<Chest> chests;
    std::vector<bool> framedTiles;
    std::vector<int> surface;

public:
    World(int w, int h);

    int getWidth() const;
    int getHeight() const;
    int &getSurfaceLevel(int x);
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
    void placePainting(int x, int y, Painting painting);
    std::pair<int, int> getPaintingDims(Painting painting);
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
    int dungeonX;
    int dungeonY;

    std::vector<std::function<void(Random &, World &)>> queuedTraps;
    std::vector<std::function<void(Random &, World &)>> queuedTreasures;
};

#endif // WORLD_H
