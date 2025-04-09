#include "Cloud.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include <iostream>

void makeFishingCloud(
    int startX,
    int startY,
    int width,
    int height,
    World &world)
{
    int minX = startX + 0.2 * width;
    int maxX = startX + 0.8 * width;
    int waterLevel = startY;
    while (world.getTile(minX, waterLevel).blockID == TileID::empty ||
           world.getTile(maxX, waterLevel).blockID == TileID::empty) {
        ++waterLevel;
    }
    for (int x = minX; x < maxX; ++x) {
        int depth = 0.45 * height *
                    std::sin(std::numbers::pi * (x - minX) / (0.6 * width));
        int y = startY;
        while (depth > 0) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::empty) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                if (y > waterLevel) {
                    tile.liquid = Liquid::water;
                }
                --depth;
            }
            ++y;
        }
    }
}
void makeResourceCloud(
    int startX,
    int startY,
    int width,
    int height,
    Random &rnd,
    World &world)
{
    int minX = startX + 0.2 * width;
    int maxX = startX + 0.8 * width;
    for (int x = minX; x < maxX; ++x) {
        int depth = 0.45 * height *
                    std::sin(std::numbers::pi * (x - minX) / (0.6 * width));
        int y = startY;
        bool prevWasEmpty = true;
        while (depth > 0) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::empty) {
                tile.blockID = TileID::dirt;
                if (prevWasEmpty) {
                    tile.wallID = WallID::empty;
                    prevWasEmpty = false;
                } else {
                    tile.wallID = WallID::Unsafe::dirt;
                }
                --depth;
            }
            ++y;
        }
    }
    for (int x = startX; x < startX + width; ++x) {
        for (int y = startY; y < startY + height; ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.blockID != TileID::empty &&
                (std::abs(rnd.getFineNoise(x, y + startY)) > 0.65 ||
                 std::abs(rnd.getFineNoise(x + startX, y)) > 0.65)) {
                tile.blockID = world.goldVariant;
            }
        }
    }
}

void genCloud(Random &rnd, World &world)
{
    std::cout << "Condensing clouds\n";
    rnd.shuffleNoise();
    int numClouds = world.getWidth() / rnd.getInt(600, 1300);
    while (numClouds > 0) {
        int width = rnd.getInt(90, 160);
        int height = rnd.getInt(35, 50);
        int x = rnd.getInt(200, world.getWidth() - 200 - width);
        int y = rnd.getInt(100, 0.45 * world.getUndergroundLevel() - height);
        if (!world.regionPasses(x, y, width, height, [](Tile &tile) {
                return tile.blockID == TileID::empty;
            })) {
            continue;
        }
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                double threshold =
                    8 * std::hypot(
                            static_cast<double>(i) / width - 0.5,
                            static_cast<double>(j) / height - 0.5) -
                    3;
                if (rnd.getFineNoise(x + i, y + j) < threshold) {
                    continue;
                }
                Tile &tile = world.getTile(x + i, y + j);
                tile.blockID = TileID::cloud;
                tile.wallID = WallID::Safe::cloud;
            }
        }
        switch (numClouds % 3) {
        case 1:
            makeFishingCloud(x, y, width, height, world);
            break;
        case 2:
            makeResourceCloud(x, y, width, height, rnd, world);
            break;
        }
        --numClouds;
    }
}
