#include "biomes/patches/Base.h"

#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/Base.h"
#include <algorithm>
#include <iostream>

enum class Biome { forest, snow, desert, jungle, underworld };

struct BiomeData {
    Biome active;
    double forest;
    double snow;
    double desert;
    double jungle;
    double underworld;
};

BiomeData computeBiomeData(int x, int y, Random &rnd)
{
    double snow = std::clamp(
        std::min(
            -10.31 * rnd.getTemperature(x, y) - 3.917,
            6.67 * rnd.getHumidity(x, y) + 4.67),
        0.0,
        1.0);
    double desert = std::clamp(
        std::min(
            -10.31 * rnd.getHumidity(x, y) - 3.917,
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
    if (underworld > 0.99) {
        snow = desert = jungle = 0;
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
    int quantFactor = 1400;
    int target = fnv1a32pt(x, y) % (quantFactor - 1);
    int accu = 0;
    Biome active = Biome::forest;
    for (auto [prob, biome] :
         {std::pair{snow, Biome::snow},
          {desert, Biome::desert},
          {jungle, Biome::jungle},
          {underworld, Biome::underworld},
          {forest, Biome::forest}}) {
        accu += prob * quantFactor;
        if (accu > target) {
            active = biome;
            break;
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

void genWorldBasePatches(Random &rnd, World &world)
{
    std::cout << "Generating base terrain\n";
    std::vector<std::pair<int, int>> depositNoise;
    for (int iter = 0; iter < 7; ++iter) {
        depositNoise.emplace_back(
            rnd.getInt(0, world.getWidth()),
            rnd.getInt(0, world.getHeight()));
    }
    computeSurfaceLevel(rnd, world);
    std::vector<BiomeData> biomeMap(world.getWidth() * world.getHeight());
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [&depositNoise, &biomeMap, &rnd, &world](int x) {
            bool nearEdge = x < 350 || x > world.getWidth() - 350;
            for (int y = 0; y < world.getHeight(); ++y) {
                BiomeData biome = computeBiomeData(x, y, rnd);
                biomeMap[x * world.getHeight() + y] = biome;
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
                case Biome::snow:
                    tile.blockID = snowTiles[tileType];
                    break;
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
                    break;
                case Biome::jungle:
                    tile.blockID = jungleTiles[tileType];
                    break;
                case Biome::forest:
                    tile.blockID = forestTiles[tileType];
                    break;
                case Biome::underworld:
                    tile.blockID = underworldTiles[tileType];
                    break;
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
                            y + depositNoise[idx].second) < -0.645) {
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
            }
        });
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [&biomeMap, &world](int x) {
            for (int y = 0; y < world.getHeight(); ++y) {
                Tile &tile = world.getTile(x, y);
                if (tile.blockID == TileID::empty || !world.isExposed(x, y)) {
                    continue;
                }
                if (world.isIsolated(x, y)) {
                    tile.blockID = TileID::empty;
                    continue;
                }
                BiomeData &biome = biomeMap[x * world.getHeight() + y];
                if (biome.active == Biome::forest &&
                    y < world.getUndergroundLevel()) {
                    if (tile.blockID == TileID::dirt) {
                        tile.blockID = TileID::grass;
                    }
                } else if (biome.active == Biome::jungle) {
                    if (tile.blockID == TileID::mud) {
                        tile.blockID = TileID::jungleGrass;
                    }
                }
            }
        });
}
