#include "structures/DesertTomb.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/Paint.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include "structures/data/Buildings.h"
#include "vendor/frozen/set.h"
#include <algorithm>
#include <iostream>

Point selectTombLocation(TileBuffer &tomb, Random &rnd, World &world)
{
    constexpr auto clearableTiles = frozen::make_set<int>({
        TileID::empty,       TileID::sand,          TileID::hardenedSand,
        TileID::sandstone,   TileID::desertFossil,  TileID::copperOre,
        TileID::tinOre,      TileID::ironOre,       TileID::leadOre,
        TileID::silverOre,   TileID::tungstenOre,   TileID::goldOre,
        TileID::platinumOre, TileID::cobaltOre,     TileID::palladiumOre,
        TileID::mythrilOre,  TileID::orichalcumOre, TileID::adamantiteOre,
        TileID::titaniumOre,
    });
    int minX = world.conf.patches
                   ? 350
                   : world.desertCenter -
                         world.conf.desertSize * 0.06 * world.getWidth();
    int maxX = world.conf.patches
                   ? world.getWidth() - 350
                   : world.desertCenter +
                         world.conf.desertSize * 0.06 * world.getWidth() -
                         tomb.getWidth();
    int minY = world.getCavernLevel();
    int maxY = (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5 -
               tomb.getHeight();
    int maxFoundationEmpty = 0.4 * tomb.getWidth();
    int biomeScan = std::max(tomb.getWidth(), tomb.getHeight()) / 2;
    for (int tries = 0; tries < 8000; ++tries) {
        int x = rnd.getInt(minX, maxX);
        int y = rnd.getInt(minY, maxY);
        int numEmpty = 0;
        int numFilled = 0;
        int maxEntryFilled = tries / 250;
        if ((!world.conf.patches || isInBiome(
                                        x + biomeScan,
                                        y + biomeScan,
                                        biomeScan,
                                        Biome::desert,
                                        world)) &&
            world.regionPasses(
                x - 3,
                y + tomb.getHeight() - 9,
                4,
                4,
                [maxEntryFilled, &numFilled](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < maxEntryFilled;
                }) &&
            world.regionPasses(
                x + tomb.getWidth() - 1,
                y + tomb.getHeight() - 9,
                4,
                4,
                [maxEntryFilled, &numFilled](Tile &tile) {
                    if (tile.blockID != TileID::empty) {
                        ++numFilled;
                    }
                    return numFilled < maxEntryFilled;
                }) &&
            world.regionPasses(
                x - 10,
                y - 10,
                tomb.getWidth() + 20,
                tomb.getHeight() + 20,
                [&clearableTiles](Tile &tile) {
                    return !tile.guarded && tile.liquid != Liquid::shimmer &&
                           clearableTiles.contains(tile.blockID);
                }) &&
            world.regionPasses(
                x,
                y + tomb.getHeight() - 2,
                tomb.getWidth(),
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

void degradeTombWalls(std::vector<Point> &locations, Random &rnd, World &world)
{
    int shuffleX = rnd.getInt(0, world.getWidth());
    int shuffleY = rnd.getInt(0, world.getHeight());
    for (auto [x, y] : locations) {
        if (std::min(
                std::abs(rnd.getFineNoise(2 * x, 2 * y)),
                std::abs(
                    rnd.getFineNoise(shuffleX + 2 * x, shuffleY + 2 * y))) <
            0.09) {
            Tile &tile = world.getTile(x, y);
            tile.wallID = WallID::Unsafe::sandstone;
            tile.wallPaint = Paint::none;
        }
    }
}

void addTombTreasure(std::vector<Point> &locations, Random &rnd, World &world)
{
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    for (auto [x, y] : locations) {
        if (static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) % 9 == 0) {
            if (world.regionPasses(
                    x,
                    y,
                    2,
                    1,
                    [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
                world.regionPasses(x, y + 1, 2, 1, [](Tile &tile) {
                    return tile.blockID == TileID::sandstoneBrick ||
                           tile.blockID == TileID::sandstoneSlab;
                })) {
                world.placeFramedTile(x, y, TileID::smallPile, Variant::gold);
            }
        } else if (
            world.regionPasses(
                x,
                y,
                2,
                2,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            world.regionPasses(x, y + 2, 2, 1, [](Tile &tile) {
                return tile.blockID == TileID::sandstoneBrick ||
                       tile.blockID == TileID::sandstoneSlab;
            })) {
            world.placeFramedTile(x, y, TileID::pot, Variant::pyramid);
        }
    }
}

void genDesertTomb(Random &rnd, World &world)
{
    std::cout << "Embalming\n";
    TileBuffer tomb = Data::getBuilding(
        rnd.select({Data::Building::desertTomb1, Data::Building::desertTomb2}),
        world.getFramedTiles());
    auto [x, y] = selectTombLocation(tomb, rnd, world);
    if (x == -1) {
        return;
    }
    std::vector<Point> locations;
    for (int i = 0; i < tomb.getWidth(); ++i) {
        for (int j = 0; j < tomb.getHeight(); ++j) {
            Tile &tombTile = tomb.getTile(i, j);
            if (tombTile.blockID == TileID::empty &&
                tombTile.wallID == WallID::empty) {
                continue;
            }
            Tile &tile = world.getTile(x + i, y + j);
            if (tombTile.blockID == TileID::sandstoneSlab &&
                fnv1a32pt(x + i, y + j) % 11 == 0) {
                tombTile.blockID = TileID::sandstoneBrick;
            }
            if (tombTile.wallID == WallID::empty) {
                tombTile.wallID = tile.wallID;
            } else if (tombTile.wallPaint != Paint::none) {
                locations.emplace_back(x + i, y + j);
            }
            tile = tombTile;
            tile.guarded = true;
            if (tile.blockID == TileID::chestGroup2 && tile.frameX % 36 == 0 &&
                tile.frameY == 0) {
                fillCavernSandstoneChest(
                    world.registerStorage(x + i, y + j),
                    rnd,
                    world);
            }
        }
    }
    world.placePainting(
        x + 23,
        y + 16,
        rnd.select({Painting::lifeAboveTheSand, Painting::oasis}));
    std::erase_if(locations, [&world](Point &pt) {
        return !world.regionPasses(
            pt.first - 1,
            pt.second - 1,
            3,
            3,
            [](Tile &tile) {
                return tile.wallPaint != Paint::none ||
                       tile.wallID == WallID::empty;
            });
    });
    degradeTombWalls(locations, rnd, world);
    addTombTreasure(locations, rnd, world);
    world.queuedDeco.emplace_back(
        [x, y, tW = tomb.getWidth(), tH = tomb.getHeight()](
            Random &,
            World &world) {
            for (int i = 4; i < tW - 4; ++i) {
                for (int j = 4; j < tH - 4; ++j) {
                    world.getTile(x + i, y + j).liquid = Liquid::none;
                }
            }
        });
}
