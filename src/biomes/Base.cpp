#include "Base.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/BiomeUtil.h"
#include "ids/WallID.h"
#include <algorithm>
#include <iostream>

void computeSurfaceLevel(Random &rnd, World &world)
{
    double surfaceLevel = rnd.getDouble(
        0.7 * world.getUndergroundLevel(),
        0.8 * world.getUndergroundLevel());
    int center = world.getWidth() / 2;
    int delta = 0;
    int deltaLen = 1;
    int prevY = surfaceLevel;
    // Keep surface terrain mostly level near spawn and oceans.
    for (int x = 0; x < world.getWidth(); ++x) {
        double drop =
            world.conf.sunken && !world.conf.shattered
                ? 120 * (1 / (1 + std::exp(0.057 * (180 + center - x))) +
                         1 / (1 + std::exp(0.057 * (180 + x - center)))) -
                      90
                : 0;
        int curY = surfaceLevel + drop +
                   std::min(
                       {0.1 * std::abs(center - x) + 15,
                        0.08 * std::min(x, world.getWidth() - x) + 5,
                        world.conf.surfaceAmplitude * 50.0}) *
                       rnd.getCoarseNoise(x, 0);
        world.getSurfaceLevel(x) = curY;
        if (delta == curY - prevY) {
            ++deltaLen;
        } else {
            if (deltaLen > 4 && (delta == 1 || delta == -1)) {
                // Break up boring slopes.
                for (int i = 0; i < deltaLen; ++i) {
                    world.getSurfaceLevel(x - i) +=
                        9 * (0.5 - std::abs(i - 0.5 * deltaLen) / deltaLen) *
                        rnd.getFineNoise(x - 2 * i, 0);
                }
            }
            delta = curY - prevY;
            deltaLen = 1;
        }
        prevY = curY;
    }
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

void registerSnow(Random &rnd, World &world)
{
    std::cout << "Freezing land\n";
    rnd.shuffleNoise();
    double center = world.snowCenter;
    double scanDist = world.conf.snowSize * 0.08 * world.getWidth();
    double snowFloor =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    parallelFor(
        std::views::iota(
            std::max<int>(center - scanDist, 0),
            std::min<int>(center + scanDist, world.getWidth())),
        [center, snowFloor, &rnd, &world](int x) {
            for (int y = 0; y < world.getUnderworldLevel(); ++y) {
                double threshold = std::max(
                    std::abs(x - center) / 100.0 -
                        (world.conf.snowSize * world.getWidth() / 1700.0),
                    15 * (y - snowFloor) / world.getHeight());
                BiomeData &biome = world.getBiome(x, y);
                biome.snow = std::clamp(0.45 - 0.75 * threshold, 0.0, 1.0);
                if (rnd.getCoarseNoise(x, y) > threshold) {
                    biome.active = Biome::snow;
                }
            }
        });
}

void registerDesert(Random &rnd, World &world)
{
    std::cout << "Desertification\n";
    rnd.shuffleNoise();
    double center = world.desertCenter;
    double scanDist = world.conf.desertSize * 0.08 * world.getWidth();
    double desertFloor =
        (world.getCavernLevel() + 4 * world.getUnderworldLevel()) / 5;
    parallelFor(
        std::views::iota(
            std::max<int>(center - scanDist, 0),
            std::min<int>(center + scanDist, world.getWidth())),
        [center, desertFloor, &rnd, &world](int x) {
            for (int y = 0; y < world.getUnderworldLevel(); ++y) {
                double threshold = std::max(
                    std::abs(x - center) / 100.0 -
                        (world.conf.desertSize * world.getWidth() / 1700.0),
                    15 * (y - desertFloor) / world.getHeight());
                BiomeData &biome = world.getBiome(x, y);
                biome.desert = std::clamp(0.45 - 0.75 * threshold, 0.0, 1.0);
                if (rnd.getCoarseNoise(x, y) > threshold) {
                    biome.active = Biome::desert;
                }
            }
        });
}

void registerJungle(Random &rnd, World &world)
{
    rnd.shuffleNoise();
    double center = world.jungleCenter;
    double scanDist = world.conf.jungleSize * 0.11 * world.getWidth();
    parallelFor(
        std::views::iota(
            std::max<int>(center - scanDist, 0),
            std::min<int>(center + scanDist, world.getWidth())),
        [center, &rnd, &world](int x) {
            double threshold =
                std::abs(x - center) / 100.0 -
                (world.conf.jungleSize * world.getWidth() / 1050.0);
            for (int y = 0; y < world.getHeight(); ++y) {
                BiomeData &biome = world.getBiome(x, y);
                biome.jungle = std::clamp(0.25 - 0.45 * threshold, 0.0, 1.0);
                if (rnd.getCoarseNoise(x, y) > threshold &&
                    rnd.getFineNoise(x, y) > std::abs(x - center) / 260.0 -
                                                 (world.conf.jungleSize *
                                                  world.getWidth() / 2700.0)) {
                    biome.active = Biome::jungle;
                }
            }
        });
}

void registerUnderworld(Random &rnd, World &world)
{
    rnd.shuffleNoise();
    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        int underworldLevel =
            world.getUnderworldLevel() + 20 * rnd.getCoarseNoise(x, 0);
        for (int y = 0; y < underworldLevel; ++y) {
            BiomeData &biome = world.getBiome(x, y);
            biome.forest = std::clamp(
                1 - biome.snow - biome.desert - biome.jungle,
                0.0,
                1.0);
        }
        for (int y = underworldLevel; y < world.getHeight(); ++y) {
            world.getBiome(x, y) = {Biome::underworld, 0.0, 0.0, 0.0, 0.0, 1.0};
        }
    });
}

double computeStoneThreshold(int y, World &world)
{
    std::array<std::pair<int, double>, 5> steps{{
        {0, -3},
        {world.getUndergroundLevel(), 0},
        {std::midpoint(world.getUndergroundLevel(), world.getCavernLevel()),
         world.conf.dontDigUp ? 0.8225 : 0.0918},
        {world.getCavernLevel(), world.conf.dontDigUp ? 0.1127 : 0.1836},
        {world.getUnderworldLevel(), world.conf.dontDigUp ? -0.6413 : 0.8225},
    }};
    std::pair<int, double> from;
    std::pair<int, double> to;
    for (auto step : steps) {
        from = to;
        to = step;
        if (y < to.first) {
            break;
        }
    }
    return std::lerp(
        from.second,
        to.second,
        static_cast<double>(y - from.first) / (to.first - from.first));
}

std::vector<std::tuple<int, int, int, int>> getOreLayers(World &world)
{
    if (world.conf.dontDigUp) {
        return {
            {3,
             (3 * world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 5,
             world.getUnderworldLevel(),
             world.copperVariant},
            {4,
             (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
             (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3,
             world.ironVariant},
            {5,
             (4 * world.getUndergroundLevel() + world.getCavernLevel()) / 5,
             (4 * world.getCavernLevel() + world.getUnderworldLevel()) / 5,
             world.silverVariant},
            {6,
             0.6 * world.getUndergroundLevel(),
             (3 * world.getUndergroundLevel() + 5 * world.getCavernLevel()) / 8,
             world.goldVariant}};
    }
    return {
        {3,
         0.6 * world.getUndergroundLevel(),
         (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
         world.copperVariant},
        {4,
         0.85 * world.getUndergroundLevel(),
         (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
         world.ironVariant},
        {5,
         (world.getUndergroundLevel() + world.getCavernLevel()) / 2,
         (world.getCavernLevel() + world.getUnderworldLevel()) / 2,
         world.silverVariant},
        {6,
         (2 * world.getCavernLevel() + world.getUnderworldLevel()) / 3,
         world.getUnderworldLevel(),
         world.goldVariant}};
}

void applyBaseTerrain(Random &rnd, World &world)
{
    rnd.shuffleNoise();
    // Save so later generators can match cave structures.
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
    world.spawn = {
        world.getWidth() / 2,
        world.getSurfaceLevel(world.getWidth() / 2) - 1};
    double oreThreshold = computeOreThreshold(world.conf.ore);
    double goldThreshold = world.conf.forTheWorthy
                               ? computeOreThreshold(1.35 * world.conf.ore)
                               : oreThreshold;
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [oreThreshold,
         goldThreshold,
         underworldHeight = world.getHeight() - world.getUnderworldLevel(),
         hellstoneThreshold = -computeOreThreshold(4.24492 * world.conf.ore),
         &depositNoise,
         &wallVarNoise,
         &rnd,
         &world](int x) {
            bool nearEdge = x < 350 || x > world.getWidth() - 350;
            int underworldRoof =
                world.getUnderworldLevel() + 0.22 * underworldHeight +
                19 * rnd.getCoarseNoise(x, 0.33 * world.getHeight());
            int underworldFloor =
                world.getUnderworldLevel() + 0.42 * underworldHeight +
                35 * rnd.getCoarseNoise(x, 0.66 * world.getHeight());
            for (int y = world.getSurfaceLevel(x); y < world.getHeight(); ++y) {
                BiomeData &biome = world.getBiome(x, y);
                double threshold = computeStoneThreshold(y, world);
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
                    if (y > (world.conf.dontDigUp ? world.getUndergroundLevel()
                                                  : world.getCavernLevel()) &&
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
                    if (y > underworldFloor) {
                        tile.blockID = std::abs(rnd.getFineNoise(x, y)) >
                                               hellstoneThreshold
                                           ? TileID::hellstone
                                           : TileID::ash;
                    } else if (y < underworldRoof) {
                        tile.blockID = TileID::ash;
                    }
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
                for (auto [idx, oreRoof, oreFloor, ore] : getOreLayers(world)) {
                    if (y > oreRoof && y < oreFloor &&
                        rnd.getFineNoise(
                            x + depositNoise[idx].first,
                            y + depositNoise[idx].second) <
                            (ore == world.goldVariant ? goldThreshold
                                                      : oreThreshold)) {
                        tile.blockID = ore;
                        break;
                    }
                }
                if (nearEdge && y < 0.9 * world.getUndergroundLevel()) {
                    continue;
                }
                threshold = y < world.getUndergroundLevel()
                                ? 2.94 - 3.1 * y / world.getUndergroundLevel()
                            : y > world.getUnderworldLevel()
                                ? 3.1 * (y - world.getUnderworldLevel()) /
                                          underworldHeight -
                                      0.16
                                : -0.16;
                if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                    rnd.getFineNoise(x, y) > threshold) {
                    // Strings of nearly connected caves, with horizontal bias.
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
                    // Increasingly large isolated deep caves.
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
                        1 + 15.0 *
                                ((world.conf.dontDigUp
                                      ? world.getUndergroundLevel()
                                      : world.getCavernLevel()) -
                                 y) /
                                world.getHeight());
                    threshold = std::lerp(1.0, threshold, biome.snow);
                    if (std::abs(rnd.getCoarseNoise(2 * x, y) + 0.1) < 0.12 &&
                        rnd.getFineNoise(x, y) > threshold) {
                        tile.blockID = tile.blockID == TileID::snow
                                           ? TileID::thinIce
                                           : TileID::empty;
                    }
                }
                if (biome.desert > 0.01) {
                    threshold = std::max(
                        1.2 + 3.0 * (world.getUndergroundLevel() - y) /
                                  world.getHeight(),
                        0.4);
                    threshold = std::lerp(1.0, threshold, biome.desert);
                    if (std::abs(rnd.getBlurNoise(x, 5 * y)) > threshold &&
                        rnd.getFineNoise(
                            x + depositNoise[4].first,
                            y + depositNoise[5].second) > -0.3 &&
                        (tile.blockID == TileID::sandstone ||
                         ((tile.blockID == TileID::sand ||
                           tile.blockID == TileID::hardenedSand) &&
                          rnd.getFineNoise(
                              x + depositNoise[2].first,
                              y + depositNoise[3].second) > 0))) {
                        tile.blockID = TileID::empty;
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

    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        int stalactiteLen = 0;
        int stalacIter = 0;
        int cavernGrassLevel =
            world.conf.dontDigUp
                ? 25 * rnd.getCoarseNoise(x, world.getCavernLevel()) +
                      world.getCavernLevel()
                : world.getHeight();
        for (int y = 0; y < world.getHeight(); ++y) {
            if (y % 500 == 0) {
                stalacIter = y / 35;
            }
            Tile &tile = world.getTile(x, y);
            if (tile.blockID == TileID::empty) {
                if (y > cavernGrassLevel &&
                    world.getBiome(x, y).active != Biome::underworld &&
                    rnd.getCoarseNoise(x, y) > 0) {
                    tile.wallID = WallID::empty;
                }
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
                (y < world.getUndergroundLevel() || y > cavernGrassLevel)) {
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
}

void genWorldBase(Random &rnd, World &world)
{
    registerSnow(rnd, world);
    registerDesert(rnd, world);
    registerJungle(rnd, world);
    registerUnderworld(rnd, world);

    std::cout << "Generating base terrain\n";
    applyBaseTerrain(rnd, world);
}
