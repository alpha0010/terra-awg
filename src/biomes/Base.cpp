#include "Base.h"

#include "Random.h"
#include "World.h"

void scatterResource(Random &rnd, World &world, int resource)
{
    rnd.shuffleNoise();
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            if (rnd.getFineNoise(x, y) > 0.7) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID != TileID::empty) {
                    tile.blockID = resource;
                }
            }
        }
    }
}

void genWorldBase(Random &rnd, World &world)
{
    rnd.initNoise(world.getWidth(), world.getHeight(), 0.07);
    double surfaceLevel = rnd.getDouble(
        0.7 * world.getUndergroundLevel(),
        0.8 * world.getUndergroundLevel());
    int center = world.getWidth() / 2;
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = surfaceLevel +
                     std::min(0.1 * std::abs(center - x) + 15, 50.0) *
                         rnd.getCoarseNoise(x, 0);
             y < world.getHeight();
             ++y) {
            double threshold =
                y < world.getUndergroundLevel()
                    ? 3.0 * y / world.getUndergroundLevel() - 3
                    : static_cast<double>(y - world.getUndergroundLevel()) /
                          (world.getHeight() - world.getUndergroundLevel());
            Tile &tile = world.getTile(x, y);
            tile.blockID = rnd.getFineNoise(x, y) > threshold ? TileID::dirt
                                                              : TileID::stone;
        }
    }

    scatterResource(rnd, world, TileID::clay);
    scatterResource(rnd, world, TileID::sand);
    scatterResource(rnd, world, TileID::mud);

    int underworldHeight = world.getHeight() - world.getUnderworldLevel();
    for (int x = 0; x < world.getWidth(); ++x) {
        int underworldRoof =
            world.getUnderworldLevel() + 0.1 * underworldHeight +
            20 * rnd.getCoarseNoise(x, 0.33 * world.getHeight());
        int underworldFloor =
            world.getUnderworldLevel() + 0.5 * underworldHeight +
            20 * rnd.getCoarseNoise(x, 0.66 * world.getHeight());
        for (int y = world.getUnderworldLevel() + 20 * rnd.getCoarseNoise(x, 0);
             y < world.getHeight();
             ++y) {
            Tile &tile = world.getTile(x, y);
            tile.blockID = y < underworldRoof || y > underworldFloor
                               ? TileID::ash
                               : TileID::empty;
        }
    }

    rnd.shuffleNoise();
    for (int x = 0; x < world.getWidth(); ++x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            double threshold = y < world.getUndergroundLevel()
                                   ? 3 - 3.1 * y / world.getUndergroundLevel()
                               : y > world.getUnderworldLevel()
                                   ? 3.1 * (y - world.getUnderworldLevel()) /
                                             underworldHeight -
                                         0.1
                                   : -0.1;
            if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                rnd.getFineNoise(x, y) > threshold) {
                Tile &tile = world.getTile(x, y);
                tile.blockID = TileID::empty;
            }
        }
    }
}
