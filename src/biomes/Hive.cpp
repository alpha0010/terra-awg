#include "Hive.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
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
    return !world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
        return tile.wallID == WallID::Unsafe::hive;
    });
}

Point selectLarvaeLocation(
    int hiveX,
    int hiveY,
    int size,
    std::vector<Point> &usedLocations,
    Random &rnd,
    World &world)
{
    for (int numTries = 0; numTries < 100; ++numTries) {
        int x = rnd.getInt(hiveX - size, hiveX + size);
        int y = rnd.getInt(hiveY - size, hiveY + size);
        if (world.getTile(x, y).blockID == TileID::empty) {
            y = scanWhileEmpty({x, y}, {0, 1}, world).second;
        }
        if (world.getTile(x + 1, y).wallID == WallID::Unsafe::hive &&
            world.regionPasses(
                x,
                y - 2,
                3,
                3,
                [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           tile.liquid == Liquid::none;
                }) &&
            world.regionPasses(
                x,
                y + 1,
                3,
                1,
                [](Tile &tile) { return tile.blockID == TileID::hive; }) &&
            !isLocationUsed(x, y, 35, usedLocations)) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void fillHive(int hiveX, int hiveY, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    double size =
        world.conf.hiveSize * (15 + world.getWidth() / rnd.getDouble(84, 166));
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
    world.queuedTreasures.emplace_back(
        [hiveX, hiveY, size](Random &rnd, World &world) {
            std::vector<Point> usedLocations;
            for (int larvaCount = rnd.getInt(1, std::max(2.0, size / 25));
                 larvaCount > 0;
                 --larvaCount) {
                auto [x, y] = selectLarvaeLocation(
                    hiveX,
                    hiveY,
                    size,
                    usedLocations,
                    rnd,
                    world);
                if (x != -1) {
                    usedLocations.emplace_back(x, y);
                    world.placeFramedTile(x, y - 2, TileID::larva);
                }
            }
            if (rnd.getDouble(0, 1) > 0.4) {
                auto [x, y] = selectLarvaeLocation(
                    hiveX,
                    hiveY,
                    size,
                    usedLocations,
                    rnd,
                    world);
                if (x != -1) {
                    Chest &chest = world.placeChest(x, y - 1, Variant::honey);
                    if (y < world.getCavernLevel()) {
                        fillUndergroundHoneyChest(chest, rnd, world);
                    } else {
                        fillCavernHoneyChest(chest, rnd, world);
                    }
                }
            }
            embedWaterfalls(
                {hiveX - size, hiveY - size / 2.5},
                {hiveX + size, hiveY + size / 3.5},
                {TileID::hive},
                Liquid::honey,
                35,
                rnd,
                world);
        });
}

void genHive(Random &rnd, World &world)
{
    std::cout << "Importing bees\n";
    int numHives =
        world.conf.hiveFreq * std::max(0.4 * world.conf.jungleSize, 1.0) *
        (2 +
         rnd.getDouble(0, world.getWidth() * world.getHeight() / 5750000.0));
    for (int i = 0; i < numHives; ++i) {
        fillHive(
            rnd.getInt(
                std::max<int>(
                    world.jungleCenter -
                        world.conf.jungleSize * 0.075 * world.getWidth(),
                    100),
                std::min<int>(
                    world.jungleCenter +
                        world.conf.jungleSize * 0.075 * world.getWidth(),
                    world.getWidth() - 100)),
            rnd.getInt(
                (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
                (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3),
            rnd,
            world);
    }
}
