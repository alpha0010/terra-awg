#include "biomes/patches/Base.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/Base.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>

BiomeData computeBiomeData(int x, int y, Random &rnd)
{
    double snow = std::clamp(
        std::min(
            -10.31 * rnd.getTemperature(x, y) - 4.712,
            6.67 * rnd.getHumidity(x, y) + 4.67),
        0.0,
        1.0);
    double desert = std::clamp(
        std::min(
            -10.31 * rnd.getHumidity(x, y) - 4.712,
            6.67 * rnd.getTemperature(x, y) + 4.67),
        0.0,
        1.0);
    double jungle = std::clamp(
        14.706 * std::min(rnd.getTemperature(x, y), rnd.getHumidity(x, y)) -
            0.206,
        0.0,
        1.0);
    double underworld =
        std::clamp(14.286 * rnd.getTemperature(x, y) - 14.5, 0.0, 1.0);
    if (underworld > 0.5) {
        double mult = 2 - 2 * underworld;
        snow *= mult;
        desert *= mult;
        jungle *= mult;
    }
    double total = snow + desert + jungle + underworld;
    double forest = std::clamp(1 - total, 0.0, 1.0);
    total += forest;
    total = 1 / total;
    snow *= total;
    desert *= total;
    jungle *= total;
    forest *= total;
    underworld *= total;

    std::array biomes = std::to_array({
        std::pair{forest, Biome::forest},
        {desert, Biome::desert},
        {jungle, Biome::jungle},
        {snow, Biome::snow},
        {underworld, Biome::underworld},
    });
    std::vector<std::pair<double, Biome>> activeBiomes;
    if (fnv1a32pt(x, y) % 37 > 4) {
        std::vector<std::pair<double, Biome>> looseActiveBiomes;
        for (auto [prob, biome] : biomes) {
            if (prob > 0.02) {
                activeBiomes.emplace_back(prob, biome);
            }
            if (prob > 0.1) {
                looseActiveBiomes.emplace_back(prob, biome);
            }
        }
        if (looseActiveBiomes.size() == 2) {
            activeBiomes = looseActiveBiomes;
        }
    }
    Biome active = Biome::forest;
    if (activeBiomes.size() == 2) {
        int offset =
            static_cast<int>(99999 * (1 + rnd.getFineNoise(0, 0))) % 997;
        active = activeBiomes[0].first +
                             0.6 * rnd.getFineNoise(x + offset, y + offset) >
                         0.5
                     ? activeBiomes[0].second
                     : activeBiomes[1].second;
    } else {
        int quantFactor = 1400;
        int target = fnv1a32pt(x, y) % (quantFactor - 1);
        int accu = 0;
        for (auto [prob, biome] : biomes) {
            accu += prob * quantFactor;
            if (accu > target) {
                active = biome;
                break;
            }
        }
    }
    return {active, forest, snow, desert, jungle, underworld};
}

inline std::array const snowTiles{
    TileID::snow,
    TileID::snow,
    TileID::ice,
    TileID::stone,
    TileID::thinIce,
    TileID::slush};
inline std::array const desertTiles{
    TileID::sand,
    TileID::sand,
    TileID::sandstone,
    TileID::desertFossil,
    TileID::desertFossil,
    TileID::desertFossil};
inline std::array const jungleTiles{
    TileID::mud,
    TileID::jungleGrass,
    TileID::mud,
    TileID::clay,
    TileID::silt,
    TileID::stone};
inline std::array const forestTiles{
    TileID::dirt,
    TileID::grass,
    TileID::stone,
    TileID::clay,
    TileID::sand,
    TileID::mud};
inline std::array const underworldTiles{
    TileID::ash,
    TileID::ash,
    TileID::ash,
    TileID::ash,
    TileID::ash,
    TileID::ash};

int getWallVarIndex(
    int x,
    int y,
    const std::vector<std::pair<int, int>> &wallVarNoise,
    Random &rnd)
{
    for (size_t idx = 0; idx < wallVarNoise.size(); ++idx) {
        auto [i, j] = wallVarNoise[idx];
        if (std::abs(rnd.getCoarseNoise(x + i, y + j)) < 0.07) {
            return idx;
        }
    }
    return -1;
}

void identifySurfaceBiomes(World &world)
{
    double maxDesert = 0;
    double maxJungle = 0;
    double maxSnow = 0;
    double curDesert = 0;
    double curJungle = 0;
    double curSnow = 0;
    int desertCenter = 0;
    int jungleCenter = 0;
    int snowCenter = 0;
    auto scanColumn = [&](int x, bool apply) {
        curDesert *= 0.99;
        curJungle *= 0.99;
        curSnow *= 0.99;
        int surface = world.getSurfaceLevel(x);
        for (int y = surface; y < surface + 30; ++y) {
            const BiomeData &biome = world.getBiome(x, y);
            curDesert += 0.01 * biome.desert;
            curJungle += 0.01 * biome.jungle;
            curSnow += 0.01 * biome.snow;
        }
        if (!apply) {
            return;
        }
        if (curDesert > maxDesert) {
            maxDesert = curDesert;
            desertCenter = x;
        }
        if (curJungle > maxJungle) {
            maxJungle = curJungle;
            jungleCenter = x;
        }
        if (curSnow > maxSnow) {
            maxSnow = curSnow;
            snowCenter = x;
        }
    };
    for (int x = 50; x < world.getWidth() - 200; ++x) {
        scanColumn(x, x > 350);
    }
    world.desertCenter = desertCenter;
    world.jungleCenter = jungleCenter;
    world.snowCenter = snowCenter;

    maxDesert = 0;
    maxJungle = 0;
    maxSnow = 0;
    curDesert = 0;
    curJungle = 0;
    curSnow = 0;
    for (int x = world.getWidth() - 50; x > 200; --x) {
        scanColumn(x, x < world.getWidth() - 350);
    }

    auto mergeResults =
        [threshold =
             std::min<int>(0.22 * world.getWidth(), 1200)](double a, double b) {
            return std::abs(a - b) < threshold ? std::midpoint(a, b)
                                               : std::max(a - 150, 0.0);
        };
    world.desertCenter = mergeResults(world.desertCenter, desertCenter);
    world.jungleCenter = mergeResults(world.jungleCenter, jungleCenter);
    world.snowCenter = mergeResults(world.snowCenter, snowCenter);
}

void genWorldBasePatches(Random &rnd, World &world)
{
    std::cout << "Generating base terrain\n";
    rnd.shuffleNoise();
    rnd.saveShuffleState();
    std::vector<std::pair<int, int>> depositNoise;
    for (int iter = 0; iter < 7; ++iter) {
        depositNoise.emplace_back(
            rnd.getInt(0, world.getWidth()),
            rnd.getInt(0, world.getHeight()));
    }
    std::vector<std::pair<int, int>> wallVarNoise;
    for (size_t iter = 0; iter < WallVariants::dirt.size(); ++iter) {
        wallVarNoise.emplace_back(
            rnd.getInt(0, world.getWidth()),
            rnd.getInt(0, world.getHeight()));
    }
    computeSurfaceLevel(rnd, world);
    world.spawnY = world.getSurfaceLevel(world.getWidth() / 2) - 1;
    world.initBiomeData();
    double oreThreshold = computeOreThreshold(world.conf.ore);
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [oreThreshold, &depositNoise, &wallVarNoise, &rnd, &world](int x) {
            bool nearEdge = x < 350 || x > world.getWidth() - 350;
            for (int y = 0; y < world.getHeight(); ++y) {
                BiomeData biome = computeBiomeData(x, y, rnd);
                world.getBiome(x, y) = biome;
                if (y < world.getSurfaceLevel(x)) {
                    continue;
                }
                double threshold =
                    y < world.getUndergroundLevel()
                        ? 3.0 * y / world.getUndergroundLevel() - 3
                        : static_cast<double>(y - world.getUndergroundLevel()) /
                              (world.getHeight() - world.getUndergroundLevel());
                int tileType = rnd.getFineNoise(
                                   x + depositNoise[0].first,
                                   y + depositNoise[1].second) > threshold
                                   ? 0
                                   : 2;
                for (int iter = 0; iter < 3; ++iter) {
                    if (rnd.getFineNoise(
                            x + depositNoise[iter].first,
                            y + depositNoise[iter].second) > 0.7) {
                        tileType = iter + 3;
                        break;
                    }
                }
                if (tileType == 0 && y < world.getUndergroundLevel() &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            100 ==
                        0) {
                    tileType = 1;
                }
                Tile &tile = world.getTile(x, y);
                switch (biome.active) {
                case Biome::snow: {
                    tile.blockID = snowTiles[tileType];
                    int index = getWallVarIndex(x, y, wallVarNoise, rnd);
                    if (index != -1 &&
                        fnv1a32pt(index, wallVarNoise[0].second) % 5 == 0) {
                        tile.wallID = WallVariants::stone
                            [wallVarNoise[0].first %
                             WallVariants::stone.size()];
                    } else if (index != -1 || y < world.getUndergroundLevel()) {
                        tile.wallID = tile.blockID == TileID::snow
                                          ? WallID::Unsafe::snow
                                          : WallID::Unsafe::ice;
                    }
                    break;
                }
                case Biome::desert:
                    tile.blockID = desertTiles[tileType];
                    if (y > world.getCavernLevel() &&
                        tile.blockID == TileID::sandstone) {
                        if (std::abs(
                                rnd.getCoarseNoise(
                                    x + depositNoise[2].first,
                                    y + depositNoise[3].second) +
                                0.23) < 0.04) {
                            tile.blockID = TileID::sand;
                        } else if (
                            std::abs(
                                rnd.getCoarseNoise(
                                    x + depositNoise[2].first,
                                    y + depositNoise[3].second) -
                                0.23) < 0.04) {
                            tile.blockID = TileID::hardenedSand;
                        }
                    }
                    tile.wallID = tile.blockID == TileID::sandstone
                                      ? WallID::Unsafe::sandstone
                                      : WallID::Unsafe::hardenedSand;
                    break;
                case Biome::jungle:
                    tile.blockID = jungleTiles[tileType];
                    if (y < world.getUndergroundLevel()) {
                        tile.wallID = WallID::Unsafe::mud;
                    } else {
                        int index = getWallVarIndex(x, y, wallVarNoise, rnd);
                        if (index != -1) {
                            tile.wallID = WallVariants::jungle
                                [fnv1a32pt(index, wallVarNoise[1].first) %
                                 WallVariants::jungle.size()];
                        }
                    }
                    break;
                case Biome::forest: {
                    tile.blockID = forestTiles[tileType];
                    int index = getWallVarIndex(x, y, wallVarNoise, rnd);
                    if (index != -1) {
                        tile.wallID = WallVariants::dirt[index];
                    } else if (y < world.getUndergroundLevel()) {
                        tile.wallID = tile.blockID == TileID::stone
                                          ? WallID::Unsafe::rockyDirt
                                          : WallID::Unsafe::dirt;
                    }
                    break;
                }
                case Biome::underworld: {
                    tile.blockID = underworldTiles[tileType];
                    int index = getWallVarIndex(x, y, wallVarNoise, rnd);
                    if (index != -1) {
                        tile.wallID = WallVariants::underworld
                            [fnv1a32pt(index, wallVarNoise[2].first) %
                             WallVariants::underworld.size()];
                    }
                    break;
                }
                }
                threshold = 0.4 + 0.03 * rnd.getFineNoise(x, y);
                if ((biome.desert > threshold && biome.snow > threshold) ||
                    (biome.snow > 0.57 && biome.active == Biome::desert) ||
                    (biome.desert > 0.57 && biome.active == Biome::snow)) {
                    tile.blockID = TileID::marble;
                    tile.wallID = WallID::Unsafe::marble;
                }
                for (auto [idx, oreRoof, oreFloor, ore] :
                     {std::tuple{
                          3,
                          0.6 * world.getUndergroundLevel(),
                          (world.getUndergroundLevel() +
                           world.getCavernLevel()) /
                              2,
                          world.copperVariant},
                      {4,
                       0.85 * world.getUndergroundLevel(),
                       (2 * world.getCavernLevel() +
                        world.getUnderworldLevel()) /
                           3,
                       world.ironVariant},
                      {5,
                       (world.getUndergroundLevel() + world.getCavernLevel()) /
                           2,
                       (world.getCavernLevel() + world.getUnderworldLevel()) /
                           2,
                       world.silverVariant},
                      {6,
                       (2 * world.getCavernLevel() +
                        world.getUnderworldLevel()) /
                           3,
                       world.getUnderworldLevel(),
                       world.goldVariant}}) {
                    if (y > oreRoof && y < oreFloor &&
                        rnd.getFineNoise(
                            x + depositNoise[idx].first,
                            y + depositNoise[idx].second) < oreThreshold) {
                        tile.blockID = ore;
                        break;
                    }
                }
                if (nearEdge && y < 0.9 * world.getUndergroundLevel()) {
                    continue;
                }
                threshold =
                    y < world.getUndergroundLevel()
                        ? 2.94 - 3.1 * y / world.getUndergroundLevel()
                    : y > world.getUnderworldLevel()
                        ? 3.1 * (y - world.getUnderworldLevel()) / 230 - 0.16
                        : -0.16;
                if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                    rnd.getFineNoise(x, y) > threshold) {
                    tile.blockID = TileID::empty;
                }
                threshold =
                    y > world.getUnderworldLevel()
                        ? (world.getUnderworldLevel() - y) / 10.0
                        : static_cast<double>(y - world.getUndergroundLevel()) /
                                  (world.getUnderworldLevel() -
                                   world.getUndergroundLevel()) -
                              1;
                if (std::abs(rnd.getCoarseNoise(x, 2 * y)) > 0.55 &&
                    rnd.getFineNoise(x, y) < threshold + 0.1) {
                    Tile &tile = world.getTile(x, y);
                    tile.blockID = TileID::empty;
                }
                if (y > world.getCavernLevel() && biome.jungle > 0.01) {
                    threshold =
                        2.0 * (y - world.getCavernLevel()) *
                            (y - world.getHeight()) /
                            std::pow(
                                world.getHeight() - world.getCavernLevel(),
                                2) +
                        0.75;
                    threshold = std::lerp(1.0, threshold, biome.jungle);
                    if (y > world.getCavernLevel() &&
                        rnd.getCoarseNoise(2 * x, 2 * y) > threshold) {
                        tile.blockID = TileID::empty;
                    }
                }
                if (biome.snow > 0.01) {
                    threshold = std::max(
                        -0.1,
                        1 + 15.0 * (world.getCavernLevel() - y) /
                                world.getHeight());
                    threshold = std::lerp(1.0, threshold, biome.snow);
                    if (std::abs(rnd.getCoarseNoise(2 * x, y) + 0.1) < 0.12 &&
                        rnd.getFineNoise(x, y) > threshold) {
                        tile.blockID = tile.blockID == TileID::snow
                                           ? TileID::thinIce
                                           : TileID::empty;
                    }
                }
                if (tile.blockID == TileID::empty &&
                    y < world.getUndergroundLevel()) {
                    if (biome.active == Biome::jungle) {
                        int index = getWallVarIndex(x, y, wallVarNoise, rnd);
                        if (fnv1a32pt(index, wallVarNoise[3].second) % 3 == 0) {
                            tile.wallID = WallVariants::stone
                                [fnv1a32pt(index, wallVarNoise[3].first) %
                                 WallVariants::stone.size()];
                        } else {
                            tile.wallID = WallID::Unsafe::jungle;
                        }
                    } else if (
                        biome.active == Biome::forest &&
                        getWallVarIndex(x, y, wallVarNoise, rnd) < 1) {
                        tile.wallID = rnd.getFineNoise(
                                          x + wallVarNoise[4].first,
                                          y + wallVarNoise[4].second) > 0
                                          ? WallID::Unsafe::grass
                                          : WallID::Unsafe::flower;
                    }
                }
            }
        });

    parallelFor(
        std::views::iota(0, world.getWidth()),
        [underworldHeight = world.getHeight() - world.getUnderworldLevel(),
         hellstoneThreshold = -computeOreThreshold(4.24492 * world.conf.ore),
         &rnd,
         &world](int x) {
            int underworldRoof =
                world.getUnderworldLevel() + 0.22 * underworldHeight +
                19 * rnd.getCoarseNoise(x, 0.33 * world.getHeight());
            int underworldFloor =
                world.getUnderworldLevel() + 0.42 * underworldHeight +
                35 * rnd.getCoarseNoise(x, 0.66 * world.getHeight());
            for (int y =
                     world.getUnderworldLevel() + 20 * rnd.getCoarseNoise(x, 0);
                 y < world.getHeight();
                 ++y) {
                Tile &tile = world.getTile(x, y);
                if (y > underworldFloor) {
                    if (std::abs(rnd.getFineNoise(x, y)) > hellstoneThreshold) {
                        tile.blockID = TileID::hellstone;
                    }
                } else if (y > underworldRoof && tile.blockID == TileID::ash) {
                    tile.blockID = TileID::empty;
                }
            }
        });

    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        int stalactiteLen = 0;
        int stalacIter = 0;
        for (int y = 0; y < world.getHeight(); ++y) {
            if (y % 500 == 0) {
                stalacIter = y / 35;
            }
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::empty) {
                if (stalactiteLen > 0 &&
                    world.getTile(x, y + 1).blockID == TileID::empty) {
                    tile.blockID = TileID::marble;
                    if (tile.wallID != WallID::Unsafe::marble) {
                        stalactiteLen /= 2;
                    }
                    --stalactiteLen;
                }
                continue;
            }
            if (!world.isExposed(x, y)) {
                continue;
            }
            if (world.isIsolated(x, y)) {
                tile.blockID = TileID::empty;
                continue;
            }
            if (tile.blockID == TileID::marble &&
                world.getTile(x, y + 1).blockID == TileID::empty) {
                stalactiteLen = std::max(
                    0.0,
                    16 * rnd.getFineNoise(4 * x, 100 * stalacIter));
                ++stalacIter;
            }
            BiomeData &biome = world.getBiome(x, y);
            if (biome.active == Biome::forest &&
                y < world.getUndergroundLevel()) {
                if (tile.blockID == TileID::dirt) {
                    tile.blockID = TileID::grass;
                }
            } else if (biome.active == Biome::jungle) {
                if (tile.blockID == TileID::mud) {
                    tile.blockID = TileID::jungleGrass;
                }
            } else if (
                biome.active == Biome::underworld &&
                y < world.getSurfaceLevel(x) + 10 &&
                std::abs(x - world.getWidth() / 2) < 100) {
                if (tile.blockID == TileID::ash) {
                    tile.blockID = TileID::ashGrass;
                }
            }
        }
    });
    identifySurfaceBiomes(world);
}
