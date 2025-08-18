#include "Hive.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include <iostream>

bool isHiveEdge(int x, int y, World &world)
{
    Tile &centerTile = world.getTile(x, y);
    if (centerTile.wallID != WallID::Unsafe::hive ||
        centerTile.flag == Flag::border) {
        return false;
    }
    return !world.regionPasses(x - 1, y - 1, 3, 3, [](Tile &tile) {
        return tile.wallID == WallID::Unsafe::hive && tile.flag != Flag::border;
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
            y = scanWhileEmpty({x, y}, {0, 1}, world).y;
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
    fillHive(
        hiveX,
        hiveY,
        world.conf.hiveSize * (15 + world.getWidth() / rnd.getDouble(84, 166)),
        rnd,
        world);
}

void fillHive(int hiveX, int hiveY, double size, Random &rnd, World &world)
{
    rnd.shuffleNoise();
    for (int x = hiveX - size; x < hiveX + size; ++x) {
        for (int y = hiveY - size; y < hiveY + size; ++y) {
            auto [centroidX, centroidY] = getHexCentroid(x, y, 10);
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
                tile.flag = Flag::none;
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
    std::vector<Point> hiveEdges;
    for (int x = hiveX - size; x < hiveX + size; ++x) {
        for (int y = hiveY - size; y < hiveY + size; ++y) {
            if (isHiveEdge(x, y, world) &&
                std::hypot(x - hiveX, y - hiveY) < size) {
                hiveEdges.emplace_back(x, y);
            }
        }
    }
    if (size > 200) {
        for (auto [x, y] : hiveEdges) {
            for (int i = -2; i < 3; ++i) {
                for (int j = -2; j < 3; ++j) {
                    Tile &tile = world.getTile(x + i, y + j);
                    tile.blockID = TileID::hive;
                    tile.wallID = WallID::Unsafe::hive;
                }
            }
        }
    } else {
        for (auto [x, y] : hiveEdges) {
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
    world.queuedTreasures.emplace_back([hiveX,
                                        hiveY,
                                        size](Random &rnd, World &world) {
        std::vector<Point> usedLocations;
        for (int x = hiveX - size; x < hiveX + size; ++x) {
            for (int y = hiveY - size; y < hiveY + size; ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::larva && tile.frameX == 0 &&
                    tile.frameY == 0) {
                    usedLocations.emplace_back(x, y);
                }
            }
        }
        double area = size * size;
        for (int larvaCount = 1 + area / rnd.getInt(2500, 7000); larvaCount > 0;
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
        if (rnd.getDouble(0, 1) > std::min(0.4, 1 - area / 15000)) {
            auto [x, y] = selectLarvaeLocation(
                hiveX,
                hiveY,
                size,
                usedLocations,
                rnd,
                world);
            if (x != -1) {
                Chest &chest = world.placeChest(x, y - 1, Variant::honey);
                fillHoneyChest(chest, getChestDepth(x, y, world), rnd, world);
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

Point selectHiveLocation(Random &rnd, World &world)
{
    if (world.conf.biomes == BiomeLayout::columns && !world.conf.hiveQueen) {
        return {
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
                (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3)};
    }
    for (int numTries = 0; numTries < 100; ++numTries) {
        int x = rnd.getInt(350, world.getWidth() - 350);
        int y = rnd.getInt(
            (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
            (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3);
        int borderScan = 90 - numTries / 2;
        if (isInBiome(
                x,
                y,
                15 + world.getWidth() / 120,
                Biome::jungle,
                world) &&
            (!world.conf.hiveQueen ||
             world.regionPasses(
                 x - borderScan / 2,
                 y - borderScan / 2,
                 borderScan,
                 borderScan,
                 [](Tile &tile) { return tile.flag != Flag::border; }))) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void genHive(Random &rnd, World &world)
{
    std::cout << "Importing bees\n";
    int numHives =
        world.conf.hiveFreq * std::max(0.4 * world.conf.jungleSize, 1.0) *
        (2.1 +
         rnd.getDouble(0, world.getWidth() * world.getHeight() / 5750000.0));
    for (int tries = 5 * numHives; tries > 0 && numHives > 0; --tries) {
        auto [x, y] = selectHiveLocation(rnd, world);
        if (x != -1) {
            fillHive(x, y, rnd, world);
            --numHives;
        }
    }
}
