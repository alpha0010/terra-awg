#include "biomes/glitched/TerrainGlitch.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/glitched/PaddedWorld.h"
#include "vendor/frozen/set.h"
#include <iostream>

void shiftWorldCol(int x, int delta, PaddedWorld &world)
{
    if (delta > 0) {
        for (int y = world.getHeight() - 1; y >= delta; --y) {
            world.getTile(x, y) = world.getTile(x, y - delta);
        }
    } else {
        for (int y = 0; y < world.getHeight() + delta; ++y) {
            world.getTile(x, y) = world.getTile(x, y - delta);
        }
    }
}

void rotateWorldRow(int y, int delta, PaddedWorld &world)
{
    thread_local std::vector<Tile> scratch;
    scratch.clear();
    if (delta > 0) {
        for (int x = world.getWidth() - 1; x >= delta; --x) {
            Tile &tile = world.getTile(x, y);
            if (x >= world.getWidth() - delta) {
                scratch.push_back(tile);
            }
            tile = world.getTile(x - delta, y);
        }
        for (size_t i = 0; i < scratch.size(); ++i) {
            world.getTile(delta - 1 - i, y) = scratch[i];
        }
    } else {
        for (int x = 0; x < world.getWidth() + delta; ++x) {
            Tile &tile = world.getTile(x, y);
            if (x < -delta) {
                scratch.push_back(tile);
            }
            tile = world.getTile(x - delta, y);
        }
        for (size_t i = 0; i < scratch.size(); ++i) {
            world.getTile(world.getWidth() + i + delta, y) = scratch[i];
        }
    }
}

void swapRect(Point from, Point to, int width, int height, World &world)
{
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            std::swap(
                world.getTile(from + Point{i, j}),
                world.getTile(to + Point{i, j}));
        }
    }
}

void terrainGlitch(Random &rnd, World &world)
{
    PaddedWorld padded{world};
    int maxBand =
        std::min(std::min(padded.getHeight(), padded.getWidth()) / 8, 400);
    if (maxBand < 100) {
        return;
    }
    std::cout << "ating terrGenerain\n";

    int numRowShifts = padded.getHeight() / 100;
    int numColShifts = padded.getWidth() / 100;
    while (numRowShifts > 0 || numColShifts > 0) {
        int band = rnd.getInt(50, maxBand);
        int delta = rnd.getInt(-10, 11);
        if (delta > 0) {
            delta += 2;
        } else {
            delta -= 3;
        }
        if (rnd.getInt(0, numRowShifts + numColShifts - 1) < numRowShifts) {
            --numRowShifts;
            int row = rnd.getInt(band, padded.getHeight() - band) - band / 2;
            parallelFor(
                std::views::iota(row, row + band),
                [delta, &padded, &world](int y) {
                    rotateWorldRow(y, delta, padded);
                });
        } else {
            --numColShifts;
            int col = rnd.getInt(band, padded.getWidth() - band) - band / 2;
            parallelFor(
                std::views::iota(col, col + band),
                [delta, &padded, &world](int x) {
                    shiftWorldCol(x, delta, padded);
                    world.getSurfaceLevel(x) += delta;
                    if (world.spawn.x == x) {
                        world.spawn.y += delta;
                    }
                });
        }
    }

    constexpr auto groundTiles = frozen::make_set<int>({
        TileID::ash,
        TileID::ashGrass,
        TileID::corruptGrass,
        TileID::crimsand,
        TileID::crimsonGrass,
        TileID::dirt,
        TileID::ebonsand,
        TileID::grass,
        TileID::hardenedCrimsand,
        TileID::hardenedEbonsand,
        TileID::hardenedSand,
        TileID::ice,
        TileID::jungleGrass,
        TileID::mud,
        TileID::sand,
        TileID::snow,
    });
    for (int x = 0; x < world.getWidth(); ++x) {
        int &y = world.getSurfaceLevel(x);
        while (y > 0 && groundTiles.contains(world.getTile(x, y - 1).blockID)) {
            --y;
        }
        while (y < world.getUndergroundLevel() &&
               world.getTile(x, y).blockID == TileID::empty) {
            ++y;
        }
    }

    int numSwaps = world.getWidth() * world.getHeight() / 8500;
    while (numSwaps > 0) {
        --numSwaps;
        swapRect(
            {rnd.getInt(50, world.getWidth() - 50),
             rnd.getInt(50, world.getHeight() - 50)},
            {rnd.getInt(50, world.getWidth() - 50),
             rnd.getInt(50, world.getHeight() - 50)},
            rnd.getInt(1, 10),
            rnd.getInt(1, 10),
            world);
    }
}
