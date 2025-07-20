#include "structures/TorchArena.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include "structures/data/Torches.h"
#include "vendor/frozen/set.h"
#include <iostream>

Point selectArenaLocation(
    int arenaWidth,
    int arenaHeight,
    Random &rnd,
    World &world)
{
    constexpr auto clearableTiles = frozen::make_set<int>({
        TileID::empty,
        TileID::dirt,
        TileID::stone,
        TileID::mud,
        TileID::sand,
        TileID::clay,
        TileID::ironOre,
        TileID::leadOre,
        TileID::silverOre,
        TileID::tungstenOre,
        TileID::goldOre,
        TileID::platinumOre,
        TileID::cobaltOre,
        TileID::palladiumOre,
        TileID::mythrilOre,
        TileID::orichalcumOre,
        TileID::adamantiteOre,
        TileID::titaniumOre,
    });
    int minX = 100;
    int maxX = world.getWidth() - 100 - arenaWidth;
    int minY =
        std::midpoint(world.getCavernLevel(), world.getUnderworldLevel()) -
        arenaHeight;
    int maxY = world.getUnderworldLevel() - arenaHeight;
    int maxFoundationEmpty = 0.4 * arenaWidth;
    int padding = world.conf.hiveQueen ? 6 : 20;
    for (int tries = 0; tries < 8000; ++tries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        if (world.conf.hiveQueen) {
            while (world.getBiome(x, y).active == Biome::jungle) {
                x = rnd.getInt(minX, maxX);
                y = rnd.getInt(minY, maxY);
            }
        }
        if (hypot(world.gemGrove, {x, y}) < 150) {
            continue;
        }
        int numEmpty = 0;
        int numFilled = 0;
        int maxEntryFilled = tries / 250;
        if (world.conf.hiveQueen) {
            maxEntryFilled += 5;
        }
        if (world.regionPasses(
                x - 3,
                y + arenaHeight - 9,
                4,
                4,
                [maxEntryFilled, &numFilled](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < maxEntryFilled;
                }) &&
            world.regionPasses(
                x + arenaWidth - 1,
                y + arenaHeight - 9,
                4,
                4,
                [maxEntryFilled, &numFilled](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < maxEntryFilled;
                }) &&
            world.regionPasses(
                x - padding / 2,
                y - padding / 2,
                arenaWidth + padding,
                arenaHeight + padding,
                [&clearableTiles](Tile &tile) {
                    return !tile.guarded && tile.liquid != Liquid::shimmer &&
                           clearableTiles.contains(tile.blockID);
                }) &&
            world.regionPasses(
                x,
                y + arenaHeight - 2,
                arenaWidth,
                4,
                [maxFoundationEmpty, &numEmpty](Tile &tile) {
                    if (tile.blockID == TileID::empty) {
                        ++numEmpty;
                    }
                    return numEmpty < maxFoundationEmpty;
                })) {
            return {x, y};
        }
    }
    return {-1, -1};
}

void placeTorchStructure(int x, int y, TileBuffer &data, World &world)
{
    for (int i = 0; i < data.getWidth(); ++i) {
        for (int j = 0; j < data.getHeight(); ++j) {
            Tile &dataTile = data.getTile(i, j);
            if (dataTile.wallID == WallID::Safe::cloud) {
                if (world.getTile(x + i, y + j).wallID == WallID::empty) {
                    dataTile.wallID = WallID::Unsafe::ember;
                    dataTile.echoCoatWall = true;
                } else {
                    dataTile.wallID = WallID::empty;
                }
            }
        }
    }
    world.placeBuffer(x, y, data);
    world.queuedDeco.emplace_back(
        [x, y, dW = data.getWidth(), dH = data.getHeight()](
            Random &,
            World &world) {
            for (int i = 0; i < dW; ++i) {
                for (int j = 0; j < dH; ++j) {
                    world.getTile(x + i, y + j).liquid = Liquid::none;
                }
            }
        });
}

void addArenaBubble(int x, int y, World &world)
{
    if (!world.conf.sunken || !world.conf.shattered || x < 420 ||
        x > world.getWidth() - 420) {
        return;
    }
    double maxS = 45.5;
    double minS = maxS - 3;
    for (int i = -maxS; i < maxS; ++i) {
        for (int j = -maxS; j < maxS; ++j) {
            Tile &tile = world.getTile(x + i, y + j);
            if (tile.blockID != TileID::empty) {
                continue;
            }
            double dist = std::hypot(i, j);
            if (dist < maxS && dist > minS) {
                tile.blockID = TileID::bubble;
            }
        }
    }
}

void genTorchArena(Random &rnd, World &world)
{
    std::cout << "Fueling TORCH\n";
    int arenaWidth = rnd.getInt(90, 110);
    int arenaHeight = rnd.getInt(45, 52);
    auto [x, y] = selectArenaLocation(arenaWidth, arenaHeight, rnd, world);
    if (x == -1) {
        return;
    }
    arenaWidth /= 2;
    x += arenaWidth;
    arenaWidth += 3;
    arenaHeight -= 5;
    y += arenaHeight;
    int favorBase = -3;
    for (int i = -arenaWidth; i < arenaWidth; ++i) {
        int arenaBase = 3.5 * (10 + arenaWidth - std::abs(i)) *
                        rnd.getFineNoise(x + i, 0) / arenaWidth;
        if (i > -4 && i < 4) {
            favorBase = std::max(favorBase, arenaBase);
        }
        for (int j = -arenaHeight; j < arenaBase; ++j) {
            double threshold = 3.5 * std::hypot(
                                         static_cast<double>(i) / arenaWidth,
                                         static_cast<double>(j) / arenaHeight) -
                               2.5;
            if (rnd.getFineNoise(x + i, y + j) > threshold) {
                world.getTile(x + i, y + j).blockID = TileID::empty;
            }
        }
    }
    TileBuffer torchFavor =
        Data::getTorch(Data::Torch::favor, world.getFramedTiles());
    placeTorchStructure(
        x - torchFavor.getWidth() / 2,
        y + favorBase + 1 - torchFavor.getHeight(),
        torchFavor,
        world);
    addArenaBubble(x, y + favorBase - torchFavor.getHeight(), world);
    int offset =
        rnd.select({-1, 1}) * (torchFavor.getWidth() / 2 + arenaWidth / 4);
    TileBuffer torch = Data::getTorch(
        rnd.getBool() ? Data::Torch::up
        : offset > 0  ? Data::Torch::left
                      : Data::Torch::right,
        world.getFramedTiles());
    offset -= torch.getWidth() / 2;
    int torchBase = -3;
    for (int i = 2; i < torch.getWidth() - 2; ++i) {
        int curBase =
            scanWhileEmpty({x + offset + i, y - 3}, {0, 1}, world).y - y;
        if (curBase < 5) {
            torchBase = std::max(torchBase, curBase);
        }
    }
    placeTorchStructure(
        x + offset,
        y + torchBase + 2 - torch.getHeight(),
        torch,
        world);
}
