#include "biomes/AsteroidField.h"

#include "Random.h"
#include "World.h"
#include "ids/Paint.h"
#include <iostream>

std::pair<int, int>
selectAsteroidFieldLocation(int width, int height, Random &rnd, World &world)
{
    while (true) {
        int x = rnd.getInt(40, 0.3 * world.getWidth() - width);
        if (rnd.getBool()) {
            x = world.getWidth() - x - width;
        }
        int y = rnd.getInt(
            40,
            std::max<int>(0.35 * world.getUndergroundLevel() - height, 50));
        if (world.regionPasses(
                x + 0.1 * width,
                y,
                0.8 * width,
                height,
                [](Tile &tile) { return tile.blockID == TileID::empty; })) {
            return {x, y};
        }
    }
}

void genAsteroidField(Random &rnd, World &world)
{
    std::cout << "Suspending asteroids\n";
    int width = rnd.getDouble(0.06, 0.07) * world.getWidth();
    int height = rnd.getDouble(0.13, 0.16) * world.getUndergroundLevel();
    auto [fieldX, fieldY] =
        selectAsteroidFieldLocation(width, height, rnd, world);
    int numAsteroids = width * height / 220;
    while (numAsteroids > 0) {
        double radius = rnd.getDouble(2, 9);
        int x = rnd.getInt(fieldX + radius, fieldX + width - radius);
        int y = rnd.getInt(fieldY + radius, fieldY + height - radius);
        double centerDist = std::hypot(
            (fieldX + 0.5 * width - x) / width,
            (fieldY + 0.5 * height - y) / height);
        if ((centerDist > 0.48 && (x + y) % 9 != 0) ||
            !world.regionPasses(
                x - radius,
                y - radius,
                2 * radius,
                2 * radius,
                [](Tile &tile) { return tile.blockID == TileID::empty; })) {
            continue;
        }
        int paint = rnd.select({Paint::brown, Paint::black});
        for (int i = -radius; i < radius; ++i) {
            for (int j = -radius; j < radius; ++j) {
                if (std::hypot(i, j) / radius <
                    0.6 + 0.6 * rnd.getFineNoise(x + i, y + j)) {
                    Tile &tile = world.getTile(x + i, y + j);
                    tile.blockID =
                        std::min(
                            std::abs(rnd.getFineNoise(x + i, j + radius)),
                            std::abs(rnd.getFineNoise(i + radius, y + j))) <
                                0.03
                            ? TileID::meteorite
                            : TileID::stone;
                    tile.blockPaint = paint;
                }
            }
        }
        --numAsteroids;
    }
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            Tile &tile = world.getTile(fieldX + i, fieldY + j);
            if ((tile.blockID == TileID::stone ||
                 tile.blockID == TileID::meteorite) &&
                world.isExposed(fieldX + i, fieldY + j)) {
                tile.actuated = true;
            }
        }
    }
}
