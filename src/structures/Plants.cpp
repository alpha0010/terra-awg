#include "structures/Plants.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/Traps.h"
#include "structures/data/Trees.h"
#include "vendor/frozen/set.h"
#include <iostream>

bool isRegionEmpty(int x, int y, int width, int height, World &world)
{
    return world.regionPasses(x, y, width, height, [](Tile &tile) {
        return tile.blockID == TileID::empty;
    });
}

bool isSunken(int x, int y, World &world)
{
    return world.conf.sunken && y < world.getUndergroundLevel() &&
           y > std::max(
                   world.getSurfaceLevel(world.getWidth() / 2 - 130),
                   world.getSurfaceLevel(world.getWidth() / 2 + 130)) &&
           std::hypot(world.dungeonX - x, world.dungeonY - y) > 78;
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
    int paint = world.getTile(x, y).blockPaint;
    for (int i = 0; i < bambooSize; ++i) {
        Tile &tile = world.getTile(x, y - i - 1);
        tile.blockID = TileID::bambooStalk;
        tile.blockPaint = paint;
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
        if (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 13 == 0) {
            world.queuedDeco.emplace_back([x, y](Random &, World &world) {
                if (isRegionEmpty(x, y - 2, 3, 2, world) &&
                    world.regionPasses(x, y, 3, 1, [](Tile &tile) {
                        return tile.blockID == TileID::sand &&
                               tile.slope == Slope::none;
                    })) {
                    world.placeFramedTile(
                        x,
                        y - 2,
                        TileID::largePileGroup2,
                        Variant::dryBone);
                }
            });
        }
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
    if (isSunken(x, y, world)) {
        return;
    }
    Tile &probeTile = world.getTile(x, y - 1);
    if (probeTile.wallID != WallID::empty || probeTile.liquid != Liquid::none) {
        return;
    }
    int sandCount = 0;
    for (int i = -10; i < 10; ++i) {
        for (int j = -8; j < 8; ++j) {
            switch (world.getTile(x + i, y + j).blockID) {
            case TileID::grass:
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
    if (isSunken(x, y, world)) {
        return;
    }
    for (int i = 0; i < 3; ++i) {
        if (world.getTile(x + i, y).blockID != groundTile) {
            return;
        }
    }
    constexpr auto treeTiles = frozen::make_set<int>(
        {TileID::tree,
         TileID::sakuraTree,
         TileID::yellowWillowTree,
         TileID::ashTree,
         TileID::amethystTree,
         TileID::topazTree,
         TileID::sapphireTree,
         TileID::emeraldTree,
         TileID::rubyTree,
         TileID::amberTree,
         TileID::diamondTree});
    if (treeTiles.contains(world.getTile(x - 1, y - 2).blockID) ||
        treeTiles.contains(world.getTile(x - 2, y - 2).blockID)) {
        return;
    }
    int height = rnd.getInt(7, 15);
    if (!isRegionEmpty(x, y - 4 - height, 3, height + 4, world)) {
        return;
    }
    if (y < world.getUndergroundLevel()) {
        for (int i = 0; i < 3; ++i) {
            world.getTile(x + i, y).wallID = WallID::empty;
        }
    }
    int paint = world.getTile(x + 1, y).blockPaint;
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
                tile.blockPaint = paint;
            }
            tile.wallID = curWall;
        }
    }
}

bool inGemGrove(int x, int y, World &world)
{
    return std::hypot(world.gemGroveX - x, world.gemGroveY - y) <
           world.gemGroveSize;
}

void genPlants(const LocationBins &locations, Random &rnd, World &world)
{
    std::cout << "Growing trees\n";
    std::vector<Point> oreLocations;
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
            case TileID::hallowedGrass:
            case TileID::snow:
                if (y < world.getUndergroundLevel() &&
                    world.getTile(x, y - 1).liquid == Liquid::none &&
                    world.getTile(x, y - 1).wallID == WallID::empty &&
                    rnd.getDouble(0, 1) < 0.21) {
                    growTree(
                        x,
                        y,
                        curTileID,
                        curTileID == TileID::grass &&
                                rnd.getDouble(0, 1) <
                                    (world.conf.doubleTrouble ? 0.23 : 0.1)
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
                        std::max<int>(
                            11 / std::max(world.conf.traps, 0.1),
                            2) ==
                    0) {
                    growRollingCactus(x, y, world);
                }
                break;
            case TileID::stone:
            case TileID::pearlstone:
                if (y > world.getCavernLevel() &&
                    world.getTile(x, y - 1).liquid == Liquid::none &&
                    (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                             100 ==
                         0 ||
                     (inGemGrove(x, y, world) && rnd.getDouble(0, 1) < 0.85))) {
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
            case TileID::copperOre:
            case TileID::tinOre:
            case TileID::ironOre:
            case TileID::leadOre:
            case TileID::silverOre:
            case TileID::tungstenOre:
            case TileID::goldOre:
            case TileID::platinumOre:
                oreLocations.emplace_back(x, y);
                break;
            }
        }
    }
    world.queuedTraps.emplace_back(
        [locs = std::move(oreLocations)](Random &rnd, World &world) mutable {
            addOreTraps(std::move(locs), rnd, world);
        });
}

void placeLivingTreeDecoAt(int x, Random &rnd, World &world)
{
    int y = world.getSurfaceLevel(x);
    switch (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 5) {
    case 0:
        for (int j = -10; j < 10; ++j) {
            if (world.regionPasses(
                    x,
                    y + j,
                    2,
                    1,
                    [](Tile &tile) {
                        return tile.blockID == TileID::empty &&
                               tile.wallID == WallID::empty;
                    }) &&
                world.regionPasses(x, y + j + 1, 2, 1, [](Tile &tile) {
                    return tile.slope == Slope::none &&
                           (tile.blockID == TileID::grass ||
                            tile.blockID == TileID::livingWood);
                })) {
                world.placeFramedTile(
                    x,
                    y + j,
                    TileID::smallPile,
                    Variant::livingWood);
                return;
            }
        }
        break;
    case 1:
        for (int j = -10; j < 10; ++j) {
            if (world.regionPasses(
                    x,
                    y + j,
                    3,
                    2,
                    [](Tile &tile) {
                        return tile.blockID == TileID::empty &&
                               tile.wallID == WallID::empty;
                    }) &&
                world.regionPasses(x, y + j + 2, 3, 1, [](Tile &tile) {
                    return tile.slope == Slope::none &&
                           (tile.blockID == TileID::grass ||
                            tile.blockID == TileID::livingWood);
                })) {
                world.placeFramedTile(
                    x,
                    y + j,
                    TileID::largePileGroup2,
                    Variant::livingWood);
                return;
            }
        }
        break;
    case 2:
    case 3:
        for (int j = -120; j < -10; ++j) {
            if (world.regionPasses(
                    x,
                    y + j + 2,
                    3,
                    1,
                    [](Tile &tile) {
                        return tile.blockID == TileID::leaf &&
                               tile.slope == Slope::none;
                    }) &&
                world.regionPasses(x, y + j, 3, 2, [](Tile &tile) {
                    return tile.blockID == TileID::empty &&
                           tile.wallID == WallID::empty;
                })) {
                world.placeFramedTile(
                    x,
                    y + j,
                    TileID::largePileGroup2,
                    Variant::livingLeaf);
                return;
            }
        }
        break;
    }
}

void growLivingTreeDeco(Random &rnd, World &world)
{
    int doPlacementUntil = -1;
    for (int x = 300; x < world.getWidth() - 300; ++x) {
        if (world.getTile(x, world.getSurfaceLevel(x)).blockID ==
            TileID::livingWood) {
            doPlacementUntil = x + 20;
        }
        if (x - 20 < doPlacementUntil) {
            placeLivingTreeDecoAt(x - 20, rnd, world);
        }
    }
}

bool placeSmallPile(int x, int y, World &world)
{
    Tile &nextBase = world.getTile(x + 1, y);
    if (nextBase.blockID != world.getTile(x, y).blockID ||
        nextBase.slope != Slope::none ||
        world.getTile(x + 1, y - 1).blockID != TileID::empty) {
        return false;
    }
    if (!world.regionPasses(x - 1, y - 1, 4, 3, [](Tile &tile) {
            return tile.wallID != WallID::Unsafe::spider;
        })) {
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::spider);
        return true;
    }
    switch (nextBase.blockID) {
    case TileID::ash:
    case TileID::ashGrass:
    case TileID::blueBrick:
    case TileID::greenBrick:
    case TileID::pinkBrick:
    case TileID::hellstone:
    case TileID::hellstoneBrick:
    case TileID::obsidianBrick:
        world.placeFramedTile(
            x,
            y - 1,
            TileID::smallPile,
            y < world.getUndergroundLevel() ? Variant::stone : Variant::bone);
        return true;
    case TileID::dirt:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::dirt);
        return true;
    case TileID::granite:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::granite);
        return true;
    case TileID::grass:
    case TileID::hallowedGrass:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::forest);
        return true;
    case TileID::ice:
    case TileID::snow:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::ice);
        return true;
    case TileID::marble:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::marble);
        return true;
    case TileID::sandstone:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::sandstone);
        return true;
    case TileID::stone:
        world.placeFramedTile(x, y - 1, TileID::smallPile, Variant::stone);
        return true;
    }
    return false;
}

bool placeLargePile(int x, int y, World &world)
{
    if (!isRegionEmpty(x, y - 2, 3, 2, world) ||
        !world.regionPasses(x + 1, y, 2, 1, [](Tile &tile) {
            return tile.slope == Slope::none && isSolidBlock(tile.blockID);
        })) {
        return false;
    }
    if (!world.regionPasses(x - 1, y - 1, 5, 4, [](Tile &tile) {
            return tile.wallID != WallID::Unsafe::spider;
        })) {
        world.placeFramedTile(
            x,
            y - 2,
            TileID::largePileGroup2,
            Variant::spider);
        return true;
    }
    switch (world.getTile(x + 1, y).blockID) {
    case TileID::ash:
    case TileID::ashGrass:
    case TileID::hellstone:
    case TileID::hellstoneBrick:
    case TileID::obsidianBrick:
        if (y > world.getUnderworldLevel() && fnv1a32pt(x, y) % 5 == 0) {
            world.placeFramedTile(x, y - 2, TileID::largePile, Variant::bone);
        } else {
            world.placeFramedTile(
                x,
                y - 2,
                TileID::largePileGroup2,
                Variant::ash);
        }
        return true;
    case TileID::blueBrick:
    case TileID::greenBrick:
    case TileID::pinkBrick:
        if (y > world.getUndergroundLevel() && fnv1a32pt(x, y) % 5 == 0) {
            world.placeFramedTile(x, y - 2, TileID::largePile, Variant::bone);
            return true;
        }
        return false;
    case TileID::granite:
        world.placeFramedTile(
            x,
            y - 2,
            TileID::largePileGroup2,
            Variant::granite);
        return true;
    case TileID::grass:
    case TileID::hallowedGrass:
        world.placeFramedTile(
            x,
            y - 2,
            TileID::largePileGroup2,
            Variant::forest);
        return true;
    case TileID::ice:
    case TileID::snow:
        world.placeFramedTile(x, y - 2, TileID::largePile, Variant::ice);
        return true;
    case TileID::jungleGrass:
        world.placeFramedTile(
            x,
            y - 2,
            TileID::largePileGroup2,
            Variant::jungle);
        return true;
    case TileID::lihzahrdBrick:
        if (world.getTile(x + 1, y - 1).wallID ==
            WallID::Unsafe::lihzahrdBrick) {
            world.placeFramedTile(
                x,
                y - 2,
                TileID::largePileGroup2,
                Variant::lihzahrd);
            return true;
        }
        return false;
    case TileID::marble:
        world.placeFramedTile(
            x,
            y - 2,
            TileID::largePileGroup2,
            Variant::marble);
        return true;
    case TileID::mushroomGrass:
        world.placeFramedTile(x, y - 2, TileID::largePile, Variant::mushroom);
        return true;
    case TileID::sandstone:
    case TileID::ebonsandstone:
    case TileID::crimsandstone:
    case TileID::pearlsandstone:
        world.placeFramedTile(
            x,
            y - 2,
            TileID::largePileGroup2,
            Variant::sandstone);
        return true;
    case TileID::stone:
        world.placeFramedTile(x, y - 2, TileID::largePile, Variant::stone);
        return true;
    }
    return false;
}

bool placeSunflower(int x, int y, World &world)
{
    Tile &nextBase = world.getTile(x + 1, y);
    if (nextBase.blockID != TileID::grass || nextBase.slope != Slope::none ||
        !world.regionPasses(x, y - 4, 2, 4, [](Tile &tile) {
            return tile.blockID == TileID::empty &&
                   tile.wallID == WallID::empty;
        })) {
        return false;
    }
    world.placeFramedTile(x, y - 4, TileID::sunflower);
    return true;
}

bool placeGeyser(int x, int y, World &world)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    if (y > lavaLevel && isRegionEmpty(x, y - 3, 2, 3, world) &&
        world.regionPasses(x, y, 2, 1, [&world](Tile &tile) {
            return tile.slope == Slope::none &&
                   (tile.blockID == TileID::ash ||
                    tile.blockID == TileID::jungleGrass ||
                    tile.blockID == TileID::stone ||
                    (tile.blockID == TileID::obsidianBrick &&
                     tile.wallID != WallID::Unsafe::obsidianBrick &&
                     world.conf.traps > 14));
        })) {
        world.placeFramedTile(x, y - 1, TileID::geyser);
        return true;
    }
    return false;
}

void growGrass(int x, int y, Random &rnd, World &world)
{
    Tile &baseTile = world.getTile(x, y);
    Tile &probeTile = world.getTile(x, y - 1);
    if (baseTile.slope != Slope::none || probeTile.blockID != TileID::empty) {
        return;
    }
    int randInt = 99999 * (1 + rnd.getFineNoise(x, y));
    if (probeTile.liquid == Liquid::water &&
        (baseTile.blockID == TileID::sand ||
         baseTile.blockID == TileID::coralstone) &&
        (x < 200 || x > world.getWidth() - 200) && randInt % 7 == 0) {
        probeTile.blockID = TileID::coral;
        probeTile.frameX = 26 * (fnv1a32pt(x, y) % 6);
        return;
    }
    if (probeTile.liquid != Liquid::none) {
        return;
    }
    if (randInt % 61 < 2) {
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
        case TileID::hallowedGrass:
            return;
        case TileID::jungleGrass:
            if (randInt % 61 == 0) {
                probeTile.frameX = 18;
                return;
            }
            break;
        case TileID::sand:
        case TileID::pearlsand:
            if (x > 350 && x < world.getWidth() - 350) {
                probeTile.frameX = 72;
                return;
            }
            break;
        case TileID::snow:
        case TileID::ice:
            if (randInt % 61 == 0) {
                probeTile.frameX = 108;
                return;
            }
            break;
        }
        probeTile.blockID = TileID::empty;
    }
    bool increasedPileRate = probeTile.wallID == WallID::Unsafe::granite ||
                             probeTile.wallID == WallID::Unsafe::marble;
    if (randInt % 41 < (increasedPileRate ? 5 : 1) &&
        placeSmallPile(x, y, world)) {
        return;
    }
    if (randInt % 37 < (increasedPileRate ? 6 : 1) &&
        placeLargePile(x, y, world)) {
        return;
    }
    if (randInt % std::max<int>(131 / std::max(world.conf.traps, 0.1), 7) ==
            0 &&
        placeGeyser(x, y, world)) {
        return;
    }
    if (baseTile.blockID == TileID::grass && randInt % 3 == 0 &&
        rnd.getCoarseNoise(x, y) < -0.3 && placeSunflower(x, y, world)) {
        return;
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
        if (isRegionEmpty(x, y - 2, 3, 2, world) &&
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
        world.getTile(x, y - 1).blockPaint = world.getTile(x, y).blockPaint;
        break;
    case TileID::mushroomGrass:
        world.placeFramedTile(x, y - 1, TileID::mushroomPlant);
        world.getTile(x, y - 1).blockPaint = world.getTile(x, y).blockPaint;
        break;
    }
}

void genGrasses(const LocationBins &locations, Random &rnd, World &world)
{
    std::cout << "Growing plants\n";
    rnd.shuffleNoise();
    for (const auto &applyQueuedDeco : world.queuedDeco) {
        applyQueuedDeco(rnd, world);
    }
    growLivingTreeDeco(rnd, world);
    for (const auto &bin : locations) {
        for (auto [x, y] : bin.second) {
            growGrass(x, y, rnd, world);
            growGrass(x + 1, y, rnd, world);
        }
    }
}
