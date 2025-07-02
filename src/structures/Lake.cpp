#include "structures/Lake.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "ids/WallID.h"
#include "vendor/frozen/set.h"
#include <iostream>
#include <set>

bool isLiquidPathable(World &world, int x, int y)
{
    Tile &tile = world.getTile(x, y);
    return (tile.blockID == TileID::empty || tile.actuated) &&
           tile.liquid == Liquid::none;
}

template <typename Func>
std::tuple<int, int, int>
followRainFrom(World &world, int x, int y, Func isPathable)
{
    while (true) {
        if (isPathable(world, x, y + 1)) {
            ++y;
            if (y >= world.getHeight()) {
                return {x, x, y};
            }
            continue;
        }
        int flowLeft = x;
        for (; flowLeft > 0; --flowLeft) {
            if (isPathable(world, flowLeft, y + 1) ||
                !isPathable(world, flowLeft - 1, y)) {
                break;
            }
        }
        int flowRight = x;
        for (; flowRight < world.getWidth(); ++flowRight) {
            if (isPathable(world, flowRight, y + 1) ||
                !isPathable(world, flowRight + 1, y)) {
                break;
            }
        }
        if (x - flowLeft > flowRight - x &&
            isPathable(world, flowLeft, y + 1)) {
            x = flowLeft;
            continue;
        }
        if (isPathable(world, flowRight, y + 1)) {
            x = flowRight;
            continue;
        }
        if (isPathable(world, flowLeft, y + 1)) {
            x = flowLeft;
            continue;
        }
        return {flowLeft, flowRight + 1, y};
    }
}

void simulateRain(Random &rnd, World &world, int x)
{
    if (x % 4 != 0) {
        return;
    }
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    constexpr auto surfaceDryBlocks = frozen::make_set<int>(
        {TileID::ebonstone,
         TileID::ebonsand,
         TileID::lesion,
         TileID::corruptGrass,
         TileID::corruptJungleGrass,
         TileID::crimstone,
         TileID::crimsand,
         TileID::flesh,
         TileID::crimsonGrass,
         TileID::crimsonJungleGrass,
         TileID::pearlstone,
         TileID::pearlsand,
         TileID::aetherium,
         TileID::crystalBlock,
         TileID::livingWood,
         TileID::leaf,
         TileID::livingMahogany,
         TileID::mahoganyLeaf,
         TileID::ashGrass,
         TileID::sandstoneBrick,
         TileID::ebonstoneBrick,
         TileID::crimstoneBrick,
         TileID::pearlstoneBrick,
         TileID::blueBrick,
         TileID::greenBrick,
         TileID::pinkBrick});
    std::set<int> dryWalls{
        WallID::Safe::sandstoneBrick,
        WallID::Unsafe::sandstone,
        WallID::Unsafe::hardenedSand,
        WallID::Unsafe::hardenedEbonsand,
        WallID::Unsafe::hardenedCrimsand,
        WallID::Unsafe::hardenedPearlsand,
        WallID::Unsafe::ebonsandstone,
        WallID::Unsafe::crimsandstone,
        WallID::Unsafe::pearlsandstone,
        WallID::Unsafe::spider};
    dryWalls.insert(WallVariants::dungeon.begin(), WallVariants::dungeon.end());
    double pendingWater =
        world.conf.patches ? 0
        : std::abs(x - world.jungleCenter) <
                world.conf.jungleSize * 0.08 * world.getWidth()
            ? 15
            : -4;
    for (int y = world.getSurfaceLevel(x) - 45; y < world.getUnderworldLevel();
         y += 3) {
        if (!isLiquidPathable(world, x, y) ||
            (y < lavaLevel && dryWalls.contains(world.getTile(x, y).wallID))) {
            pendingWater = 2.1;
            continue;
        }
        pendingWater += world.getTile(x, y).wallID == WallID::Unsafe::hive ? 2.4
                        : world.conf.patches && y < lavaLevel
                            ? 1.15 + rnd.getHumidity(x, y)
                            : 1.6;
        auto [minDropX, maxDropX, dropY] =
            followRainFrom(world, x, y, isLiquidPathable);
        if (maxDropX - minDropX < pendingWater) {
            pendingWater -= maxDropX - minDropX;
            Tile &probeTile =
                world.getTile((minDropX + maxDropX) / 2, dropY + 1);
            if (probeTile.liquid == Liquid::shimmer ||
                probeTile.blockID == TileID::bubble ||
                (y < world.getUndergroundLevel() &&
                 (surfaceDryBlocks.contains(probeTile.blockID) ||
                  probeTile.liquid == Liquid::lava)) ||
                dropY > world.getUnderworldLevel() + 50) {
                continue;
            }
            if (y < world.getUndergroundLevel() &&
                (surfaceDryBlocks.contains(
                     world.getTile(minDropX - 1, dropY).blockID) ||
                 surfaceDryBlocks.contains(
                     world.getTile(maxDropX, dropY).blockID))) {
                continue;
            }
            for (int dropX = minDropX; dropX < maxDropX; ++dropX) {
                Tile &tile = world.getTile(dropX, dropY);
                tile.liquid = tile.wallID == WallID::Unsafe::hive
                                  ? Liquid::honey
                              : dropY > lavaLevel ? Liquid::lava
                                                  : Liquid::water;
                if (probeTile.blockID == TileID::hive &&
                    tile.liquid != Liquid::honey) {
                    tile.blockID = tile.liquid == Liquid::lava
                                       ? TileID::crispyHoney
                                       : TileID::honey;
                    tile.liquid = Liquid::none;
                }
            }
        }
    }
}

void evaporateSmallPools(World &world, int x)
{
    for (int y = world.getSurfaceLevel(x) - 50; y < world.getUnderworldLevel();
         ++y) {
        Tile &tile = world.getTile(x, y);
        if (tile.liquid != Liquid::water &&
            (tile.liquid != Liquid::lava || y < world.getUndergroundLevel())) {
            continue;
        }
        int poolDepth = std::get<2>(
            followRainFrom(world, x, y, [](World &world, int x, int y) {
                Tile &tile = world.getTile(x, y);
                return tile.liquid == Liquid::water ||
                       tile.liquid == Liquid::lava;
            }));
        if (poolDepth - y < 4 &&
            world.getTile(x, y - 1).blockID == TileID::empty) {
            while (y <= poolDepth) {
                world.getTile(x, y).liquid = Liquid::none;
                ++y;
            }
        } else if (
            poolDepth == y && world.getTile(x - 1, y).liquid == Liquid::none &&
            world.getTile(x + 1, y).liquid == Liquid::none) {
            world.getTile(x, y).liquid = Liquid::none;
        } else {
            if ((tile.wallID == WallID::Unsafe::snow ||
                 tile.wallID == WallID::Unsafe::ice) &&
                tile.liquid == Liquid::water) {
                tile.liquid = Liquid::none;
                tile.blockID = TileID::thinIce;
            }
            y = poolDepth;
        }
    }
}

void fillLavaHotzones(Random &rnd, World &world, int x)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    constexpr auto skipWalls = frozen::make_set<int>({
        WallID::empty,
        WallID::Unsafe::blueBrick,
        WallID::Unsafe::blueSlab,
        WallID::Unsafe::blueTiled,
        WallID::Unsafe::greenBrick,
        WallID::Unsafe::greenSlab,
        WallID::Unsafe::greenTiled,
        WallID::Unsafe::pinkBrick,
        WallID::Unsafe::pinkSlab,
        WallID::Unsafe::pinkTiled,
        WallID::Unsafe::lihzahrdBrick,
    });
    for (int y = world.getSurfaceLevel(x) + 10; y < lavaLevel; ++y) {
        if (world.getBiome(x, y).underworld < 0.99) {
            continue;
        }
        Tile &tile = world.getTile(x, y);
        if (tile.blockID == TileID::empty && !skipWalls.contains(tile.wallID) &&
            tile.liquid != Liquid::shimmer) {
            if (std::abs(rnd.getFineNoise(x, y)) < 0.06) {
                tile.blockID = TileID::obsidian;
            } else {
                tile.liquid = Liquid::lava;
            }
        }
    }
}

void genLake(Random &rnd, World &world)
{
    std::cout << "Raining\n";
    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        simulateRain(rnd, world, x);
    });
    parallelFor(std::views::iota(0, world.getWidth()), [&world](int x) {
        evaporateSmallPools(world, x);
    });
    if (world.conf.patches) {
        rnd.shuffleNoise();
        parallelFor(
            std::views::iota(0, world.getWidth()),
            [&rnd, &world](int x) { fillLavaHotzones(rnd, world, x); });
    }
}
