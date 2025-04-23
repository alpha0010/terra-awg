#include "structures/Plants.h"

#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/data/Trees.h"
#include <iostream>
#include <set>

bool isRegionEmpty(int x, int y, int width, int height, World &world)
{
    return world.regionPasses(x, y, width, height, [](Tile &tile) {
        return tile.blockID == TileID::empty;
    });
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
    if (rnd.getDouble(0, 1) > 0.07 ||
        !isRegionEmpty(x - 1, y - 8, 3, 8, world)) {
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

void growPalmTree(int x, int y, Random &rnd, World &world)
{
    if (rnd.getDouble(0, 1) > 0.3) {
        return;
    }
    for (int i = 0; i < 3; ++i) {
        if (world.getTile(x + i, y).blockID != TileID::sand) {
            return;
        }
    }
    if (world.getTile(x - 1, y - 2).blockID == TileID::palmTree ||
        world.getTile(x - 2, y - 2).blockID == TileID::palmTree) {
        return;
    }
    int height = rnd.getInt(10, 20);
    if (!isRegionEmpty(x, y - 4 - height, 3, height + 4, world)) {
        return;
    }
    int bend = 0;
    int targetBend = 2 * rnd.getInt(-8, 8);
    for (int j = 0; j < height; ++j) {
        Tile &tile = world.getTile(x + 1, y - j - 1);
        tile.blockID = TileID::palmTree;
        tile.frameX = j == height - 1 ? 22 * rnd.getInt(4, 6)
                      : j == 0        ? 66
                                      : 22 * rnd.getInt(0, 2);
        tile.frameY = bend;
        if (j * 4 > height) {
            if (bend > targetBend) {
                bend -= 2;
            } else if (bend < targetBend) {
                bend += 2;
            }
        }
    }
}

void growSandPlant(int x, int y, Random &rnd, World &world)
{
    Tile &probeTile = world.getTile(x, y - 1);
    if (probeTile.wallID != WallID::empty || probeTile.liquid != Liquid::none) {
        return;
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
    if (sandCount < 15) {
        return;
    }
    if (x < 380 || x > world.getWidth() - 380) {
        growPalmTree(x, y, rnd, world);
        return;
    }
    int waterCount = 0;
    for (int i = -50; i < 50; ++i) {
        for (int j = -25; j < 25; ++j) {
            if (world.getTile(x + i, y + j).liquid == Liquid::water) {
                ++waterCount;
                if (waterCount > 5) {
                    growPalmTree(x, y, rnd, world);
                    return;
                }
            }
        }
    }
    growCactus(x, y, rnd, world);
}

void growRollingCactus(int x, int y, World &world)
{
    Tile &probeTile = world.getTile(x, y - 1);
    if ((probeTile.wallID != WallID::Unsafe::sandstone &&
         probeTile.wallID != WallID::Unsafe::hardenedSand) ||
        probeTile.liquid != Liquid::none) {
        return;
    }
    world.queuedTraps.emplace_back([x, y](Random &, World &world) {
        if (isRegionEmpty(x - 1, y - 3, 4, 3, world)) {
            world.placeFramedTile(x, y - 2, TileID::rollingCactus);
        }
    });
}

void growTree(
    int x,
    int y,
    int groundTile,
    int treeTile,
    Random &rnd,
    World &world)
{
    for (int i = 0; i < 3; ++i) {
        if (world.getTile(x + i, y).blockID != groundTile) {
            return;
        }
    }
    std::set<int> treeTiles{
        TileID::tree,
        TileID::sakuraTree,
        TileID::yellowWillowTree,
        TileID::ashTree,
        TileID::amethystTree,
        TileID::topazTree,
        TileID::sapphireTree,
        TileID::emeraldTree,
        TileID::rubyTree,
        TileID::amberTree,
        TileID::diamondTree};
    if (treeTiles.contains(world.getTile(x - 1, y - 2).blockID) ||
        treeTiles.contains(world.getTile(x - 2, y - 2).blockID)) {
        return;
    }
    int height = rnd.getInt(7, 15);
    if (!isRegionEmpty(x, y - 4 - height, 3, height + 4, world)) {
        return;
    }
    for (int j = 0; j < height; ++j) {
        TileBuffer tree =
            Data::getTree(rnd.select(Data::trees), world.getFramedTiles());
        int treeRow = j > height - 3 ? j - height + tree.getHeight()
                      : j < 2        ? j
                                     : rnd.getInt(2, tree.getHeight() - 3);
        for (int i = 0; i < tree.getWidth(); ++i) {
            Tile &tile = world.getTile(x + i, y + j - height);
            int curWall = tile.wallID;
            tile = tree.getTile(i, treeRow);
            if (tile.blockID != TileID::empty) {
                tile.blockID = treeTile;
            }
            tile.wallID = curWall;
        }
    }
}

void genPlants(const LocationBins &locations, Random &rnd, World &world)
{
    std::cout << "Growing plants\n";
    for (const auto &bin : locations) {
        for (auto [x, y] : bin.second) {
            int curTileID = world.getTile(x, y).blockID;
            switch (curTileID) {
            case TileID::ashGrass:
                if (world.getTile(x, y - 1).liquid == Liquid::none &&
                    rnd.getDouble(0, 1) < 0.18) {
                    growTree(
                        x,
                        y,
                        TileID::ashGrass,
                        TileID::ashTree,
                        rnd,
                        world);
                }
                break;
            case TileID::corruptGrass:
            case TileID::corruptJungleGrass:
            case TileID::crimsonGrass:
            case TileID::crimsonJungleGrass:
            case TileID::grass:
            case TileID::snow:
                if (y < world.getUndergroundLevel() &&
                    world.getTile(x, y - 1).liquid == Liquid::none &&
                    world.getTile(x, y - 1).wallID == WallID::empty &&
                    rnd.getDouble(0, 1) < 0.21) {
                    growTree(
                        x,
                        y,
                        curTileID,
                        curTileID == TileID::grass && rnd.getDouble(0, 1) < 0.1
                            ? rnd.select(
                                  {TileID::sakuraTree,
                                   TileID::yellowWillowTree})
                            : TileID::tree,
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
                            TileID::tree,
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
                        TileID::tree,
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
                        TileID::tree,
                        rnd,
                        world);
                }
                break;
            case TileID::sand:
                if (y < world.getUndergroundLevel()) {
                    growSandPlant(x, y, rnd, world);
                    break;
                }
                [[fallthrough]];
            case TileID::hardenedSand:
            case TileID::sandstone:
                if (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                        11 ==
                    0) {
                    growRollingCactus(x, y, world);
                }
                break;
            case TileID::stone:
                if (y > world.getCavernLevel() &&
                    world.getTile(x, y - 1).liquid == Liquid::none &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            100 ==
                        0) {
                    growTree(
                        x,
                        y,
                        TileID::stone,
                        rnd.select(
                            {TileID::amethystTree,
                             TileID::topazTree,
                             TileID::sapphireTree,
                             TileID::emeraldTree,
                             TileID::rubyTree,
                             TileID::amberTree,
                             TileID::diamondTree}),
                        rnd,
                        world);
                }
                break;
            }
        }
    }
}

void growGrass(int x, int y, Random &rnd, World &world)
{
    Tile &baseTile = world.getTile(x, y);
    Tile &probeTile = world.getTile(x, y - 1);
    if (baseTile.slope != Slope::none || probeTile.blockID != TileID::empty ||
        probeTile.liquid != Liquid::none) {
        return;
    }
    int randInt = 99999 * (1 + rnd.getFineNoise(x, y));
    if (randInt % 61 == 0) {
        probeTile.blockID = TileID::herb;
        switch (baseTile.blockID) {
        case TileID::ash:
        case TileID::ashGrass:
            probeTile.frameX = 90;
            return;
        case TileID::corruptGrass:
        case TileID::corruptJungleGrass:
        case TileID::crimsonGrass:
        case TileID::crimsonJungleGrass:
        case TileID::ebonstone:
        case TileID::crimstone:
            probeTile.frameX = 54;
            return;
        case TileID::dirt:
        case TileID::mud:
            probeTile.frameX = 36;
            return;
        case TileID::grass:
            return;
        case TileID::jungleGrass:
            probeTile.frameX = 18;
            return;
        case TileID::sand:
            probeTile.frameX = 72;
            return;
        case TileID::snow:
        case TileID::ice:
            probeTile.frameX = 108;
            return;
        default:
            probeTile.blockID = TileID::empty;
        }
    }
    switch (baseTile.blockID) {
    case TileID::ashGrass:
        world.placeFramedTile(x, y - 1, TileID::ashPlant);
        break;
    case TileID::corruptGrass:
        world.placeFramedTile(x, y - 1, TileID::corruptPlant);
        break;
    case TileID::crimsonGrass:
        world.placeFramedTile(x, y - 1, TileID::crimsonPlant);
        break;
    case TileID::grass:
        world.placeFramedTile(
            x,
            y - 1,
            rnd.getCoarseNoise(x, y) > 0 ? TileID::tallGrassPlant
                                         : TileID::grassPlant);
        break;
    case TileID::jungleGrass:
        if (isRegionEmpty(x + 1, y - 2, 2, 2, world) &&
            world.regionPasses(
                x + 1,
                y,
                2,
                1,
                [](Tile &tile) {
                    return tile.blockID == TileID::jungleGrass &&
                           tile.slope == Slope::none;
                }) &&
            randInt % 3 != 0) {
            world.placeFramedTile(x, y - 2, TileID::largeJunglePlant);
            break;
        }
        world.placeFramedTile(
            x,
            y - 1,
            rnd.getCoarseNoise(x, y) > 0 ? TileID::tallJunglePlant
                                         : TileID::junglePlant);
        break;
    case TileID::mushroomGrass:
        world.placeFramedTile(x, y - 1, TileID::mushroomPlant);
        break;
    }
}

void genGrasses(const LocationBins &locations, Random &rnd, World &world)
{
    std::cout << "Growing plants\n";
    for (const auto &bin : locations) {
        for (auto [x, y] : bin.second) {
            growGrass(x, y, rnd, world);
            growGrass(x + 1, y, rnd, world);
        }
    }
}
