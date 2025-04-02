#include "structures/Plants.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/data/Trees.h"
#include <iostream>
#include <span>

bool isRegionEmpty(int x, int y, int width, int height, World &world)
{
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (world.getTile(x + i, y + j).blockID != TileID::empty) {
                return false;
            }
        }
    }
    return true;
}

void growBamboo(int x, int y, Random &rnd, World &world)
{
    if (rnd.getDouble(0, 1) > 0.21) {
        return;
    }
    for (int i = 0; i < 11; ++i) {
        Tile &tile = world.getTile(x, y - i - 1);
        if (tile.blockID != TileID::empty ||
            (tile.liquid == Liquid::water && i > 4)) {
            return;
        }
    }
    int bambooSize = rnd.getInt(3, 10);
    for (int i = 0; i < bambooSize; ++i) {
        Tile &tile = world.getTile(x, y - i - 1);
        tile.blockID = TileID::bambooStalk;
        if (i == 0) {
            tile.frameX = 18 * rnd.getInt(0, 4);
        } else if (i == bambooSize - 1) {
            tile.frameX = 18 * rnd.getInt(14, 19);
        } else {
            tile.frameX = 18 * rnd.getInt(5, 13);
        }
    }
}

void growCactus(int x, int y, Random &rnd, World &world)
{
    if (x < 400 || x > world.getWidth() - 400 ||
        world.getTile(x, y - 1).wallID != WallID::empty ||
        rnd.getDouble(0, 1) > 0.07) {
        return;
    }
    int waterCount = 0;
    for (int i = -50; i < 50; ++i) {
        for (int j = -25; j < 25; ++j) {
            if (world.getTile(x + i, y + j).liquid == Liquid::water) {
                ++waterCount;
                if (waterCount > 5) {
                    return;
                }
            }
        }
    }
    int sandCount = 0;
    for (int i = -10; i < 10; ++i) {
        for (int j = -8; j < 8; ++j) {
            switch (world.getTile(x + i, y + j).blockID) {
            case TileID::cactusPlant:
                return;
            case TileID::sand:
            case TileID::hardenedSand:
            case TileID::ebonsand:
            case TileID::hardenedEbonsand:
            case TileID::crimsand:
            case TileID::hardenedCrimsand:
                ++sandCount;
                break;
            }
        }
    }
    if (sandCount < 15 || !isRegionEmpty(x - 1, y - 8, 3, 8, world)) {
        return;
    }
    int cactusSize = rnd.getInt(3, 7);
    for (int i = 0; i < cactusSize; ++i) {
        world.getTile(x, y - i - 1).blockID = TileID::cactusPlant;
        if (i > 0) {
            if (rnd.getInt(0, 4) <
                (world.getTile(x - 1, y - i).blockID == TileID::cactusPlant
                     ? 3
                     : 1)) {
                world.getTile(x - 1, y - i - 1).blockID = TileID::cactusPlant;
            }
            if (rnd.getInt(0, 4) <
                (world.getTile(x + 1, y - i).blockID == TileID::cactusPlant
                     ? 3
                     : 1)) {
                world.getTile(x + 1, y - i - 1).blockID = TileID::cactusPlant;
            }
        }
    }
}

void growTree(
    int x,
    int y,
    int groundTile,
    std::span<const Data::Tree> templateTrees,
    Random &rnd,
    World &world)
{
    for (int i = 0; i < 3; ++i) {
        if (world.getTile(x + i, y).blockID != groundTile) {
            return;
        }
    }
    int height = rnd.getInt(7, 15);
    if (!isRegionEmpty(x, y - 4 - height, 3, height + 4, world)) {
        return;
    }
    for (int j = 0; j < height; ++j) {
        TileBuffer tree =
            Data::getTree(rnd.select(templateTrees), world.getFramedTiles());
        int treeRow = j == 0            ? 0
                      : j == height - 1 ? tree.getHeight() - 1
                                        : rnd.getInt(1, tree.getHeight() - 2);
        for (int i = 0; i < tree.getWidth(); ++i) {
            Tile &tile = world.getTile(x + i, y + j - height);
            int curWall = tile.wallID;
            tile = tree.getTile(i, treeRow);
            tile.wallID = curWall;
        }
    }
}

void genPlants(const LocationBins &locations, Random &rnd, World &world)
{
    std::cout << "Growing plants\n";
    for (const auto &bin : locations) {
        for (auto [x, y] : bin.second) {
            switch (world.getTile(x, y).blockID) {
            case TileID::ashGrass:
                if (world.getTile(x, y - 1).liquid == Liquid::none &&
                    rnd.getDouble(0, 1) < 0.18) {
                    growTree(
                        x,
                        y,
                        TileID::ashGrass,
                        Data::ashTrees,
                        rnd,
                        world);
                }
                break;
            case TileID::grass:
                if (world.getTile(x, y - 1).liquid == Liquid::none &&
                    world.getTile(x, y - 1).wallID == WallID::empty &&
                    rnd.getDouble(0, 1) < 0.21) {
                    growTree(
                        x,
                        y,
                        TileID::grass,
                        Data::forestTrees,
                        rnd,
                        world);
                }
                break;
            case TileID::jungleGrass:
                if (y < world.getUndergroundLevel()) {
                    if (world.getTile(x, y - 1).liquid == Liquid::water) {
                        growBamboo(x, y, rnd, world);
                        if (world.getTile(x + 1, y).blockID ==
                            TileID::jungleGrass) {
                            growBamboo(x + 1, y, rnd, world);
                        }
                    } else if (
                        world.getTile(x, y - 1).wallID == WallID::empty &&
                        rnd.getDouble(0, 1) < 0.2) {
                        growTree(
                            x,
                            y,
                            TileID::jungleGrass,
                            Data::mahoganyTrees,
                            rnd,
                            world);
                    }
                } else if (
                    world.getTile(x, y - 1).liquid == Liquid::none &&
                    rnd.getDouble(0, 1) < 0.09) {
                    growTree(
                        x,
                        y,
                        TileID::jungleGrass,
                        Data::mahoganyUndergroundTrees,
                        rnd,
                        world);
                }
                break;
            case TileID::mushroomGrass:
                if (world.getTile(x, y - 1).liquid == Liquid::none &&
                    rnd.getDouble(0, 1) < 0.35) {
                    growTree(
                        x,
                        y,
                        TileID::mushroomGrass,
                        Data::mushroomTrees,
                        rnd,
                        world);
                }
                break;
            case TileID::sand:
                if (y < world.getUndergroundLevel()) {
                    growCactus(x, y, rnd, world);
                }
                break;
            }
        }
    }
}
