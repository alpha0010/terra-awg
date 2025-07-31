#ifndef WORLD_H
#define WORLD_H

#include "Chest.h"
#include "Point.h"
#include "Tile.h"
#include "ids/Biome.h"
#include "ids/TileVariant.h"
#include <cstdint>
#include <functional>
#include <vector>

struct Config;
class Random;
class TileBuffer;

enum class Blend { normal, blockOnly };

uint32_t fnv1a32pt(uint32_t x, uint32_t y);

struct BiomeData {
    Biome active;
    double forest;
    double snow;
    double desert;
    double jungle;
    double underworld;
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
    std::vector<int> surface;
    std::vector<BiomeData> biomeMap;

public:
    World(const Config &c);

    int getWidth() const;
    int getHeight() const;
    int &getSurfaceLevel(int x);
    int getUndergroundLevel() const;
    int getCavernLevel() const;
    int getUnderworldLevel() const;
    Tile &getTile(int x, int y);
    Tile &getTile(Point pt)
    {
        return getTile(pt.x, pt.y);
    }
    BiomeData &getBiome(int x, int y);
    std::vector<Point> placeBuffer(
        int x,
        int y,
        const TileBuffer &data,
        Blend blendMode = Blend::normal);
    void placeFramedTile(
        int x,
        int y,
        int blockID,
        Variant type = Variant::none,
        int paint = 0);
    void placePainting(int x, int y, Painting painting);
    Point getPaintingDims(Painting painting);
    Chest &placeChest(int x, int y, Variant type);
    Chest &registerStorage(int x, int y);

    std::vector<Chest> &getChests()
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
     * Check if the given location is surrounded exclusively by empty tiles.
     */
    bool isIsolated(int x, int y) const;
    /**
     * Select desert, jungle, and snow locations.
     */
    void planBiomes(Random &rnd);

    const Config &conf;

    bool isCrimson;
    int copperVariant;
    int ironVariant;
    int silverVariant;
    int goldVariant;
    int cobaltVariant;
    int mythrilVariant;
    int adamantiteVariant;

    Point spawn;

    double desertCenter;
    double jungleCenter;
    double snowCenter;
    int surfaceEvilCenter;
    int oceanCaveCenter;
    Point aether;
    Point dungeon;
    Point gemGrove;
    int gemGroveSize;
    std::vector<Point> mushroomCenter;

    std::vector<std::function<void(Random &, World &)>> queuedEvil;
    std::vector<std::function<void(Random &, World &)>> queuedDeco;
    std::vector<std::function<void(Random &, World &)>> queuedTraps;
    std::vector<std::function<void(Random &, World &)>> queuedTreasures;
};

#endif // WORLD_H
