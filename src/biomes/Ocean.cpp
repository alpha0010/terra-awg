#include "Ocean.h"

#include "Config.h"
#include "Random.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include "structures/LootRules.h"
#include "structures/StructureUtil.h"
#include <algorithm>
#include <iostream>

bool canPlaceReefChest(int x, int y, World &world)
{
    return world.regionPasses(x, y - 1, 2, 3, [](Tile &tile) {
        return tile.blockID == TileID::empty && tile.liquid == Liquid::water;
    }) && world.regionPasses(x, y + 2, 2, 1, [](Tile &tile) {
        return tile.blockID == TileID::sand ||
               tile.blockID == TileID::coralstone;
    });
}

void addGlowRocks(std::vector<Point> &locations, Random &rnd, World &world)
{
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    locations.resize(locations.size() / 150);
    for (auto [x, y] : locations) {
        double radius = rnd.getDouble(2, 3.2);
        if (!world.regionPasses(
                x - radius - 3,
                y - radius - 3,
                6 + 2 * radius,
                6 + 2 * radius,
                [](Tile &tile) { return tile.blockID == TileID::empty; })) {
            continue;
        }
        for (int i = -radius; i < radius; ++i) {
            for (int j = -radius; j < radius; ++j) {
                if (std::hypot(i, j) / radius <
                    0.6 + 0.6 * rnd.getFineNoise(x + i, y + j)) {
                    Tile &tile = world.getTile(x + i, y + j);
                    tile.blockID = TileID::stone;
                }
            }
        }
        int mossType = rnd.select(
            {TileID::kryptonMossStone,
             TileID::xenonMossStone,
             TileID::argonMossStone,
             TileID::neonMossStone});
        std::vector<Point> mossLocations;
        for (int i = -radius; i < radius; ++i) {
            for (int j = -radius; j < radius; ++j) {
                Tile &tile = world.getTile(x + i, y + j);
                if (tile.blockID == TileID::stone &&
                    world.isExposed(x + i, y + j)) {
                    tile.blockID = mossType;
                    mossLocations.emplace_back(x + i, y + j);
                }
            }
        }
        world.queuedDeco.emplace_back([mossLocations](Random &, World &world) {
            for (auto [x, y] : mossLocations) {
                growMossOn(x, y, world);
            }
        });
    }
}

void selectOceanCave(Random &rnd, World &world)
{
    int centerOpt1 = 105;
    int centerOpt2 = world.getWidth() - 105;
    world.oceanCaveCenter =
        centerOpt1 > world.jungleCenter - 0.11 * world.getWidth() - 220
            ? centerOpt2
        : centerOpt2 < world.jungleCenter + 0.11 * world.getWidth() + 220
            ? centerOpt1
            : rnd.select({centerOpt1, centerOpt2});
}

void addOceanCave(int waterTable, Random &rnd, World &world)
{
    int shuffleX = rnd.getInt(0, world.getWidth());
    int shuffleY = rnd.getInt(0, world.getHeight());
    std::vector<Point> locations;
    int centerX = world.oceanCaveCenter;
    int oceanFloor =
        scanWhileEmpty({centerX - 90, waterTable}, {0, 1}, world).y;
    int minY = scanWhileEmpty({centerX + 90, waterTable}, {0, 1}, world).y;
    if (minY > oceanFloor) {
        std::swap(minY, oceanFloor);
    }
    int maxY = std::min<int>(
        oceanFloor + world.conf.oceanCaveSize * 0.2262 * world.getHeight(),
        world.getUnderworldLevel() - 10);
    for (int x = centerX - 100; x < centerX + 100; ++x) {
        for (int y = minY; y < maxY; ++y) {
            double threshold =
                std::max(std::abs(x - centerX), y + 100 - maxY) / 25.0 - 3;
            if (rnd.getFineNoise(x, y) < threshold - 0.5 ||
                std::abs(rnd.getCoarseNoise(2 * x, 2 * y)) > 0.51) {
                continue;
            }
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::stone ||
                tile.blockID == TileID::sandstone ||
                tile.blockID == TileID::ice || tile.blockID == TileID::clay ||
                tile.blockID == TileID::silt) {
                tile.blockID = fnv1a32pt(x, y) % 7 == 0 ? TileID::sand
                                                        : TileID::coralstone;
            } else if (
                tile.blockID == TileID::dirt || tile.blockID == TileID::grass ||
                tile.blockID == TileID::mud ||
                tile.blockID == TileID::jungleGrass ||
                (tile.blockID == TileID::empty &&
                 tile.liquid == Liquid::none)) {
                tile.blockID = fnv1a32pt(x, y) % 5 == 0 ? TileID::hardenedSand
                                                        : TileID::sand;
            }
            if (std::abs(rnd.getBlurNoise(2 * x, 2 * y)) < 0.18 &&
                (rnd.getFineNoise(x, y) > std::max(threshold, -0.14) ||
                 rnd.getFineNoise(x, y) < -0.5 ||
                 rnd.getFineNoise(x + shuffleX, y + shuffleY) <
                     std::min(-threshold - 0.5, -0.4))) {
                tile.blockID = TileID::empty;
                tile.wallID = WallID::empty;
                tile.liquid = Liquid::water;
                if (y > oceanFloor) {
                    locations.emplace_back(x, y);
                }
            }
        }
    }
    addGlowRocks(locations, rnd, world);
    locations.clear();
    for (int x = centerX - 50; x < centerX + 50; ++x) {
        for (int y = world.getUndergroundLevel(); y < maxY; ++y) {
            if (canPlaceReefChest(x, y, world)) {
                locations.emplace_back(x, y);
            }
        }
    }
    std::shuffle(locations.begin(), locations.end(), rnd.getPRNG());
    world.queuedTreasures.emplace_back(
        [locs = std::move(locations)](Random &rnd, World &world) {
            int numChests = std::max<int>(locs.size() / 100, 2);
            std::vector<Point> usedLocations;
            for (auto [x, y] : locs) {
                if (canPlaceReefChest(x, y, world) &&
                    !isLocationUsed(x, y, 50, usedLocations)) {
                    usedLocations.emplace_back(x, y);
                    Chest &chest = world.placeChest(x, y, Variant::reef);
                    if (y < world.getCavernLevel()) {
                        fillWaterChest(chest, Depth::underground, rnd, world);
                    } else {
                        fillWaterChest(chest, Depth::cavern, rnd, world);
                    }
                    --numChests;
                    if (numChests <= 0) {
                        break;
                    }
                }
            }
        });
}

void setOceanSpawn(Random &rnd, World &world)
{
    std::vector<Point> opts;
    std::vector<Point> opts4;
    auto addSpawnOpt = [&opts, &opts4, &world](int x, int y) {
        if (world.regionPasses(
                x - 1,
                y - 4,
                3,
                5,
                [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
            world.regionPasses(x - 1, y + 1, 3, 1, [](Tile &tile) {
                return tile.blockID != TileID::empty;
            })) {
            opts.emplace_back(x, y);
            if (world.regionPasses(
                    x - 2,
                    y - 4,
                    1,
                    5,
                    [](Tile &tile) { return tile.blockID == TileID::empty; }) &&
                world.regionPasses(x - 2, y + 1, 1, 1, [](Tile &tile) {
                    return tile.blockID != TileID::empty;
                })) {
                opts4.emplace_back(x, y);
            }
        }
    };
    std::vector<int> centers;
    if (world.conf.celebration || world.conf.hiveQueen) {
        centers.push_back(
            world.oceanCaveCenter < 400 ? 300 : world.getWidth() - 300);
    } else {
        centers.insert(centers.end(), {300, world.getWidth() - 300});
    }
    for (int x : centers) {
        int y = world.getSurfaceLevel(x);
        for (int i = -50; i < 50; ++i) {
            for (int j = -20; j < 20; ++j) {
                addSpawnOpt(x + i, y + j);
            }
        }
    }
    if (!opts4.empty()) {
        opts = opts4;
    } else if (opts.empty()) {
        return;
    }
    std::shuffle(opts.begin(), opts.end(), rnd.getPRNG());
    world.spawn = *std::min_element(
        opts.begin(),
        opts.end(),
        [](const Point &a, const Point &b) { return a.y < b.y; });
}

void genOceans(Random &rnd, World &world)
{
    std::cout << "Filling oceans\n";
    rnd.shuffleNoise();
    int waterTable = std::max(
                         world.getSurfaceLevel(300),
                         world.getSurfaceLevel(world.getWidth() - 300)) +
                     rnd.getInt(4, 12);
    selectOceanCave(rnd, world);
    for (int x = 0; x < 390; ++x) {
        double drop = world.conf.oceanSize * 90 *
                      (1 - 1 / (1 + std::exp(0.041 * (200 - x))));
        double sandDepth = (40 + 9 * rnd.getCoarseNoise(x, 0)) *
                           std::min(1.0, (400.0 - x) / 160);
        auto fillColumn = [&](int effectiveX) {
            int sandTile = TileID::sand;
            int shellPileTile = TileID::shellPile;
            int siltTile = TileID::silt;
            if (world.conf.hiveQueen &&
                ((world.oceanCaveCenter < 400) != (effectiveX < 400))) {
                sandTile = TileID::honey;
                shellPileTile = TileID::hive;
                siltTile = TileID::honey;
            }
            for (int y = world.getSurfaceLevel(effectiveX) - 10;
                 y < world.getUndergroundLevel();
                 ++y) {
                if (world.getTile(effectiveX, y).blockID != TileID::empty &&
                    (y > waterTable ||
                     world.getTile(effectiveX, y).blockID != TileID::hive)) {
                    for (int i = 0; i < drop; ++i) {
                        Tile &tile = world.getTile(effectiveX, y + i);
                        tile.wallID = WallID::empty;
                        tile.blockID = TileID::empty;
                        if (y + i > waterTable) {
                            tile.liquid = Liquid::water;
                        }
                    }
                    for (int i = drop + 1; i < drop + sandDepth; ++i) {
                        Tile &tile = world.getTile(effectiveX, y + i);
                        if (tile.wallID == WallID::Unsafe::dirt) {
                            tile.wallID = WallID::Unsafe::wornStone;
                        } else {
                            tile.wallID = WallID::empty;
                        }
                        if (tile.blockID == TileID::mud ||
                            tile.blockID == TileID::jungleGrass) {
                            tile.blockID = siltTile;
                        } else if (y + i > waterTable + 20) {
                            if (tile.blockID == TileID::stone ||
                                tile.blockID == TileID::sandstone ||
                                tile.blockID == TileID::ice) {
                                tile.blockID = TileID::coralstone;
                            } else {
                                tile.blockID = sandTile;
                            }
                        } else if (
                            tile.blockID != TileID::dirt &&
                            tile.blockID != TileID::grass &&
                            tile.blockID != TileID::sand &&
                            tile.blockID != TileID::snow) {
                            tile.blockID = shellPileTile;
                        } else {
                            tile.blockID = sandTile;
                        }
                    }
                    break;
                }
            }
        };
        fillColumn(x);
        fillColumn(world.getWidth() - x - 1);
    }
    addOceanCave(waterTable, rnd, world);
    if (world.conf.spawn == SpawnPoint::ocean) {
        setOceanSpawn(rnd, world);
    }
}
