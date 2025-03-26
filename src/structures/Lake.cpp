#include "structures/Lake.h"

#include "Util.h"
#include "World.h"
#include <iostream>
#include <set>

bool isLiquidPathable(World &world, int x, int y)
{
    Tile &tile = world.getTile(x, y);
    return tile.blockID == TileID::empty && tile.liquid == Liquid::none;
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

void simulateRain(World &world, int minX, int maxX)
{
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    std::set<int> surfaceDryBlocks{
        TileID::ebonstone,
        TileID::ebonsand,
        TileID::lesion,
        TileID::corruptGrass,
        TileID::corruptJungleGrass,
        TileID::crimstone,
        TileID::crimsand,
        TileID::flesh,
        TileID::crimsonGrass,
        TileID::crimsonJungleGrass};
    std::set<int> dryWalls{
        WallID::Unsafe::sandstone,
        WallID::Unsafe::hardenedSand,
        WallID::Unsafe::hardenedEbonsand,
        WallID::Unsafe::hardenedCrimsand,
        WallID::Unsafe::ebonsandstone,
        WallID::Unsafe::crimsandstone};
    for (int x = minX; x < maxX; x += 4) {
        double pendingWater =
            std::abs(x - world.jungleCenter) < 0.08 * world.getWidth() ? 15 : 2;
        for (int y = world.spawnY - 45; y < world.getUnderworldLevel();
             y += 3) {
            if (!isLiquidPathable(world, x, y) ||
                (y < lavaLevel &&
                 dryWalls.contains(world.getTile(x, y).wallID))) {
                pendingWater = 2.1;
                continue;
            }
            pendingWater +=
                world.getTile(x, y).wallID == WallID::Unsafe::hive ? 1.9 : 1.1;
            auto [minDropX, maxDropX, dropY] =
                followRainFrom(world, x, y, isLiquidPathable);
            if (maxDropX - minDropX < pendingWater) {
                pendingWater -= maxDropX - minDropX;
                Tile &probeTile =
                    world.getTile((minDropX + maxDropX) / 2, dropY + 1);
                if (probeTile.liquid == Liquid::shimmer ||
                    probeTile.blockID == TileID::bubble ||
                    (y < world.getUndergroundLevel() &&
                     surfaceDryBlocks.contains(probeTile.blockID))) {
                    continue;
                }
                for (int dropX = minDropX; dropX < maxDropX; ++dropX) {
                    Tile &tile = world.getTile(dropX, dropY);
                    tile.liquid = tile.wallID == WallID::Unsafe::hive
                                      ? Liquid::honey
                                  : dropY > lavaLevel ? Liquid::lava
                                                      : Liquid::water;
                }
            }
        }
    }
}

void evaporateSmallPools(World &world, int minX, int maxX)
{
    for (int x = minX; x < maxX; ++x) {
        for (int y = 0; y < world.getUnderworldLevel(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (tile.liquid != Liquid::water && tile.liquid != Liquid::lava) {
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
}

void genLake(World &world)
{
    std::cout << "Raining\n";
    int numSegments = world.getWidth() / 500;
    int segmentSize = 1 + world.getWidth() / numSegments;
    parallelFor(
        std::views::iota(0, numSegments),
        [segmentSize, &world](int segment) {
            simulateRain(
                world,
                segmentSize * segment,
                std::min(segmentSize * (1 + segment), world.getWidth()));
        });
    parallelFor(
        std::views::iota(0, numSegments),
        [segmentSize, &world](int segment) {
            evaporateSmallPools(
                world,
                segmentSize * segment,
                std::min(segmentSize * (1 + segment), world.getWidth()));
        });
}
