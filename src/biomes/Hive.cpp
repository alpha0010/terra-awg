#include "Hive.h"

#include "Random.h"
#include "World.h"
#include <iostream>

std::pair<int, int> getHexCentroid(int x, int y)
{
    int scale = 10;
    int targetX = x;
    int targetY = y;
    double minDist = 2 * scale;
    double centralDist = scale * 2 / std::sqrt(3);
    double startRow = std::floor(y / centralDist);
    for (int col : {x / scale, x / scale + 1}) {
        for (double row : {startRow, startRow + 1}) {
            if (col % 2 == 0) {
                row += 0.5;
            }
            int testX = col * scale;
            int testY = row * centralDist;
            double testDist = std::hypot(x - testX, y - testY);
            if (testDist < minDist) {
                targetX = testX;
                targetY = testY;
                minDist = testDist;
            }
        }
    }
    return {targetX, targetY};
}

bool isHiveEdge(int x, int y, World &world)
{
    if (world.getTile(x, y).wallID != WallID::Unsafe::hive) {
        return false;
    }
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (world.getTile(x + i, y + j).wallID != WallID::Unsafe::hive) {
                return true;
            }
        }
    }
    return false;
}

void fillHive(Random &rnd, World &world)
{
    rnd.shuffleNoise();
    double size = world.getWidth() / rnd.getDouble(70, 120);
    int hiveX = rnd.getInt(
        world.jungleCenter - 0.075 * world.getWidth(),
        world.jungleCenter + 0.075 * world.getWidth());
    int hiveY = rnd.getInt(
        (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3);
    for (int x = hiveX - size; x < hiveX + size; ++x) {
        for (int y = hiveY - size; y < hiveY + size; ++y) {
            auto [centroidX, centroidY] = getHexCentroid(x, y);
            double threshold =
                3 * std::hypot(hiveX - centroidX, hiveY - centroidY) / size - 2;
            Tile &tile = world.getTile(x, y);
            if (rnd.getFineNoise(centroidX, centroidY) > threshold) {
                tile.blockID = std::abs(rnd.getFineNoise(
                                   centroidX + hiveX,
                                   centroidY + hiveY)) > 0.31
                                   ? TileID::hive
                                   : TileID::empty;
                tile.wallID = WallID::Unsafe::hive;
            } else if (
                rnd.getFineNoise(x + hiveX, y + hiveY) >
                    std::max(0.5, threshold) &&
                (tile.blockID == TileID::mud ||
                 tile.blockID == TileID::jungleGrass ||
                 tile.blockID == TileID::mushroomGrass ||
                 tile.blockID == TileID::marble)) {
                tile.blockID = TileID::honey;
            }
        }
    }
    for (int x = hiveX - size; x < hiveX + size; ++x) {
        for (int y = hiveY - size; y < hiveY + size; ++y) {
            if (isHiveEdge(x, y, world)) {
                for (int i = -2; i < 3; ++i) {
                    for (int j = -2; j < 3; ++j) {
                        Tile &tile = world.getTile(x + i, y + j);
                        if (tile.wallID == WallID::Unsafe::hive) {
                            tile.blockID = TileID::hive;
                        }
                    }
                }
            }
        }
    }
}

void genHive(Random &rnd, World &world)
{
    std::cout << "Importing bees\n";
    int numHives = rnd.getInt(1, 3);
    for (int i = 0; i < numHives; ++i) {
        fillHive(rnd, world);
    }
}
