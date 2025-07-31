#include "biomes/hiveQueen/Base.h"

#include "Config.h"
#include "Random.h"
#include "Util.h"
#include "World.h"
#include "biomes/Base.h"
#include "biomes/BiomeUtil.h"
#include "biomes/patches/Base.h"
#include "ids/WallID.h"
#include "structures/StructureUtil.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <unordered_set>

inline std::array const snowTiles{
    TileID::snow,
    TileID::ice,
    TileID::stone,
    TileID::thinIce,
    TileID::slush};
inline std::array const desertTiles{
    TileID::sand,
    TileID::sandstone,
    TileID::desertFossil,
    TileID::desertFossil,
    TileID::desertFossil};
inline std::array const jungleTiles{
    TileID::mud,
    TileID::mud,
    TileID::clay,
    TileID::silt,
    TileID::stone};
inline std::array const forestTiles{
    TileID::dirt,
    TileID::stone,
    TileID::clay,
    TileID::sand,
    TileID::mud};
inline std::array const underworldTiles{
    TileID::ash,
    TileID::ash,
    TileID::ash,
    TileID::ash,
    TileID::ash};

inline auto hashPoint = [](const Point &pt) { return fnv1a32pt(pt.x, pt.y); };

std::vector<Point>
iterateHex(Point start, int scale, std::function<void(Point)> f)
{
    std::unordered_set<Point, decltype(hashPoint)> visited(
        scale * scale,
        hashPoint);
    std::vector<Point> locations{start};
    Point centroid = getHexCentroid(start, scale);
    std::vector<Point> border;
    while (!locations.empty()) {
        Point loc = locations.back();
        locations.pop_back();
        if (visited.contains(loc)) {
            continue;
        }
        visited.insert(loc);
        if (centroid == getHexCentroid(loc, scale)) {
            f(loc);
            for (auto delta : {Point{-1, 0}, {1, 0}, {0, -1}, {0, 1}}) {
                locations.push_back(loc + delta);
            }
        } else {
            border.push_back(loc);
        }
    }
    return border;
}

int getWallVarIndex(
    int x,
    int y,
    const std::vector<std::pair<int, int>> &wallVarNoise,
    Random &rnd);

Biome getBiomeAt(int x, int y, Random &rnd, World &world)
{
    if (world.conf.biomes != BiomeLayout::columns) {
        return computeBiomeData(x, y, rnd).active;
    }
    if (y >= world.getUnderworldLevel() + 20 * rnd.getCoarseNoise(x, 0)) {
        return Biome::underworld;
    } else if (
        std::abs(x - world.jungleCenter) <
        world.conf.jungleSize * 0.09 * world.getWidth()) {
        return Biome::jungle;
    } else if (
        y < 0.7917 * world.getHeight() &&
        std::abs(x - world.desertCenter) <
            world.conf.desertSize * 0.0584 * world.getWidth()) {
        return Biome::desert;
    } else if (
        y < 0.7244 * world.getHeight() &&
        std::abs(x - world.snowCenter) <
            world.conf.snowSize * 0.0584 * world.getWidth()) {
        return Biome::snow;
    }
    return Biome::forest;
}

std::vector<Point> planHiveQueenBiomes(Random &rnd, World &world)
{
    std::mutex ptMtx;
    std::vector<Point> borders;
    Point hexShift{rnd.getInt(0, 99999), rnd.getInt(0, 99999)};
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [hexShift, &ptMtx, &borders, &rnd, &world](int x) {
            auto valueLess = [](const std::pair<Biome, int> &a,
                                const std::pair<Biome, int> &b) {
                return a.second < b.second;
            };
            std::vector<Point> queuedBorders;
            for (int y = 0; y < world.getHeight(); ++y) {
                if (world.getBiome(x, y).forest > 0) {
                    continue;
                }
                std::vector<Point> locations;
                std::map<Biome, int> biomes;
                std::vector<Point> hexBorder = iterateHex(
                    hexShift + Point{x, y},
                    131,
                    [hexShift, &locations, &biomes, &rnd, &world](Point pt) {
                        pt -= hexShift;
                        if (pt.x < 0 || pt.y < 0 || pt.x >= world.getWidth() ||
                            pt.y >= world.getHeight()) {
                            return;
                        }
                        locations.push_back(pt);
                        biomes[getBiomeAt(pt.x, pt.y, rnd, world)] += 1;
                    });
                for (Point pt : hexBorder) {
                    queuedBorders.push_back(pt - hexShift);
                }
                auto targBiome =
                    std::max_element(biomes.begin(), biomes.end(), valueLess);
                for (auto [ptX, ptY] : locations) {
                    BiomeData &biome = world.getBiome(ptX, ptY);
                    biome.active = targBiome->first;
                    biome.forest = 1;
                }
            }
            if (!queuedBorders.empty()) {
                std::lock_guard lock{ptMtx};
                borders.insert(
                    borders.end(),
                    queuedBorders.begin(),
                    queuedBorders.end());
            }
        });
    parallelFor(std::views::iota(0, world.getWidth()), [&world](int x) {
        for (int y = 0; y < world.getHeight(); ++y) {
            BiomeData &biome = world.getBiome(x, y);
            biome.forest = 0;
            switch (biome.active) {
            case Biome::forest:
                biome.forest = 1;
                break;
            case Biome::snow:
                biome.snow = 1;
                break;
            case Biome::desert:
                biome.desert = 1;
                break;
            case Biome::jungle:
                biome.jungle = 1;
                break;
            case Biome::underworld:
                biome.underworld = 1;
                break;
            }
        }
    });
    if (world.conf.biomes != BiomeLayout::columns) {
        identifySurfaceBiomes(world);
    }
    return borders;
}

void genWorldBaseHiveQueen(Random &rnd, World &world)
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
    std::vector<Point> hexBorders = planHiveQueenBiomes(rnd, world);
    double oreThreshold = computeOreThreshold(1.35 * world.conf.ore);
    double goldThreshold = world.conf.forTheWorthy
                               ? computeOreThreshold(1.82 * world.conf.ore)
                               : oreThreshold;
    int lavaLevel =
        (world.getCavernLevel() + 2 * world.getUnderworldLevel()) / 3;
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [lavaLevel,
         oreThreshold,
         goldThreshold,
         &depositNoise,
         &wallVarNoise,
         &rnd,
         &world](int x) {
            bool nearEdge = x < 350 || x > world.getWidth() - 350;
            for (int y = 0; y < world.getHeight(); ++y) {
                BiomeData &biome = world.getBiome(x, y);
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
                                   : 1;
                for (int iter = 0; iter < 3; ++iter) {
                    if (rnd.getFineNoise(
                            x + depositNoise[iter].first,
                            y + depositNoise[iter].second) > 0.7) {
                        tileType = iter + 2;
                        break;
                    }
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
                    tile.blockID = jungleTiles
                        [y > lavaLevel && tileType == 0 ? 3 : tileType];
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
                threshold =
                    y < world.getUndergroundLevel()
                        ? 2.94 - 3.1 * y / world.getUndergroundLevel()
                    : y > world.getUnderworldLevel()
                        ? 3.1 * (y - world.getUnderworldLevel()) / 230 - 0.16
                        : -0.16;
                if (std::abs(rnd.getCoarseNoise(x, 2 * y) + 0.1) < 0.15 &&
                    rnd.getFineNoise(x, y) > threshold) {
                    tile.wireRed = true;
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
                    tile.wireRed = true;
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
                        tile.wireRed = true;
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
                        if (tile.blockID == TileID::snow) {
                            tile.blockID = TileID::thinIce;
                        } else {
                            tile.wireRed = true;
                        }
                    }
                }
                if (tile.wireRed && y < world.getUndergroundLevel()) {
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
                    tile.wireRed = true;
                }
            }
        });

    std::cout << "Generating honeycomb\n";
    parallelFor(
        std::views::iota(0, world.getWidth()),
        [lavaLevel, &rnd, &world](int x) {
            auto valueLess = [](const std::pair<int, int> &a,
                                const std::pair<int, int> &b) {
                return a.second < b.second;
            };
            for (int y = 0; y < world.getHeight(); ++y) {
                if (world.getTile(x, y).wireBlue) {
                    continue;
                }
                std::vector<Point> locations;
                std::map<int, int> tiles;
                std::map<int, int> walls;
                iterateHex(
                    {x, y},
                    10,
                    [&locations, &tiles, &walls, &world](Point pt) {
                        if (pt.x < 0 || pt.y < 0 || pt.x >= world.getWidth() ||
                            pt.y >= world.getHeight()) {
                            return;
                        }
                        locations.push_back(pt);
                        Tile &tile = world.getTile(pt);
                        tiles[tile.wireRed ? TileID::empty : tile.blockID] += 1;
                        walls[tile.wallID] += 1;
                    });
                tiles[TileID::empty] *= 1.3;
                if (y < world.getUndergroundLevel()) {
                    walls[WallID::empty] *= 1.3;
                }
                auto targTile =
                    std::max_element(tiles.begin(), tiles.end(), valueLess);
                auto targWall =
                    std::max_element(walls.begin(), walls.end(), valueLess);
                int threshold = std::max<int>(0.6 * locations.size(), 2);
                if (targTile->second < threshold) {
                    targTile->second = -1;
                } else {
                    int oreCount = 0;
                    for (auto ore :
                         {TileID::copperOre,
                          TileID::tinOre,
                          TileID::ironOre,
                          TileID::leadOre,
                          TileID::silverOre,
                          TileID::tungstenOre,
                          TileID::goldOre,
                          TileID::platinumOre,
                          TileID::hellstone,
                          TileID::desertFossil}) {
                        oreCount += tiles[ore];
                    }
                    if (oreCount < threshold &&
                        oreCount > std::max(threshold / 4, 5)) {
                        targTile->second = -1;
                    }
                }
                if (targWall->second < threshold) {
                    targWall->second = -1;
                }
                Point centroid = getHexCentroid(x, y, 10);
                int rndFlag =
                    static_cast<int>(
                        99999 *
                        (1 + rnd.getFineNoise(centroid.x, centroid.y))) %
                    13;
                Flag hexFlag = rndFlag > 5              ? Flag::orange
                               : rndFlag > 1            ? Flag::yellow
                               : centroid.y > lavaLevel ? Flag::crispyHoney
                                                        : Flag::hive;
                for (auto pt : locations) {
                    Tile &tile = world.getTile(pt);
                    tile.wireRed = false;
                    tile.wireBlue = true;
                    if (targTile->second > 0) {
                        tile.blockID = targTile->first;
                    }
                    if (targWall->second > 0) {
                        tile.wallID = targWall->first;
                    }
                    tile.flag = hexFlag;
                }
            }
        });

    parallelFor(std::views::iota(0, world.getWidth()), [&rnd, &world](int x) {
        int surfaceLevel =
            scanWhileEmpty({x, world.getSurfaceLevel(x) - 10}, {0, 1}, world)
                .y +
            1;
        world.getSurfaceLevel(x) = surfaceLevel;
        for (int y = 0; y < world.getHeight(); ++y) {
            Tile &tile = world.getTile(x, y);
            if (y <= surfaceLevel) {
                tile.wallID = WallID::empty;
            }
            BiomeData &biome = world.getBiome(x, y);
            tile.wireBlue = false;
            if (!world.isExposed(x, y)) {

                if (y < world.getUndergroundLevel() &&
                    ((biome.active == Biome::forest &&
                      tile.blockID == TileID::dirt) ||
                     (biome.active == Biome::jungle &&
                      tile.blockID == TileID::mud)) &&
                    static_cast<int>(99999 * (1 + rnd.getFineNoise(x, y))) %
                            100 ==
                        0) {
                    tile.blockID = biome.active == Biome::forest
                                       ? TileID::grass
                                       : TileID::jungleGrass;
                }

                continue;
            }
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
    world.spawn = {
        world.getWidth() / 2,
        world.getSurfaceLevel(world.getWidth() / 2) - 1};

    parallelFor(hexBorders, [&world](Point pt) {
        for (int i = -3; i < 3; ++i) {
            for (int j = -3; j < 3; ++j) {
                Tile &tile = world.getTile(pt + Point{i, j});
                tile.blockID = TileID::hive;
                tile.wallID = WallID::Unsafe::hive;
                tile.flag = Flag::border;
            }
        }
    });
}
